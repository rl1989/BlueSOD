#pragma once
#include <mutex>
#include <string>
#include <list>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"
#include "ClientInfoList.h"
#include "Messages.h"

#define BEGIN_MESSAGE "{beg}"
#define END_MESSAGE "{end}"
#define IMAGE_MESSAGE "{image}"
#define END_IMAGE_MESSAGE "{/image}"

#define MSG_DB_NAME "MessageDatabase"
#define TABLE_NAME "Conversations"

enum class message_t
{
	MESSAGE, REQUEST_CONVO_LISTS, REQUEST_CONVO_MSGS, LOGIN, LOGOUT, INVALID
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
	ClientInfoList m_newClientList{};
	//The state of the Server.
	ThreadSafe<ServerState> m_state{ ServerState::OFF };
	SQLiteDb* m_messageDb;
	std::mutex m_msgDbMutex;
	std::mutex m_runMetx;

	/*static SQLiteDb* m_msgDb;
	static int NUMBER_OF_SERVERS;
	static std::shared_mutex DB_MUTEX;*/


public:
	Server() = default;
	~Server() = default;
	
	bool AddClient(NewConnectionInfo& ci, const std::string& username);
	//Initializes the server. This is where the code for handling communication will be.
	//TO DO: Implement.
	void Run(ServerState state = ServerState::RUNNING);
	//Sets the state of the Server.
	inline void SetState(ServerState state);
	inline ServerState GetState();
	int NumberOfClients();

private:
	void DisconnectClients();
	/*Not finished.*/
	void BroadCastMessage(const std::string& msg);
	message_t ParseMessage(std::string& msg);
};

