#include "ServerConnections.h"

using std::move;
using std::chrono::steady_clock;

ConnectionInfo::ConnectionInfo(SOCKET socket)
	: m_ssl{nullptr},
	m_socket{socket}
{}

ConnectionInfo::ConnectionInfo(SOCKET socket, SSL* ssl)
	: m_socket{socket},
	m_ssl{ssl}
{}

ConnectionInfo::ConnectionInfo(ConnectionInfo&& move)
	: m_socket{move.Socket()},
	m_ssl{move.GetSSL()},
	m_socketStatus{move.SocketStatus()},
	m_sslStatus{move.SSLStatus()},
	m_bytesSent{move.BytesSent()}
{
	move.SetSocket(INVALID_SOCKET);
	move.SetSSL(nullptr);
}

ConnectionInfo& ConnectionInfo::operator=(ConnectionInfo&& move)
{
	m_socket = move.Socket();
	m_ssl = move.GetSSL();
	m_socketStatus = move.SocketStatus();
	m_sslStatus = move.SSLStatus();
	m_bytesSent = move.BytesSent();

	move.SetSocket(INVALID_SOCKET);
	move.SetSSL(nullptr);

	return *this;
}

ConnectionInfo::~ConnectionInfo()
{
	Shutdown();
}

int ConnectionInfo::SocketStatus()
{
	return m_socketStatus;
}

int ConnectionInfo::SSLStatus()
{
	return m_sslStatus;
}

int ConnectionInfo::BytesSent()
{
	return m_bytesSent;
}

void ConnectionInfo::SetSocket(SOCKET socket)
{
	m_socket = socket;
}

void ConnectionInfo::SetSSL(SSL* ssl)
{
	m_ssl = ssl;
}

ConnectionState ConnectionInfo::Send(const std::string& msg)
{
	if (m_ssl != nullptr)
	{
		return SendSSL(msg);
	}
	else if (m_socket != INVALID_SOCKET)
	{
		return SendSocket(msg);
	}
	else
	{
		return ConnectionState::NOT_INITIATED;
	}
}

ConnectionState ConnectionInfo::Receive(std::string& buffer, bool file, int expectedLength)
{
	if (m_ssl != nullptr)
	{
		return ReceiveSSL(buffer, file, expectedLength);
	}
	else if (m_socket != INVALID_SOCKET)
	{ 
		return ReceiveSocket(buffer, file, expectedLength);
	}
	else
	{
		return ConnectionState::NOT_INITIATED;
	}
}

void ConnectionInfo::Shutdown(int how)
{
	if (m_ssl != nullptr)
	{
		int shutdown = SSL_shutdown(m_ssl);
		if (shutdown == 0)
			SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
		m_ssl = nullptr;
	}
	if (m_socket != INVALID_SOCKET)
	{
		shutdown(m_socket, how);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool ConnectionInfo::IsValid()
{
	return m_ssl != nullptr || m_socket != INVALID_SOCKET;
}

ConnectionState ConnectionInfo::Accept(SOCKET listener, SSL_CTX* ssl_ctx)
{
	struct sockaddr_in addr;
	int len = sizeof(addr);
	Shutdown();
	m_socket = accept(listener, (sockaddr*)&addr, &len);

	if (m_socket == INVALID_SOCKET)
	{
		m_socketStatus = WSAGetLastError();
		if (m_socketStatus == WSAEWOULDBLOCK)
		{
			return ConnectionState::NO_DATA_PRESENT;
		}

		/* Log error. */
		string fName = string(ERROR_LOGS_LOCATION);
		fName += CONNECTION_ERROR_LOG;
		LogManager::LogConnection(fName, time(nullptr), addr.sin_addr.S_un.S_addr);

		return ConnectionState::ERR;
	}
	else
	{
		m_socketStatus = SOCKET_OK;
	}

	m_ssl = nullptr;
	if (ssl_ctx != nullptr)
	{
		/* Create the SSL object and attempt to accept an SSL connection. */
		m_ssl = SSL_new(ssl_ctx);
		SSL_set_fd(m_ssl, m_socket);
		int acpt = SSL_accept(m_ssl);
		if (acpt <= 0)
		{
			m_sslStatus = SSL_get_error(m_ssl, acpt);

			SSL_free(m_ssl);
			m_ssl = nullptr;

			closesocket(m_socket);
			m_socket = INVALID_SOCKET;

			LogManager::LogSSLError();

			return ConnectionState::ERR;
		}
		else
		{
			m_sslStatus = SSL_ERROR_NONE;
		}
	}

	return ConnectionState::OK;
}

SOCKET ConnectionInfo::Socket()
{
	return m_socket;
}

SSL* ConnectionInfo::GetSSL()
{
	return m_ssl;
}

ConnectionState ConnectionInfo::SendSSL(const std::string& msg)
{
	ConnectionState ret = ConnectionState::NOT_SENT;
	m_bytesSent = 0;
	m_sslStatus = SSL_ERROR_NONE;

	if (SelectForWrite() > 0)
	{
		int write = SSL_write(m_ssl, msg.c_str(), msg.size() + 1);
		/*Either an error occured, or not all of the bytes were sent.*/
		if (write < msg.size() + 1)
		{
			/*The client shutdown their connection.*/
			if (write == 0)
			{
				m_sslStatus = SSL_get_error(m_ssl, write);
				m_bytesSent = 0;
				Shutdown();
				ret = ConnectionState::SHUTDOWN;
			}
			else if (write < 0)
			{
				m_sslStatus = SSL_get_error(m_ssl, write);
				m_bytesSent = 0;
				switch (m_sslStatus)
				{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						ret = ConnectionState::WANT_WRITE;
						break;
					default:
						ret = ConnectionState::ERR;
						break;
				}
			}
			else
			{
				m_sslStatus = SSL_ERROR_NONE;
				m_bytesSent = write;
				ret = ConnectionState::NOT_FULLY_SENT;
			}
		}
		else
		{
			m_sslStatus = SSL_ERROR_NONE;
			m_bytesSent = write;
			ret = ConnectionState::SENT;
		}
	}

	return ret;
}

ConnectionState ConnectionInfo::SendSocket(const std::string& msg)
{
	ConnectionState ret = ConnectionState::NOT_SENT;
	m_bytesSent = 0;
	m_socketStatus = SOCKET_OK;

	if (SelectForWrite() > 0)
	{
		int write = send(m_socket, msg.c_str(), msg.size() + 1, 0);

		if (write == msg.size() + 1)
		{
			m_socketStatus = SOCKET_OK;
			m_bytesSent = write;
			ret = ConnectionState::SENT;
		}
		else if (write == SOCKET_ERROR)
		{
			m_socketStatus = GetSocketError();
			m_bytesSent = 0;
			ret = ConnectionState::ERR;
		}
		else
		{
			m_socketStatus = SOCKET_OK;
			m_bytesSent = write;
			ret = ConnectionState::NOT_FULLY_SENT;
		}
	}

	return ret;
}

ConnectionState ConnectionInfo::ReceiveSSL(std::string& buffer, bool file, int expectedLength)
{
	int bytes = SSL_pending(m_ssl);
	if (bytes > 0)
	{
		char* msg = new char[bytes + 1];
		int ret = SSL_read(m_ssl, msg, bytes);

		if (ret <= 0)
		{
			delete[] msg;
			m_sslStatus = SSL_get_error(m_ssl, ret);
			switch (m_sslStatus)
			{
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					return ConnectionState::WANT_READ;
				case SSL_ERROR_ZERO_RETURN:
					Shutdown();
					return ConnectionState::SHUTDOWN;
				default:
					return ConnectionState::ERR;

			}
		}
		else
		{
			buffer = msg;
			delete[] msg;
			m_sslStatus = SSL_ERROR_NONE;
			return ConnectionState::RECEIVED;
		}
	}
	else
	{
		m_sslStatus = SSL_ERROR_NONE;
		return ConnectionState::NO_DATA_PRESENT;
	}
}

ConnectionState ConnectionInfo::ReceiveSocket(std::string& buffer, bool file, int expectedLength)
{
	int bytes = recv(m_socket, nullptr, 0, MSG_PEEK);

	if (bytes > 0)
	{
		char* data = new char[bytes + 1];
		bytes = recv(m_socket, data, bytes, 0);
		data[bytes] = '\0';
		buffer = data;
		delete[] data;

		m_socketStatus = SOCKET_OK;
		return ConnectionState::RECEIVED;
	}
	else if (bytes == SOCKET_ERROR)
	{
		m_socketStatus = GetSocketError();
		if (m_socketStatus == WSAEWOULDBLOCK)
		{
			return ConnectionState::NO_DATA_PRESENT;
		}
		else
		{
			return ConnectionState::ERR;
		}
	}
	else
	{
		Shutdown();
		m_socketStatus = SOCKET_OK;

		return ConnectionState::SHUTDOWN;
	}
}

int ConnectionInfo::SelectForRead()
{
	fd_set set;
	ZeroMemory(&set, sizeof(set));
	FD_SET(m_socket, &set);
	timeval timeout{ 0, 100 };

	return select(0, &set, nullptr, nullptr, &timeout);
}

int ConnectionInfo::SelectForWrite()
{
	fd_set set;
	ZeroMemory(&set, sizeof(set));
	FD_SET(m_socket, &set);
	timeval timeout{ 0, 100 };

	return select(0, nullptr, &set, nullptr, &timeout);
}

int ConnectionInfo::GetSocketError()
{
	int val;
	int len = sizeof(val);

	getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&val, &len);

	return val;
}
int ConnectionInfo::GetSocketError(SOCKET socket)
{
	int val, len = sizeof(val);

	getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&val, &len);

	return val;
}