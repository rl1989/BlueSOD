#pragma once
#include <mutex>
#include <string>
#include <list>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"
#include "ClientInfo.h"

#define BEGIN_MESSAGE "{beg}"
#define END_MESSAGE "{end}"
#define IMAGE_MESSAGE "{image}"
#define END_IMAGE_MESSAGE "{/image}"

enum message_t
{
	TEXT, IMAGE, INVALID
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
	ClientInfo m_clientList;
	//The state of the Server.
	ThreadSafe<ServerState> m_state{ ServerState::OFF };

	SQLiteDb m_msgDb{};


public:
	Server();
	~Server() {}
	
	void AddClient(const ConnectionInfo& ci, const std::string& username);
	//Initializes the server. This is where the code for handling communication will be.
	//TO DO: Implement.
	void Run(ServerState state = ServerState::RUNNING);
	//Sets the state of the Server.
	inline void SetState(ServerState state);
	inline ServerState GetState();

private:
};

