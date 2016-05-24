#include "CommonServer.h"

Buffer::Buffer()
	:bytesSent{},
	bytesRecv{}
{
	buffer.buf = new CHAR[BUFFER_SIZE];
	buffer.len = BUFFER_SIZE;
}

Buffer::~Buffer()
{
	if (buffer.buf)
		delete[] buffer.buf;
}

ConnectionInfo::ConnectionInfo()
	: buffer{ new Buffer{} },
	connection{},
	verified{false}
{}

ConnectionInfo::ConnectionInfo(const Connection& ref)
	: connection{ref.socket, ref.ssl, ref.address},
	buffer{ new Buffer{} },
	verified{false}
{}

ConnectionInfo::ConnectionInfo(Connection&& move)
	: buffer{new Buffer{}},
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
	buffer{std::move(ref.buffer)},
	verified{ref.verified}
{
}

ConnectionInfo::ConnectionInfo(ConnectionInfo&& move)
	: connection{move.connection.socket, move.connection.ssl, move.connection.address},
	buffer{std::move(move.buffer)},
	verified{move.verified}
{
	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;
	move.buffer = nullptr;
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

	return *this;
}

ConnectionInfo& ConnectionInfo::operator=(ConnectionInfo&& move)
{
	connection = move.connection;
	buffer = std::move(move.buffer);
	verified = move.verified;

	move.connection.socket = INVALID_SOCKET;
	move.connection.ssl = nullptr;
	move.connection.address = 0;

	return *this;
}

bool ConnectionInfo::operator==(const ConnectionInfo & ref)
{
	return (connection == ref.connection && buffer->buffer.buf == ref.buffer->buffer.buf
		&& buffer->buffer.len == ref.buffer->buffer.len && verified == ref.verified);
}

bool Connection::operator==(const Connection & ref)
{
	return (socket == ref.socket && ssl == ref.ssl && address == ref.address);
}
