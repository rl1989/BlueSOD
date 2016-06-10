#pragma once
#include "Server.h"

using std::move;
using std::vector;

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
	while (GetState() == ServerState::RUNNING)
	{
		if (m_clientList.size() != 0)
		{
			fd_set read;
			int numIncoming;
			ZeroMemory(&read, sizeof(read));
			for (auto it = m_clientList.begin(); it != m_clientList.end(); it++)
			{
				FD_SET(it->connection.socket, &read);
			}
			numIncoming = select(&read, nullptr, nullptr);
			if (numIncoming == SOCKET_ERROR)
			{

			}

			vector<char[BUFFER_SIZE]> readBuffer{ numIncoming };
			while (numIncoming != 0)
			{

				numIncoming--;
			}
		}

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
