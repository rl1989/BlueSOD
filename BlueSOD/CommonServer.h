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
	SOCKET socket;
	SSL* ssl;
	//Required for WSA messages.
	WSABUF wsaBuffer;
	//These determine how much was sent/received.
	DWORD bytesSend;
	DWORD bytesRecv;
	UserAuthentication auth;
	ULONG address;

	ClientInfo(ClientInfo&& info)
		: socket{ info.socket },
		ssl{ info.ssl },
		bytesRecv{ info.bytesRecv },
		bytesSend{ info.bytesSend },
		auth{ info.auth },
		address{ info.address }
	{
		wsaBuffer.buf = info.wsaBuffer.buf;
		wsaBuffer.len = info.wsaBuffer.len;
		info.wsaBuffer.buf = nullptr;
	}
	ClientInfo(const ClientInfo& info) = default;
	ClientInfo(SOCKET s, SSL* sslObject, UserAuthentication user_auth)
		: socket{ s },
		ssl{ sslObject },
		bytesSend{},
		bytesRecv{},
		auth { user_auth },
		address{}
	{
		wsaBuffer.buf = new char[BUFFER_SIZE];
		wsaBuffer.len = 0;
		bytesRecv = bytesSend = 0;
	}
	ClientInfo()
		: ClientInfo(INVALID_SOCKET, nullptr, UserAuthentication::INVALID) {}

	~ClientInfo()
	{
		if (ssl != nullptr)
			SSL_free(ssl);
		if (socket != INVALID_SOCKET)
			closesocket(socket);
		if (wsaBuffer.buf != nullptr)
			delete[] wsaBuffer.buf;
	}
};