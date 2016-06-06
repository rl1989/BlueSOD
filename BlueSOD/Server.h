#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <mutex>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"

#define BEGIN_MESSAGE "{beg}"
#define END_MESSAGE "{end}"
#define IMAGE_MESSAGE "{image}"
#define END_IMAGE_MESSAGE "{/image}"


//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients. This design allows the Server to handle only one type of
//communication.
//TO DO: Almost everything.
class Server
{
private:
	//List of clients currently connected.
	std::vector<ConnectionInfo> m_clientList;
	std::mutex m_clientListMutex;
	//The state of the Server.
	ThreadSafe<ServerState> m_state;
	//Represents the client being "worked" on
	int m_client;
	SQLiteDb m_db;

public:
	//Constructor takes in the unique information regarding the initial connecting client.
	~Server() {}

	//Add a client to the server.
	void AddClient(ConnectionInfo&& client);
	
	//Returns the number of clients currently connected.
	int NumberOfClients();
	//Initializes the server. This is where the code for handling communication will be.
	//TO DO: Implement.
	void Run(ServerState state = ServerState::RUNNING);
	//Sets the state of the Server.
	inline void SetState(ServerState state);
	inline ServerState GetState();

private:
	ConnectionInfo RetrieveNextClient();
};

