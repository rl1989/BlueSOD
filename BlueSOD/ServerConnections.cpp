#include "ServerConnections.h"

using std::move;

Buffer::Buffer()
	:bytesSent{},
	bytesRecv{}
{
	buffer.buf = new CHAR[BUFFER_SIZE];
	buffer.len = BUFFER_SIZE;
}

Buffer::Buffer(const Buffer& ref)
	: bytesSent{ref.bytesSent},
	bytesRecv{ref.bytesRecv}
{
	buffer.len = ref.buffer.len;
	buffer.buf = new CHAR[buffer.len];
	for (int i = 0; i < buffer.len; i++)
		buffer.buf[i] = ref.buffer.buf[i];
}

Buffer& Buffer::operator=(const Buffer& ref)
{
	if (buffer.buf != nullptr)
		delete[] buffer.buf;
	buffer.len = ref.buffer.len;
	buffer.buf = new CHAR[buffer.len];
	for (int i = 0; i < buffer.len; i++)
		buffer.buf[i] = ref.buffer.buf[i];

	return *this;
}

Buffer::Buffer(Buffer&& move)
	: bytesRecv{move.bytesRecv},
	bytesSent{move.bytesSent}
{
	buffer.len = move.buffer.len;
	buffer.buf = move.buffer.buf;
	
	move.buffer.buf = nullptr;
	move.buffer.len = 0;
}

Buffer& Buffer::operator=(Buffer&& move)
{
	if (buffer.buf != nullptr)
		delete[] buffer.buf;

	bytesRecv = move.bytesRecv;
	bytesSent = move.bytesSent;
	buffer.buf = move.buffer.buf;
	buffer.len = move.buffer.len;

	move.buffer.len = 0;
	move.buffer.buf = nullptr;

	return *this;
}

Buffer::~Buffer()
{
	if (buffer.buf)
		delete[] buffer.buf;
}

ConnectionInfo::ConnectionInfo()
	: buffer{},
	connection{},
	verified{false},
	connStatus{ ConnectionStatus::CONNECTION_ERROR }
{}

ConnectionInfo::ConnectionInfo(const Connection& ref)
	: connection{ref.socket, ref.ssl, ref.address},
	buffer{},
	verified{false}
{}

ConnectionInfo::ConnectionInfo(Connection&& move)
	: buffer{},
	connection{ move.socket, move.ssl, move.address },
	verified{false}
{
	move.socket = INVALID_SOCKET;
	move.ssl = nullptr;
	move.address = 0;
}

ConnectionInfo& ConnectionInfo::operator=(const Connection& ref)
{
	if (connection == ref)
		return *this;
	
	connection = ref;

	return *this;
}

ConnectionInfo& ConnectionInfo::operator=(Connection&& move)
{
	connection = move;

	move.socket = INVALID_SOCKET;
	move.ssl = nullptr;
	move.address = 0;

	return *this;
}

ConnectionInfo::ConnectionInfo(const ConnectionInfo& ref)
	: connection{ref.connection.socket, ref.connection.ssl, ref.connection.address},
	verified{ref.verified},
	buffer{ref.buffer},
	connStatus{ref.connStatus},
	sslStatus{ref.sslStatus}
{
}

ConnectionInfo::ConnectionInfo(ConnectionInfo&& move)
	: connection{move.connection.socket, move.connection.ssl, move.connection.address},
	buffer{move.buffer},
	verified{move.verified},
	connStatus{move.connStatus},
	sslStatus{move.sslStatus}
{
	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;
	move.connStatus = ConnectionStatus::CONNECTION_ERROR;
	move.sslStatus = SSLStatus::SSL_ERROR;
}

ConnectionInfo& ConnectionInfo::operator=(const ConnectionInfo& ref)
{
	connection.socket = ref.connection.socket;
	connection.ssl = ref.connection.ssl;
	connection.address = ref.connection.address;
	buffer = ref.buffer;
	verified = ref.verified;
	connStatus = ref.connStatus;
	sslStatus = ref.sslStatus;

	return *this;
}

ConnectionInfo& ConnectionInfo::operator=(ConnectionInfo&& move)
{
	connection = move.connection;
	buffer = std::move(move.buffer);
	verified = move.verified;
	connStatus = move.connStatus;
	sslStatus = move.sslStatus;

	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;
	move.connStatus = ConnectionStatus::CONNECTION_ERROR;
	move.sslStatus = SSLStatus::SSL_ERROR;

	return *this;
}

ConnectionInfo::~ConnectionInfo()
{
	connection.Close();
}

bool ConnectionInfo::operator==(const ConnectionInfo& ref)
{
	return (connection == ref.connection && buffer.buffer.buf == ref.buffer.buffer.buf
		&& buffer.buffer.len == ref.buffer.buffer.len && verified == ref.verified 
		&& connStatus == ref.connStatus && sslStatus == ref.sslStatus);
}

bool Connection::operator==(const Connection& ref)
{
	return (socket == ref.socket && ssl == ref.ssl && address == ref.address);
}

void Connection::Close()
{
	if (ssl != nullptr)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}
}

ConnectionInfo* ReadFromSSL(ConnectionInfo* ci)
{
	SSL* ssl = ci->connection.ssl;
	int length = ci->buffer.buffer.len;
	char* buffer = ci->buffer.buffer.buf;

	if (SSL_pending(ssl) > 0)
	{
		int res = SSL_read(ssl, buffer, length);

		if (res <= 0)
		{
			ci->sslStatus = SSLStatus::SSL_ERROR;
			return ci;
		}
		ci->buffer.bytesRecv = res;
		ci->sslStatus = SSLStatus::SSL_READ;
	}
	else
	{
		ci->sslStatus = SSLStatus::NO_DATA_PRESENT;
	}

	return ci;
}

ConnectionInfo* WriteToSSL(ConnectionInfo* ci)
{
	SSL* ssl = ci->connection.ssl;
	int length = ci->buffer.buffer.len;
	char* buffer = ci->buffer.buffer.buf;
	{
		fd_set write;
		ZeroMemory(&write, sizeof(write));
		FD_SET(ci->connection.socket, &write);
		timeval timeout{0, 100};

		switch (select(0, nullptr, &write, nullptr, &timeout))
		{
			case 0:
				ci->connStatus = ConnectionStatus::CONNECTION_ERROR;
				return ci;
			case SOCKET_ERROR:
				ci->connStatus = ConnectionStatus::CONNECTION_ERROR;
				return ci;
			default:
				ci->connStatus = ConnectionStatus::CONNECTION_OK;
				break;
		}
	}

	int res = SSL_write(ssl, buffer, length);

	if (res <= 0)
	{
		ci->sslStatus = SSLStatus::SSL_ERROR;
		return ci;
	}
	ci->buffer.bytesSent = res;
	ci->sslStatus = SSLStatus::SSL_SENT;

	return ci;
}

ConnectionInfo* ReadFromSocket(ConnectionInfo* ci)
{
	SOCKET s = ci->connection.socket;
	int length = ci->buffer.buffer.len;
	char* buffer = ci->buffer.buffer.buf;
	int res = recv(s, buffer, length, MSG_OOB);

	if (res == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			ci->connStatus = ConnectionStatus::NO_DATA_PRESENT;
		}
		else
		{
			ci->connStatus = ConnectionStatus::CONNECTION_ERROR;
		}

		return ci;
	}

	ci->buffer.bytesRecv = res;
	ci->connStatus = ConnectionStatus::CONNECTION_READ;

	return ci;
}

ConnectionInfo* WriteToSocket(ConnectionInfo* ci)
{
	SOCKET s = ci->connection.socket;
	int length = ci->buffer.buffer.len;
	char* buffer = ci->buffer.buffer.buf;
	int res = send(s, buffer, length, MSG_OOB);

	if (res == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			ci->connStatus = ConnectionStatus::NO_DATA_PRESENT;
		}
		else
		{
			ci->connStatus = ConnectionStatus::CONNECTION_ERROR;
		}

		return ci;
	}

	ci->buffer.bytesSent = res;
	ci->connStatus = ConnectionStatus::CONNECTION_SENT;

	return ci;
}
