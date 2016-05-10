#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include "CommonServer.h"
#include "ServerConcurrency.h"

//The maximum size of a message sent from/to the client
#define BUFFER_SIZE 2048

using std::vector;

extern shared_mutex serverMutex;

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

	ClientInfo()
		: socket{INVALID_SOCKET},
		ssl{ nullptr },
		wsaBuffer{},
		bytesSend{0},
		bytesRecv{0}
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

//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients. This design allows the Server to handle only one type of
//communication.
//TO DO: Almost everything.
class Server
{
private:
	//List of clients currently connected.
	vector<ClientInfo> m_clients;
	//The state of the Server.
	ServerState m_state;

public:
	//Constructor takes in the unique information regarding the initial connecting client.
	Server(SOCKET client, SSL* ssl);
	~Server() {}

	//Add a client to the server.
	void AddClient(SOCKET client, SSL* ssl);
	//Returns the number of clients currently connected.
	int NumberOfClients();
	//Initializes the server. This is where the code for handling communication will be.
	//TO DO: Implement.
	void Run();
	//Sets the state of the Server.
	void SetServerState(ServerState state)
	{
		serverStateMutex.lock_shared();
		m_state = state;
		serverStateMutex.unlock_shared();
	}

private:
	
};

