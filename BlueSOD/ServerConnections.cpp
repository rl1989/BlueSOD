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
	connStatus{ref.connStatus}
{
}

ConnectionInfo::ConnectionInfo(ConnectionInfo&& move)
	: connection{move.connection.socket, move.connection.ssl, move.connection.address},
	buffer{move.buffer},
	verified{move.verified},
	connStatus{move.connStatus}
{
	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;
	move.connStatus = ConnectionStatus::CONNECTION_NOT_INITIATED;
}

ConnectionInfo& ConnectionInfo::operator=(const ConnectionInfo& ref)
{
	if (*this == ref)
		return *this;

	connection.socket = ref.connection.socket;
	connection.ssl = ref.connection.ssl;
	connection.address = ref.connection.address;
	buffer = ref.buffer;
	verified = ref.verified;
	connStatus = ref.connStatus;

	return *this;
}

ConnectionInfo& ConnectionInfo::operator=(ConnectionInfo&& move)
{
	connection = move.connection;
	buffer = std::move(move.buffer);
	verified = move.verified;
	connStatus = move.connStatus;

	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;
	move.connStatus = ConnectionStatus::CONNECTION_NOT_INITIATED;

	return *this;
}

bool ConnectionInfo::operator==(const ConnectionInfo & ref)
{
	return (connection == ref.connection && buffer.buffer.buf == ref.buffer.buffer.buf
		&& buffer.buffer.len == ref.buffer.buffer.len && verified == ref.verified 
		&& connStatus == ref.connStatus);
}

bool Connection::operator==(const Connection& ref)
{
	return (socket == ref.socket && ssl == ref.ssl && address == ref.address);
}

ConnectionInfo* ReadFromSSL(ConnectionInfo* ci, int length)
{
	SSL* ssl = ci->connection.ssl;
	if (ssl == nullptr)
	{
		ci->connStatus = ConnectionStatus::SSL_NOT_INITIATED;
		return ci;
	}

	char* buffer = ci->buffer.buffer.buf;
	int res = SSL_read(ssl, buffer, length);
	if (res <= 0)
	{
		LogManager::LogSSLError();
		ci->connStatus = ConnectionStatus::SSL_ERROR;
		return ci;
	}
	ci->buffer.bytesRecv = res;
	ci->connStatus = ConnectionStatus::SSL_READ;

	return ci;
}

ConnectionInfo* SendToSSL(ConnectionInfo* ci, int length)
{
	SSL* ssl = ci->connection.ssl;
	if (ssl == nullptr)
	{
		ci->connStatus = ConnectionStatus::SSL_NOT_INITIATED;
		return ci;
	}

	char* buffer = ci->buffer.buffer.buf;
	int res = SSL_write(ssl, buffer, length);
	if (res <= 0)
	{
		ci->connStatus = ConnectionStatus::SSL_ERROR;
		LogManager::LogSSLError();
		return ci;
	}
	ci->connStatus = ConnectionStatus::SSL_SENT;

	return ci;
}
