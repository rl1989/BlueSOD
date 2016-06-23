#include "ServerConnections.h"

using std::move;
using std::chrono::steady_clock;

NewConnectionInfo::NewConnectionInfo(SOCKET socket)
	: m_ssl{nullptr},
	m_socket{socket}
{}

NewConnectionInfo::NewConnectionInfo(SOCKET socket, SSL* ssl)
	: m_socket{socket},
	m_ssl{ssl}
{}

NewConnectionInfo::NewConnectionInfo(NewConnectionInfo&& move)
	: m_socket{move.GetSocket()},
	m_ssl{move.GetSSL()},
	m_socketStatus{move.GetSocketStatus()},
	m_sslStatus{move.GetSSLStatus()},
	m_bytesSent{move.BytesSent()}
{
	move.SetSocket(INVALID_SOCKET);
	move.SetSSL(nullptr);
}

NewConnectionInfo& NewConnectionInfo::operator=(NewConnectionInfo&& move)
{
	m_socket = move.GetSocket();
	m_ssl = move.GetSSL();
	m_socketStatus = move.GetSocketStatus();
	m_sslStatus = move.GetSSLStatus();
	m_bytesSent = move.BytesSent();

	move.SetSocket(INVALID_SOCKET);
	move.SetSSL(nullptr);

	return *this;
}

NewConnectionInfo::~NewConnectionInfo()
{
	Shutdown();
}

int NewConnectionInfo::GetSocketStatus()
{
	return m_socketStatus;
}

int NewConnectionInfo::GetSSLStatus()
{
	return m_sslStatus;
}

int NewConnectionInfo::BytesSent()
{
	return m_bytesSent;
}

void NewConnectionInfo::SetSocket(SOCKET socket)
{
	m_socket = socket;
}

void NewConnectionInfo::SetSSL(SSL* ssl)
{
	m_ssl = ssl;
}

connect_s NewConnectionInfo::Send(const std::string& msg)
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
		return connect_s::NOT_INITIATED;
	}
}

connect_s NewConnectionInfo::Receive(std::string& buffer, bool file, int expectedLength)
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
		return connect_s::NOT_INITIATED;
	}
}

void NewConnectionInfo::Shutdown(int how)
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

bool NewConnectionInfo::IsValid()
{
	return m_ssl != nullptr || m_socket != INVALID_SOCKET;
}

connect_s NewConnectionInfo::Accept(SOCKET listener, SSL_CTX* ssl_ctx)
{
	struct sockaddr_in addr;
	int len = sizeof(addr);
	Shutdown();
	m_socket = accept(listener, (sockaddr*)&addr, &len);

	if (m_socket == INVALID_SOCKET)
	{
		m_socketStatus = GetSocketError(m_socket);
		if (m_socketStatus == WSAEWOULDBLOCK)
		{
			return connect_s::NO_DATA_PRESENT;
		}

		/* Log error. */
		string fName = string(ERROR_LOGS_LOCATION);
		fName += CONNECTION_ERROR_LOG;
		LogManager::LogConnection(fName, time(nullptr), addr.sin_addr.S_un.S_addr);

		return connect_s::ERR;
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
			LogManager::LogSSLError();
			SSL_free(m_ssl);
			m_ssl = nullptr;

			return connect_s::ERR;
		}
		else
		{
			m_sslStatus = SSL_ERROR_NONE;
		}
	}

	return connect_s::OK;
}

SOCKET NewConnectionInfo::GetSocket()
{
	return m_socket;
}

SSL* NewConnectionInfo::GetSSL()
{
	return m_ssl;
}

connect_s NewConnectionInfo::SendSSL(const std::string& msg)
{
	connect_s ret = connect_s::NOT_SENT;
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
				ret = connect_s::SHUTDOWN;
			}
			else if (write < 0)
			{
				m_sslStatus = SSL_get_error(m_ssl, write);
				m_bytesSent = 0;
				switch (m_sslStatus)
				{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						ret = connect_s::WANT_WRITE;
						break;
					default:
						ret = connect_s::ERR;
						break;
				}
			}
			else
			{
				m_sslStatus = SSL_ERROR_NONE;
				m_bytesSent = write;
				ret = connect_s::NOT_FULLY_SENT;
			}
		}
		else
		{
			m_sslStatus = SSL_ERROR_NONE;
			m_bytesSent = write;
			ret = connect_s::SENT;
		}
	}

	return ret;
}

connect_s NewConnectionInfo::SendSocket(const std::string& msg)
{
	connect_s ret = connect_s::NOT_SENT;
	m_bytesSent = 0;
	m_socketStatus = SOCKET_OK;

	if (SelectForWrite() > 0)
	{
		int write = send(m_socket, msg.c_str(), msg.size() + 1, 0);

		if (write == msg.size() + 1)
		{
			m_socketStatus = SOCKET_OK;
			m_bytesSent = write;
			ret = connect_s::SENT;
		}
		else if (write == SOCKET_ERROR)
		{
			m_socketStatus = GetSocketError();
			m_bytesSent = 0;
			ret = connect_s::ERR;
		}
		else
		{
			m_socketStatus = SOCKET_OK;
			m_bytesSent = write;
			ret = connect_s::NOT_FULLY_SENT;
		}
	}

	return ret;
}

connect_s NewConnectionInfo::ReceiveSSL(std::string& buffer, bool file, int expectedLength)
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
					return connect_s::WANT_READ;
				case SSL_ERROR_ZERO_RETURN:
					Shutdown();
					return connect_s::SHUTDOWN;
				default:
					return connect_s::ERR;

			}
		}
		else
		{
			buffer = msg;
			delete[] msg;
			m_sslStatus = SSL_ERROR_NONE;
			return connect_s::RECEIVED;
		}
	}
	else
	{
		m_sslStatus = SSL_ERROR_NONE;
		return connect_s::NO_DATA_PRESENT;
	}
}

connect_s NewConnectionInfo::ReceiveSocket(std::string& buffer, bool file, int expectedLength)
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
		return connect_s::RECEIVED;
	}
	else if (bytes == SOCKET_ERROR)
	{
		m_socketStatus = GetSocketError();
		if (m_socketStatus == WSAEWOULDBLOCK)
		{
			return connect_s::NO_DATA_PRESENT;
		}
		else
		{
			return connect_s::ERR;
		}
	}
	else
	{
		Shutdown();
		m_socketStatus = SOCKET_OK;

		return connect_s::SHUTDOWN;
	}
}

int NewConnectionInfo::SelectForRead()
{
	fd_set set;
	ZeroMemory(&set, sizeof(set));
	FD_SET(m_socket, &set);
	timeval timeout{ 0, 100 };

	return select(0, &set, nullptr, nullptr, &timeout);
}

int NewConnectionInfo::SelectForWrite()
{
	fd_set set;
	ZeroMemory(&set, sizeof(set));
	FD_SET(m_socket, &set);
	timeval timeout{ 0, 100 };

	return select(0, nullptr, &set, nullptr, &timeout);
}

int NewConnectionInfo::GetSocketError()
{
	int val;
	int len = sizeof(val);

	getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&val, &len);

	return val;
}
int NewConnectionInfo::GetSocketError(SOCKET socket)
{
	int val, len = sizeof(val);

	getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&val, &len);

	return val;
}