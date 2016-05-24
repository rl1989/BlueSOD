#pragma once
#include <WinSock2.h>
#include <memory>
#include <openssl\ssl.h>

//The maximum size of a message a client can send.
#define BUFFER_SIZE 1024
//Defines the state of the server. Names are self explanatory.
enum ServerState
{
	OFF, RUNNING, NOT_ACCEPTING_CONNECTIONS, RESET
};

enum ConnectionStatus
{
	ALL_OK, CONNECTION_ACCEPTED, NOT_OK, NO_CONNECTION_PRESENT
};

struct Buffer
{
	WSABUF buffer;
	DWORD bytesSent;
	DWORD bytesRecv;

	Buffer();
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
	std::shared_ptr<Buffer> buffer;
	Connection connection;
	bool verified;

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