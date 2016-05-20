#pragma once
#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif
#include <memory>
#include <openssl\ssl.h>

//The maximum size of a message a client can send.
#define BUFFER_SIZE 1024
//Defines the state of the server. Names are self explanatory.
enum ServerState
{
	OFF, RUNNING, NOT_ACCEPTING_CONNECTIONS, RESET
};
/*
Tells whether the user was authenticated or if a result is still pending.
*/
enum class UserAuthentication
{
	VALID, INVALID, PENDING
};
//Holds information unique to the client.
struct ClientInfo
{
	//Unique client connection information.
	Connection connection;
	//Required for WSA messages.
	WSABUF wsaBuffer;
	//These determine how much was sent/received.
	DWORD bytesSend;
	DWORD bytesRecv;

	ClientInfo(ClientInfo&& info)
		: connection{ info.connection.socket, info.connection.ssl, info.connection.address, info.connection.authenticityStatus },
		bytesRecv{ info.bytesRecv },
		bytesSend{ info.bytesSend }
	{
		wsaBuffer.buf = info.wsaBuffer.buf;
		wsaBuffer.len = info.wsaBuffer.len;
		info.wsaBuffer.buf = nullptr;
	}
	ClientInfo(const ClientInfo& info) = default;
	ClientInfo(Connection client)
		: connection { client.socket, client.ssl, client.address, client.authenticityStatus },
		bytesSend{},
		bytesRecv{}
	{
		wsaBuffer.buf = new char[BUFFER_SIZE];
		wsaBuffer.len = 0;
		bytesRecv = bytesSend = 0;
	}
	ClientInfo()
		: ClientInfo(Connection{}) {}

	~ClientInfo()
	{
		if (connection.ssl != nullptr)
			SSL_free(connection.ssl);
		if (connection.socket != INVALID_SOCKET)
			closesocket(connection.socket);
		if (wsaBuffer.buf != nullptr)
			delete[] wsaBuffer.buf;
	}
};

struct Connection
{
	SOCKET socket;
	SSL* ssl;
	ULONG address;
	UserAuthentication authenticityStatus;

	Connection()
		: Connection(INVALID_SOCKET, nullptr, ULONG{}, UserAuthentication::INVALID)
	{}
	Connection(SOCKET s, SSL* sslc, ULONG addr, UserAuthentication auth)
		: socket{s},
		ssl{sslc},
		address{addr},
		authenticityStatus{auth}
	{}
};