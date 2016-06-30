#pragma once
#include "Server.h"

using std::lock_guard;
using std::mutex;
using std::string;
using std::move;
using std::list;
using std::deque;
using std::vector;

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

void Server::DisconnectClients()
{
	lock_guard<mutex> lck(m_pairMutex);
	/*Deleting all of the objects causes their destructor to be called which will finish sending any
	unsent information.*/
	m_clientMessages.clear();
}

void Server::BroadCastMessage(const string& msg)
{
	lock_guard<mutex> lck(m_pairMutex);

	for (int i = 0; i < m_clientMessages.size(); i++)
	{
		m_clientMessages[i].second.push_back(msg);
	}
}

void Server::RecordMessages(ClientAndMessageDeque* cm)
{
	string username = cm->first.GetUsername();

	while (cm->second.size() > 0)
	{
		WriteMsgToDb(username, cm->second.front());
		cm->second.pop_front();
	}
}

void Server::WriteMsgToDb(const string& username, const string& msg)
{
	lock_guard<mutex> lck(m_msgDbMutex);

	switch (MessageBase::GetMessageType(msg))
	{
		case message_t::MESSAGE:
			{
				MessageToUser mtu = MessageToUser::ParseMessageToUser(msg);
				string query{ "INSERT INTO " };
				query += username + COMMON_TABLE_NAME;
				query += " VALUES(";
				query += TO_COLUMN;
				query += "='" + mtu.To() + "',";
				query += FROM_COLUMN;
				query += "='" + mtu.From() + "',";
				query += MSG_COLUMN;
				query += "='" + mtu.Message() + "',";
				query += DATE_COLUMN;
				query += "='" + mtu.Date() + "',";
				query += STATUS_COLUMN;
				query += "='" + mtu.Status() + "');";

				m_messageDb.ExecuteStatement(query);
			}
			break;
	}
}

void Server::ProcessIncomingMessages()
{
	m_pairMutex.lock();
	for (auto it = m_clientMessages.begin(); it != m_clientMessages.end(); it++)
	{
		m_pairMutex.unlock();
		string msg;

		RecvMsg(it, msg);
		m_pairMutex.lock();
	}
	m_pairMutex.unlock();
}

void Server::SendQueuedMessages()
{
	m_pairMutex.lock();
	for (auto it = m_clientMessages.begin(); it != m_clientMessages.end(); it++)
	{
		m_pairMutex.unlock();
		while (it->second.size() > 0)
		{
			SendMsg(it, it->second.front());
			it->second.pop_front();
		}
		m_pairMutex.lock();
	}
	m_pairMutex.unlock();
}

string Server::BuildUnsentMessagesQuery(const string& to)
{
	string query = "SELECT ";
	query += FROM_COLUMN;
	query += ",";
	query += MSG_COLUMN;
	query += ",";
	query += DATE_COLUMN;
	query += " FROM ";
	query += to;
	query += COMMON_TABLE_NAME;
	query += " WHERE ";
	query += TO_COLUMN;
	query += "='";
	query += to;
	query += "' AND ";
	query += STATUS_COLUMN;
	query += "='";
	query += UNSENT_STATUS;
	query += "';";

	return query;
}

string Server::BuildMessage(const string& from, const string& to, const string& message, const string& date)
{
	string msg = MSG_BTWN_USERS;
	msg += DELIMITER;
	msg += FROM_FIELD;
	msg += from;
	msg += DELIMITER;
	msg += TO_FIELD;
	msg += to;
	msg += DELIMITER;
	msg += MESSAGE_FIELD;
	msg += message;
	msg += DELIMITER;
	msg += DATE_FIELD;
	msg += date;
	msg += DELIMITER;
	msg += END_OF_MSG;
	return msg;
}

void Server::SendMsg(vector<ClientAndMessageDeque>::iterator cm, const string& msg)
{
	switch (cm->first.SendMsg(msg))
	{
		case ConnectionState::ERR:
			cm->second.push_back(msg);
			RecordMessages(cm._Ptr);
			m_clientMessages.erase(cm);
			break;
		case ConnectionState::NOT_FULLY_SENT:
			/*Place the rest of the unsent message in the message deque.*/
			cm->second.push_front(msg.substr(cm->first.GetConnectionInfo()->BytesSent()));
			break;
		case ConnectionState::NOT_SENT:
			cm->second.push_back(msg);
			break;
		case ConnectionState::WANT_READ:
		case ConnectionState::WANT_WRITE:
			cm->second.push_front(msg);
			break;
	}
}

void Server::RecvMsg(vector<ClientAndMessageDeque>::iterator cm, string& msgBuffer)
{
	ClientInfo* ci = &(cm->first);

	switch (ci->ReceiveMsg(msgBuffer))
	{
		case ConnectionState::ERR:
			/*Record any unsent messages.*/
			RecordMessages(cm._Ptr);
			/*Destroy the object.*/
			m_clientMessages.erase(cm);
			break;
		case ConnectionState::SHUTDOWN:
			/*Record any unsent messages.*/
			RecordMessages(cm._Ptr);
			/*Destroy the object.*/
			m_clientMessages.erase(cm);
			break;
		case ConnectionState::RECEIVED:
			/*Process the message. Must determine what information the client needs, if any.*/
			switch (MessageBase::GetMessageType(msgBuffer))
			{
				case message_t::MESSAGE:
				{
					MessageToUser mtu = MessageToUser::ParseMessageToUser(msgBuffer);
					if (mtu.From() != cm->first.GetUsername())
					{
						cm->first.SendMsg(INVALID_MSG);
					}
					else
					{
						vector<ClientAndMessageDeque>::iterator it = FindUserInQueue(mtu.To());
						if (it != m_clientMessages.end())
						{
							it->second.push_back(BuildMessage(mtu));
						}
						else if (FindUserInDb(mtu.To()))
						{
							/*Build query to add the message to mtu.To()'s table.*/
							string query{"INSERT INTO "};
							query += mtu.To() + TABLE_NAME;
							query += "(";
							query += TO_COLUMN;
							query += ",";
							query += FROM_COLUMN;
							query += ",";
							query += MSG_COLUMN;
							query += ",";
							query += DATE_COLUMN;
							query += ",";
							query += STATUS_COLUMN;
							query += ") VALUES(";
							query += mtu.To() + "," + mtu.From() + "," + mtu.Message() + "," + mtu.Date() + "," + mtu.Status();
							query += ");";

							m_messageDb.ExecuteStatement(query);
						}
						else
						{
							string msg = USER_DOESNT_EXIST;
							msg += DELIMITER;
							msg += USERNAME_FIELD;
							msg += mtu.To();
							cm->first.SendMsg(msg);
						}
					}
				}
				break;
				case message_t::REQUEST_CONVO_LISTS:
				{
					/*
					*
					*
					*		STOPPED
					*		HERE
					*
					*
					*/
					RequestConversationLists rcl = RequestConversationLists::ParseRequest(msgBuffer);
				}
				break;
				case message_t::REQUEST_CONVO_MSGS:
				{
					RequestConversationMsgs rcm = RequestConversationMsgs::ParseRequest(msgBuffer);
				}
				break;
				case message_t::LOGOUT:
				{
					LogoutMessage lm = LogoutMessage::ParseMessage(msgBuffer);
				}
				break;
				case message_t::INVALID:
				{
					cm->second.push_back(INVALID_MSG_RESPONSE);
				}
				break;
				case message_t::REQUEST_FILE:
				{
					FileRequestMessage frm = FileRequestMessage::ParseMessage(msgBuffer);
				}
				break;
				case message_t::REQUEST_USER_INFO:
				{
					UserInfoRequest uir = UserInfoRequest::ParseMessage(msgBuffer);
				}
				break;
			}
			break;
		case ConnectionState::NO_DATA_PRESENT:
			/*Do nothing.*/
			break;
		case ConnectionState::WANT_WRITE:
		case ConnectionState::WANT_READ:
			/*Do nothing.*/
			break;
	}
}

bool Server::AddClient(ClientInfo&& ci)
{
	string username = ci.GetUsername();
	lock_guard<mutex> lck(m_pairMutex);

	for (auto it = m_clientMessages.begin(); it != m_clientMessages.end(); it++)
	{
		if (username == it->first.GetUsername())
			return false;
	}

	//m_clientMessages.push_back(ClientMessages{ move(ci), queue<string>{} });

	/*Need to fill the queue with any unsent messages.*/
	deque<string> unsentMessages{};

	m_msgDbMutex.lock();
	{
		m_messageDb.ExecuteStatement(BuildUnsentMessagesQuery(username));
		
		while (m_messageDb.HasRows())
		{
			unsentMessages.push_back(BuildMessage(m_messageDb.GetColumnTxt(0), username, m_messageDb.GetColumnTxt(1), m_messageDb.GetColumnTxt(2)));
			m_messageDb.StepNextRow();
		}
	}
	m_msgDbMutex.unlock();

	m_clientMessages.push_back(ClientAndMessageDeque{ move(ci), unsentMessages });

	return true;
}

void Server::Run(ServerState state)
{
	if (!m_runMetx.try_lock())
		return;

	SetState(state);
	while (state != ServerState::OFF)
	{
		switch (state)
		{
			case ServerState::RUNNING:
				ProcessIncomingMessages();
				SendQueuedMessages();
				break;
			case ServerState::NOT_ACCEPTING_CONNECTIONS:
				DisconnectClients();
				break;
			case ServerState::RESET:
				//BroadCastMessage(RESET_CONNECTION);
				DisconnectClients();
				SetState(ServerState::RUNNING);
				break;
		}

		state = GetState();
	}

	m_runMetx.unlock();
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
	return m_clientMessages.size();
}
