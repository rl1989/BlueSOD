#pragma once
#include <memory>
#include <openssl\ssl.h>
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

	ClientInfo()
		: socket{ INVALID_SOCKET },
		ssl{ nullptr },
		wsaBuffer{},
		bytesSend{ 0 },
		bytesRecv{ 0 },
		auth { UserAuthentication::INVALID }
	{
		wsaBuffer.buf = nullptr;
		wsaBuffer.len = 0;
	}

	~ClientInfo()
	{
		if (ssl != nullptr)
			SSL_free(ssl);
	}
};