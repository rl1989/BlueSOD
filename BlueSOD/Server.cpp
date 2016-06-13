#pragma once
#include "Server.h"

using std::lock_guard;
using std::mutex;

Server::Server()
{
}

void Server::AddClient(const ConnectionInfo& ci, const std::string& username)
{

}

void Server::Run(ServerState state)
{
	SetState(state);
	while (GetState() == ServerState::RUNNING)
	{
		
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