#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include <string>
#include "LogManager.h"

//The maximum size of a message a client can send/receive at a time.
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
	CONNECTION_READ,
	CONNECTION_SENT,
	CONNECTION_ERROR,
	NO_DATA_PRESENT,
	CONNECTION_NOT_INITIATED
};

enum SSLStatus
{
	SSL_OK,
	SSL_ACCEPTED,
	SSL_SENT,
	SSL_READ,
	SSL_ERROR,
	NO_DATA_PRESENT,
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

	void Close();
};

struct ConnectionInfo
{
	Buffer buffer;
	Connection connection;
	bool verified;
	ConnectionStatus connStatus;
	SSLStatus sslStatus;

	ConnectionInfo();
	ConnectionInfo(const Connection& ref);
	ConnectionInfo(Connection&& move);
	ConnectionInfo& operator=(const Connection& ref);
	ConnectionInfo& operator=(Connection&& move);
	ConnectionInfo(const ConnectionInfo& ref);
	ConnectionInfo(ConnectionInfo&& move);
	ConnectionInfo& operator=(const ConnectionInfo& ref);
	ConnectionInfo& operator=(ConnectionInfo&& move);
	~ConnectionInfo();

	bool operator==(const ConnectionInfo& ref);
	bool operator==(ConnectionInfo& ref);
};

/*  Read from the SSL connection. The message is stored in buffer which must be of size BUFFER_SIZE.

	The status of the connection is stored in ci.sslStatus. */
ConnectionInfo* ReadFromSSL(ConnectionInfo* ci);

/*  Send a message that is stored in buffer to an SSL connection.

	The status of the connection is stored in ci.sslStatus. */
ConnectionInfo* WriteToSSL(ConnectionInfo* ci);

ConnectionInfo* ReadFromSocket(ConnectionInfo* ci);

ConnectionInfo* WriteToSocket(ConnectionInfo* ci);