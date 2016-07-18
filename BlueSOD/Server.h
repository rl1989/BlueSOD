#pragma once
#include <mutex>
#include <string>
#include <list>
#include <utility>
#include <deque>
#include <vector>
#include <set>
#include <thread>
#include <future>
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"
#include "ClientInfo.h"
#include "Messages.h"
#include "TS_Queue.h"
#include "ThreadSafeVector.h"
#include "ThreadSafeList.h"

#define MSG_DB_NAME "MessageDatabase"
#define TABLE_NAME "ConversationsTable"
#define USER_TABLE "UsersTable"
#define UT_USERNAME "Username"

/*
	These represent the different queues a connection will use to store and retrieve messages.
	An OutMsgQueue holds messages the server needs to send to the client. The IncMsgQueue is the
	queue a client places incoming messages.
*/
typedef ThreadSafeQueue<MessageBase*> Queue;
typedef Queue OutMsgQueue;
typedef Queue IncMsgQueue;
typedef std::pair<IncMsgQueue, OutMsgQueue> MsgQueues;
typedef std::pair<ClientInfo, MsgQueues> ClientAndQueues;

//This class represents the Server. The ServerManager will handle any initial incoming connections
//and will pass it off to the Server, which lies in its own thread. The Server then will handle
//the communication between clients.
class Server
{
private:
	static const int DEFAULT_VECTOR_SIZE = 20;
	ThreadSafeVector<ClientAndQueues> m_clientList{DEFAULT_VECTOR_SIZE};
	//The state of the Server.
	ThreadSafe<ServerState> m_state{ ServerState::OFF };
	SQLiteDb m_messageDb{};
	std::mutex m_msgDbMutex{};
	/*Restricts Run(1) to be called one at a time.*/
	std::mutex m_RunMutex{};
	std::vector<std::thread> m_receiverThreads{DEFAULT_VECTOR_SIZE};

	ThreadSafeList<std::pair<int, Server*>>* m_masterList{nullptr};

public:
	Server() = default;
	~Server() = default;
	Server(ThreadSafeList<std::pair<int,Server*>>* master_list);
	
	bool AddClient(const ClientInfo& ci);
	void Run(ServerState state = ServerState::OFF);
	//Sets the state of the Server.
	inline void SetState(ServerState state);
	inline ServerState GetState();
	int NumberOfClients();

private:
	void SaveMessages(ThreadSafeQueue<MessageBase*>&& messages);
	void SearchForMessages(OutMsgQueue& messages, int userId, const std::string& username);

	/*
		Process the different incoming request messages. Each function is intended to run asynchronously. Because
		the pointer being passed in can potentially be used longer than the calling code lasts, each function will
		be tasked with deleting the pointer passed in.
	*/
	/*Send the message from From() to To() if To() is a valid user. If the valid user is logged in, it will
	  add it to its associated queue. If not, then the message will be recorded in the database for later use.*/
	void ProcessMessageToUser(MessageToUser* mtu);
	/*This will find all conversations associated with rcl->For() and add them to the queue.*/
	void FindConversationList(RequestConversationListMessage* rcl, OutMsgQueue* queue);
	/*This will find the messages associated with the conversation between rcm->Requester() and rcm->Requested()
	  and add them to the client's queue.*/
	void FindConversation(RequestConversationMsgs* rcm, OutMsgQueue* queue);
	/*This will find the file being requested and add it to the client's queue.*/
	void FindFile(FileRequestMessage* frm, OutMsgQueue* queue);
	/*This will locate the information of the user uir->Target() and add it to the client's queue.*/
	void FindUserInformation(UserInfoRequest* uir, OutMsgQueue* queue);

	/*Removes the client from the master list.*/
	void RemoveFromMasterList(int id);
};
