#pragma once
#include "Server.h"

using std::move;

void Server::AddClient(ConnectionInfo&& client)
{
	lock_guard<mutex> lck(m_clientListMutex);

	m_clientList.push_back(client);
}

int Server::NumberOfClients()
{
	return m_clientList.size();
}

void Server::Run(ServerState state)
{
	SetState(state);
	while (state == ServerState::RUNNING)
	{
		

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

ConnectionInfo Server::RetrieveNextClient()
{
	lock_guard<mutex> lck(m_clientListMutex);

	m_client++;
	if (m_client > m_clientList.size())
		m_client = 0;
	return m_clientList[m_client];
}
