#pragma once
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

	ClientInfo(SOCKET s, SSL* sslObject, UserAuthentication user_auth)
		: socket{ s },
		ssl{ sslObject },
		wsaBuffer{},
		bytesSend{ 0 },
		bytesRecv{ 0 },
		auth { user_auth }
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
		delete[] wsaBuffer.buf;
	}
};