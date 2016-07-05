#pragma once
#include <mutex>
#include <string>
#include <list>
#include <utility>
#include <deque>
#include <vector>
#include <set>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"
#include "ClientInfo.h"
#include "Messages.h"
#include "TS_Queue.h"
#include "ThreadSafeVector.h"

#define MSG_DB_NAME "MessageDatabase"
#define TABLE_NAME "ConversationsTable"
#define USER_TABLE "UsersTable"
#define UT_USERNAME "Username"

/*The MessageDEQueue is a double ended queue of strings. It is a double ended queue of strings because incoming messages
 are placed at the end and outgoing messages are from the front.*/
typedef typename ThreadSafeQueue<MessageBase> IncMsgQueue;
typedef typename ThreadSafeQueue<MessageBase> OutMsgQueue;
typedef std::pair<IncMsgQueue, OutMsgQueue> MsgQueues;
typedef std::pair<ClientInfo, MsgQueues> ClientAndQueues;
/*The ClientAndMessageQueue is a pair of ClientInfo and MessageDEQueue, effectively associating a ClientInfo and a 
  MessageDEQueue*/

//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients. This design allows the Server to handle only one type of
//communication.
//TO DO: Almost everything.
class Server
{
private:
	ThreadSafeVector<ClientAndQueues> m_clientMessages;
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
};

