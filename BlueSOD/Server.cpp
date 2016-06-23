#pragma once
#include "Server.h"

using std::lock_guard;
using std::mutex;
using std::string;
using std::move;
using std::list;

#define CONVERSATIONS_IN_VIEW 8

//Used for FindSortedPosition when inserting new information into m_clientList.
#define USERNAME_IN_LIST -1

extern const string RESET_CONNECTION;

void Server::DisconnectClients()
{
	/*Deleting all of the objects causes their destructor to be called which will finish sending any
	unsent information.*/
	m_newClientList.RemoveAll();
}

void Server::BroadCastMessage(const std::string& msg)
{
	//Need to implement.
	/*for (auto it = m_newClientList.Begin(); it != m_newClientList.End(); it++)
	{
		
	}*/
}

bool Server::AddClient(NewConnectionInfo& ci, const std::string& username)
{
	return m_newClientList.Add(move(ci), username);
}

void Server::Run(ServerState state)
{
	lock_guard<mutex> lck(m_runMetx);

	SetState(state);
	while (state != ServerState::OFF)
	{
		switch (state)
		{
			case ServerState::RUNNING:
				for (auto it = m_newClientList.Begin(); it != m_newClientList.End(); it++)
				{
					NewConnectionInfo* ci = it->GetConnectionInfo();
					string msg;
					
					switch (ci->Receive(msg))
					{
						case connect_s::ERR:
							/*Destroy the object.*/
							m_newClientList.Erase(it);
							break;
						case connect_s::SHUTDOWN:
							/*Destroy the object.*/
							m_newClientList.Erase(it);
							break;
						case connect_s::RECEIVED:
							/*Begin processing the message. Must determine what information the client needs, if any.*/
							switch (ParseMessage(msg))
							{
								case message_t::MESSAGE:
									break;
								case message_t::REQUEST_CONVO_LISTS:
									break;
								case message_t::REQUEST_CONVO_MSGS:
									break;
								case message_t::LOGIN:
									break;
								case message_t::LOGOUT:
									break;
								case message_t::INVALID:
									break;
							}
							break;
						case connect_s::NO_DATA_PRESENT:
							/*Do nothing.*/
							break;
						case connect_s::WANT_READ:
							/*As per OpenSSL's documentation, we must call the function again.*/
							ci->Receive(msg);
							break;
					}
				}
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
	return m_newClientList.Size();
}
