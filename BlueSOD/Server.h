#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <memory>
#include "ServerConnections.h"
#include "ServerConcurrency.h"

using std::vector;
using std::unique_ptr;

//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients. This design allows the Server to handle only one type of
//communication.
//TO DO: Almost everything.
class Server
{
private:
	//List of clients currently connected.
	vector<unique_ptr<ConnectionInfo>> m_clients;
	//The state of the Server.
	ServerState m_state;

	shared_mutex m_stateMutex;
	shared_mutex m_accessMutex;

public:
	//Constructor takes in the unique information regarding the initial connecting client.
	~Server() {}

	//Add a client to the server.
	void AddClient(const Connection& client);
	
	//Returns the number of clients currently connected.
	int NumberOfClients();
	//Initializes the server. This is where the code for handling communication will be.
	//TO DO: Implement.
	void Run();
	//Sets the state of the Server.
	inline void SetState(ServerState state)
	{
		lock_guard<shared_mutex> lck(m_stateMutex);

		m_state = state;
	}
	inline ServerState GetState()
	{
		lock_guard<shared_mutex> lck(m_stateMutex);

		return m_state;
	}
	inline bool IsRunning()
	{
		return GetState() == ServerState::RUNNING;
	}
	void ReconnectWithClientsOn(int port);

private:
	
};

