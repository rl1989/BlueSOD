#pragma once
#include <mutex>
#include <string>
#include <list>
#include <utility>
#include <deque>
#include <vector>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"
#include "ClientInfo.h"
#include "Messages.h"

#define MSG_DB_NAME "MessageDatabase"
#define TABLE_NAME "Conversations"

//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients. This design allows the Server to handle only one type of
//communication.
//TO DO: Almost everything.
class Server
{
private:
	typedef std::pair<ClientInfo, std::deque<std::string>> ClientAndMessageDeque;
	std::vector<ClientAndMessageDeque> m_clientMessages;
	std::mutex m_pairMutex;
	//The state of the Server.
	ThreadSafe<ServerState> m_state{ ServerState::OFF };
	SQLiteDb m_messageDb;
	std::mutex m_msgDbMutex;
	std::mutex m_runMetx;

	/*static SQLiteDb* m_msgDb;
	static int NUMBER_OF_SERVERS;
	static std::shared_mutex DB_MUTEX;*/


public:
	Server() = default;
	~Server() = default;
	
	inline bool AddClient(ClientInfo&& ci);
	void Run(ServerState state = ServerState::OFF);
	//Sets the state of the Server.
	inline void SetState(ServerState state);
	inline ServerState GetState();
	int NumberOfClients();

private:
	void DisconnectClients();
	void BroadCastMessage(const std::string& msg);
	void RecordMessages(ClientAndMessageDeque* cm);
	void WriteMsgToDb(const std::string& username, const std::string& msg);
	void ProcessIncomingMessages();
	void SendQueuedMessages();
	std::string BuildUnsentMessagesQuery(const std::string& to);
	std::string BuildMessage(const std::string& from, const std::string& to, const std::string& message, const std::string& date);
	std::string BuildMessage(const MessageToUser& mtu);
	void SendMsg(std::vector<ClientAndMessageDeque>::iterator cm, const std::string& msg);
	void RecvMsg(std::vector<ClientAndMessageDeque>::iterator cm, std::string& msgBuffer);
	std::vector<ClientAndMessageDeque>::iterator FindUserInQueue(const std::string& username);
	bool FindUserInDb(const std::string& user);
};

