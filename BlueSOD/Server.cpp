#pragma once
#include "Server.h"

//Sizes the vector with the anticipation that more connections will come.
#define DEFAULT_VECTOR_SIZE 21

Server::Server(const Connection& connection)
{
	//Build the initial connecting client.
	ClientInfo clientInfo{ connection };

	//Add the client to the list.
	m_clients = vector<ClientInfo>(DEFAULT_VECTOR_SIZE);
	m_clients.push_back(clientInfo);
}

void Server::AddClient(const Connection& client)
{
	//Obtain the client information.
	ClientInfo clientInfo{ client };

	//Add client to the list.
	m_accessMutex.lock_shared();
	m_clients.push_back(clientInfo);
	m_accessMutex.unlock_shared();
}

int Server::NumberOfClients()
{
	std::lock_guard<shared_mutex> lck(m_accessMutex);

	return static_cast<int>(m_clients.size());
}

void Server::Run()
{

}