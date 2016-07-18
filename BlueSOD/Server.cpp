#pragma once
#include "Server.h"

using std::move;
using std::string;
using std::thread;
using std::vector;
using std::async;

#define COMMON_TABLE_NAME "Messages"
#define TO_COLUMN         "To"
#define FROM_COLUMN       "From"
#define MSG_COLUMN        "Message"
#define DATE_COLUMN       "Date"
#define STATUS_COLUMN     "Status"
#define UNSENT_STATUS     "Unsent"

#define TO_COL_POS     0
#define FROM_COL_POS   1
#define MSG_COL_POS    2
#define DATE_COL_POS   3
#define STATUS_COL_POS 4

void client_handler(ClientInfo* ci, ThreadSafe<ServerState>* ts_state, IncMsgQueue* inc, OutMsgQueue* out)
{
	ServerState state = ts_state->RetrieveObject();

	while (ci->GetConnectionInfo()->IsValid() && state != ServerState::OFF && state != ServerState::RESET)
	{
		string msg;
		
		switch (ci->ReceiveMsg(msg))
		{
			case ConnectionState::RECEIVED:
				switch (MessageBase::GetMessageType(msg))
				{
					case message_t::MESSAGE_TO_USER:
						inc->push(MessageToUser::ParseMessageToUser(msg));
						break;
					case message_t::LOGIN:
						inc->push(LoginMessage::ParseLoginMsg(msg));
						break;
					case message_t::LOGOUT:
						inc->push(LogoutMessage::ParseMessage(msg));
						break;
					case message_t::REQUEST_CONVO_LISTS:
						inc->push(RequestConversationListMessage::ParseRequest(msg));
						break;
					case message_t::REQUEST_CONVO_MSGS:
						inc->push(RequestConversationMsgs::ParseRequest(msg));
						break;
					case message_t::REQUEST_FILE:
						inc->push(FileRequestMessage::ParseMessage(msg));
						break;
					case message_t::REQUEST_USER_INFO:
						inc->push(UserInfoRequest::ParseMessage(msg));
						break;
					case message_t::INVALID:
						inc->push(new InvalidMessage{});
						break;
				}
				break;
			case ConnectionState::SHUTDOWN:
				ci->CloseConnection();
				break;
			case ConnectionState::WANT_READ:
			case ConnectionState::NO_DATA_PRESENT:
				break;
			case ConnectionState::ERR:
			default:
				ci->CloseConnection();
				break;
		}

		if (!out->empty())
		{
			MessageBase* nextMsg = out->front();

			switch (nextMsg->Code())
			{
				case message_t::MESSAGE_TO_USER:
					msg = ((MessageToUser*)nextMsg)->MakeMessage();
					break;
				case message_t::LOGIN:
					msg = ((LoginMessage*)nextMsg)->MakeResponse();
					break;
				case message_t::LOGOUT:
					msg = ((LogoutMessage*)nextMsg)->MakeResponse();
					break;
				case message_t::CONVERSATION_LIST:
					
					break;
				case message_t::CONVERSATION_MSGS:
					break;
				case message_t::FILE:
					break;
				case message_t::USER_INFO:
					break;
			}
			switch (ci->SendMsg(msg))
			{
				case ConnectionState::SENT:
					out->pop();
					delete nextMsg;
					break;
				case ConnectionState::SHUTDOWN:
					ci->CloseConnection();
					break;
				case ConnectionState::ERR:
					ci->CloseConnection();
					break;
				default:
					break;
			}
		}

		state = ts_state->RetrieveObject();
	}

	/*This won't do anything if ci is Invalid, but must be done if the server is off or resetting.*/
	ci->CloseConnection();
}

Server::Server(ThreadSafeList<std::pair<int, Server*>>* master_list)
	: m_masterList{master_list}
{}

bool Server::AddClient(const ClientInfo& ci)
{
	for (auto it = m_masterList->begin(); it != m_masterList->end(); it++)
	{
		/*user is already logged in*/
		if (it->first == ci.GetId())
			return false;
	}
	
	m_clientList.push_back(ClientAndQueues{ ci, MsgQueues{} });

	auto out = m_clientList.end() - 1;
	/*Fill the outgoing messages queue*/
	async(this->SearchForMessages, out->second.second, ci.GetId(), ci.GetUsername());
	/*Allow a new thread to listen for any communication*/
	m_receiverThreads.push_back(thread{ client_handler, &(out->first), &m_state, &(out->second.first), &(out->second.second) });

	return true;
}

void Server::Run(ServerState state)
{
	lock_guard<mutex> lck(m_RunMutex);

	SetState(state);
	
	while (state != ServerState::OFF)
	{
		/*Used in ServerState::RUNNING case*/
		string msg;

		switch (state)
		{
			case ServerState::START_UP:
				/*Reserve space to increase performance later on*/
				m_clientList.reserve(DEFAULT_VECTOR_SIZE);
				m_receiverThreads.reserve(DEFAULT_VECTOR_SIZE);
				SetState(ServerState::RUNNING);
				break;
			case ServerState::RUNNING:
				/*Process messages*/
				for (auto it = m_clientList.begin(); it != m_clientList.end(); it++)
				{
					string username = it->first.GetUsername();
					IncMsgQueue& inc = it->second.first;
					/*True if inc was erased so is invalid. This will break the while loop so that !inc.empty()
					isn't evaluated again.*/
					bool erased = false;

					/*Process incoming messages*/
					while (!inc.empty() && !erased)
					{
						MessageBase* message = inc.front();

						switch (message->Code())
						{
							case message_t::LOGOUT:
								{
									/*Close the connection*/
									it->first.CloseConnection();
									/*Save the outgoing message queue*/
									async(this->SaveMessages, move(it->second.second));
									/*Remove this client from the master list and this Server's list*/
									async(this->RemoveFromMasterList, it->first.GetId());
									m_clientList.erase(it);
									/*Logout is the only one that doesn't get deleted in an async method.*/
									delete message;

									erased = true;
								}
								break;
							case message_t::MESSAGE_TO_USER:
								{
									MessageToUser* mtu = dynamic_cast<MessageToUser*>(message);
									/*Either add the message to the To user's OutMsgQueue, record the message in the
									  To user's messages table, or add an invalid message to the From user's OutMsgQueue.*/
									async(this->ProcessMessageToUser, mtu);
								}
								break;
							case message_t::REQUEST_CONVO_LISTS:
								{
									RequestConversationListMessage* rcl = dynamic_cast<RequestConversationListMessage*>(message);
									OutMsgQueue* queue = &it->second.second;
									/*Find the conversations and add it to the client's OutMsgQueue.*/
									async(this->FindConversationList, rcl, queue);
								}
								break;
							case message_t::REQUEST_CONVO_MSGS:
								{
									RequestConversationMsgs* rcm = dynamic_cast<RequestConversationMsgs*>(message);
									OutMsgQueue* queue = &it->second.second;
									/*Find the messages between rcm->Requester() and rcm->Requested() and add
									  them to the client's queue.*/
									async(this->FindConversation, rcm, queue);
								}
								break;
							case message_t::REQUEST_FILE:
								{
									FileRequestMessage* frm = dynamic_cast<FileRequestMessage*>(message);
									OutMsgQueue* queue = &it->second.second;
									/*Attempt to locate the file and add it to the client's queue.*/
									async(this->FindFile, frm, queue);
								}
								break;
							case message_t::REQUEST_USER_INFO:
								{
									UserInfoRequest* uir = dynamic_cast<UserInfoRequest*>(message);
									OutMsgQueue* queue = &it->second.second;
									/*Attempt to locate the information on the user and add it to the client's queue.*/
									async(this->FindUserInformation, uir, queue);
								}
								break;
						}
						
						/*We're finished with the message, so remove it from the queue.*/
						if (!erased)
						{
							inc.pop();
						}
					}
				}
				break;
			case ServerState::RESET:
				/*We need to make sure that the state stays at RESET until this process is finished.*/
				m_state.LockChange(ServerState::RESET);

				/*We are waiting until every thread gets the message and joins.*/
				for (auto it = m_receiverThreads.begin(); it != m_receiverThreads.end(); it++)
				{
					if (it->joinable())
						it->join();
				}

				/*Now, unallocate the memory that was used.*/
				m_receiverThreads.clear();
				m_clientList.clear();

				m_state.Unlock();

				SetState(ServerState::START_UP);
				break;
		}

		state = GetState();
	}
}

inline void Server::SetState(ServerState state)
{
	m_state.ChangeObject(state);
}

inline ServerState Server::GetState()
{
	return m_state.RetrieveObject();
}

int Server::NumberOfClients()
{
	return m_clientList.size();
}

void AddToQueue(Queue& queue, const string& msg)
{
	switch (MessageBase::GetMessageType(msg))
	{
		case message_t::INVALID:
			queue.push(new InvalidMessage{});
			break;
		case message_t::LOGOUT:
			queue.push(LogoutMessage::ParseMessage(msg));
			break;
		case message_t::MESSAGE_TO_USER:
			queue.push(MessageToUser::ParseMessageToUser(msg));
			break;
		case message_t::REQUEST_CONVO_LISTS:
			queue.push(RequestConversationListMessage::ParseRequest(msg));
			break;
		case message_t::REQUEST_CONVO_MSGS:
			queue.push(RequestConversationMsgs::ParseRequest(msg));
			break;
		case message_t::REQUEST_FILE:
			queue.push(FileRequestMessage::ParseMessage(msg));
			break;
		case message_t::REQUEST_USER_INFO:
			queue.push(UserInfoRequest::ParseMessage(msg));
			break;
	}
}

void Server::SaveMessages(ThreadSafeQueue<MessageBase*>&& messages)
{
	while (messages.size() > 0)
	{
		MessageBase* msg = messages.front();
		if (msg->Code() == message_t::MESSAGE_TO_USER)
		{
			m_msgDbMutex.lock();
			m_messageDb.ExecuteStatement(msg->MakeQueries());
			m_msgDbMutex.unlock();
		}
		messages.pop();
	}
}

void Server::SearchForMessages(OutMsgQueue& messages, int userId, const string& username)
{
	/*We need to build a query, execute the query, parse the results, and add them to messages*/
	string query = "SELECT message FROM ";
	query += SQLiteDb::MessagesTableName(username) + " WHERE id='";
	query += userId;
	query += "'";

	m_msgDbMutex.lock();
	if (m_messageDb.ExecuteStatement(query) == SQLITE_ROW)
	{
		while (m_messageDb.HasRows())
		{
			string msg = m_messageDb.GetColumnTxt(0);
			AddToQueue(messages, msg);
			m_messageDb.StepNextRow();
		}
	}
	m_msgDbMutex.unlock();
}

void Server::ProcessMessageToUser(MessageToUser* mtu)
{
	bool found = false;

	/*Search for the person the message is intended for.*/
	for (auto it = m_masterList->begin(); it != m_masterList->end(); it++)
	{
		if (it->first == mtu->ToId())
		{
			/*Find the message recipient's outgoing queue and add the message to it.*/
			for (auto iter = it->second->m_clientList.begin(); iter != it->second->m_clientList.end(); iter++)
			{
				if (mtu->ToId() == iter->first.GetId())
				{
					iter->second.second.push(mtu);
					found = true;
					break;
				}
			}
			break;
		}
		if (it->first == mtu->FromId())
		{

		}
	}

	/*If the user isn't logged in, we'll record the message right away.
	  NOTE: The message will be recorded in the database once the user logs out or their connection is severed.*/
	if (!found)
	{
		string query = "SELECT Username FROM user.db WHERE Username='" + mtu->To() + "'";
		m_msgDbMutex.lock();
		if (m_messageDb.ExecuteStatement(query) == SQLITE_ROW)
		{
			m_messageDb.ExecuteStatement(mtu->MakeQueries());
		}
		m_msgDbMutex.unlock();
	}

	/*We're done with the message, so delete it. I don't delete it outside of this method, because it
	  is intended to run asynchronously. The only time it makes sense to delete the pointer is when this
	  method is finished with it.*/
	delete mtu;
}

void Server::FindConversationList(RequestConversationListMessage* rcl, OutMsgQueue* queue)
{
	if (rcl == nullptr || queue == nullptr || rcl->IsValid() == false)
		return;

	vector<string> conversationList;
	
	m_messageDb.Lock();
	/*SELECT * FROM rcl->Requester()Conversations LIMIT 15;*/
	if (m_messageDb.ExecuteStatement(rcl->MakeQueries()) == SQLITE_ROW)
	{
		/*Load the data while there is data available.*/
		while (m_messageDb.HasRows())
		{
			conversationList.push_back(m_messageDb.GetColumnTxt(0));
			m_messageDb.StepNextRow();
		}
	}
	m_messageDb.Unlock();

	queue->push(new ConversationListMessage{ conversationList });
	/*We're done with the message, so delete it. I don't delete it outside of this method, because it
	is intended to run asynchronously. The only time it makes sense to delete the pointer is when this
	method is finished with it.*/
	delete rcl;
}

void Server::FindConversation(RequestConversationMsgs* rcm, OutMsgQueue* queue)
{
	if (rcm == nullptr || queue == nullptr || rcm->IsValid() == false)
		return;

	vector<MessageToUser> messages;

	m_messageDb.Lock();
	/*SELECT * FROM rcm->Requester()Conversations
	  WHERE to='rcm->With()' OR from='rcm->With()' LIMIT 15;*/
	if (m_messageDb.ExecuteStatement(rcm->MakeQueries()) == SQLITE_ROW)
	{
		while (m_messageDb.HasRows())
		{
			string from = m_messageDb.GetColumnTxt(0), 
				   to = m_messageDb.GetColumnTxt(1), 
				   message = m_messageDb.GetColumnTxt(2), 
				   date = m_messageDb.GetColumnTxt(3), 
				   status = m_messageDb.GetColumnTxt(4);
			MessageToUser mtu = MessageToUser::MakeMessageToUser(from, to, message, date, status);

			messages.push_back(move(mtu));

			m_messageDb.StepNextRow();
		}
	}
	m_messageDb.Unlock();

	queue->push(new ConversationMessagesMessage{ rcm->With(), move(messages) });

	/*We're done with the message, so delete it. I don't delete it outside of this method, because it
	is intended to run asynchronously. The only time it makes sense to delete the pointer is when this
	method is finished with it.*/
	delete rcm;
}

void Server::FindFile(FileRequestMessage* frm, OutMsgQueue* queue)
{
	if (frm == nullptr || queue == nullptr || frm->IsValid() == false)
		return;

	m_messageDb.Lock();

	m_messageDb.Unlock();
	/*We're done with the message, so delete it. I don't delete it outside of this method, because it
	is intended to run asynchronously. The only time it makes sense to delete the pointer is when this
	method is finished with it.*/
	delete frm;
}

void Server::FindUserInformation(UserInfoRequest* uir, OutMsgQueue* queue)
{
	if (uir == nullptr || queue == nullptr || uir->IsValid() == false)
		return;

	m_messageDb.Lock();

	m_messageDb.Unlock();
	/*We're done with the message, so delete it. I don't delete it outside of this method, because it
	is intended to run asynchronously. The only time it makes sense to delete the pointer is when this
	method is finished with it.*/
	delete uir;
}

void Server::RemoveFromMasterList(int id)
{
	for (auto iter = m_masterList->begin(); iter != m_masterList->end(); iter++)
	{
		if (iter->first == id)
		{
			m_masterList->erase(iter);
			break;
		}
	}
}
