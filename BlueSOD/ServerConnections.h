#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include "LogManager.h"

//The maximum size of a message a client can send.
#define BUFFER_SIZE 1024
//Defines the state of the server. Names are self explanatory.
enum ServerState
{
	OFF, RUNNING, NOT_ACCEPTING_CONNECTIONS, RESET, START_UP
};

enum ConnectionStatus
{
	CONNECTION_OK,
	CONNECTION_ACCEPTED,
	CONNECTION_ERROR,
	NO_DATA_PRESENT,
	CONNECTION_NOT_INITIATED,
	SSL_OK,
	SSL_SENT = SSL_OK,
	SSL_READ = SSL_OK,
	SSL_ERROR,
	SSL_NOT_INITIATED
};

struct Buffer
{
	WSABUF buffer;
	DWORD bytesSent;
	DWORD bytesRecv;

	Buffer();
	Buffer(const Buffer& ref);
	Buffer& operator=(const Buffer& ref);
	Buffer(Buffer&& move);
	Buffer& operator=(Buffer&& move);
	~Buffer();
};

struct Connection
{
	SOCKET socket;
	SSL* ssl;
	ULONG address;

	bool operator==(const Connection& ref);
	bool operator==(Connection& ref);
};

struct ConnectionInfo
{
	Buffer buffer;
	Connection connection;
	bool verified;
	ConnectionStatus connStatus;

	ConnectionInfo();
	ConnectionInfo(const Connection& ref);
	ConnectionInfo(Connection&& move);
	ConnectionInfo& operator=(const Connection& ref);
	ConnectionInfo& operator=(Connection&& move);
	ConnectionInfo(const ConnectionInfo& ref);
	ConnectionInfo(ConnectionInfo&& move);
	ConnectionInfo& operator=(const ConnectionInfo& ref);
	ConnectionInfo& operator=(ConnectionInfo&& move);
	~ConnectionInfo()=default;

	bool operator==(const ConnectionInfo& ref);
	bool operator==(ConnectionInfo& ref);
};

/*
	Read from the SSL connection. The message is stored in buffer which must be of size BUFFER_SIZE.
	Alternatively, you can tell the function where in the buffer to store the information. If the amount
	of data to be read is more than length-start, then the data is thrown away. If buffer is nullptr, then
	this function throws away the data that would have been stored there.

	The status of the connection is stored in ci.connStatus.
*/
[[noreturn]]
ConnectionInfo* ReadFromSSL(ConnectionInfo* ci, int length = BUFFER_SIZE);

/*
	Send a message to an SSL connection.

	The status of the connection is stored in ci.connStatus;
*/
[[noreturn]]
ConnectionInfo* SendToSSL(ConnectionInfo* ci, int length = BUFFER_SIZE);