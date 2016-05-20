#pragma once
#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include "CommonServer.h"
#include "ServerConcurrency.h"

using std::vector;

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

	shared_mutex m_stateMutex;
	shared_mutex m_accessMutex;

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
		m_stateMutex.lock();
		m_state = state;
		m_stateMutex.unlock();
	}
	void ReconnectWithClientsOn(int port);

private:
	
};

