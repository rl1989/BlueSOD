#pragma once
#include "Server.h"

//Sizes the vector with the anticipation that more connections will come.
#define DEFAULT_VECTOR_SIZE 21

Server::Server(const ClientInfo& client)
{
	//Add the client to the list.
	m_clients = vector<ClientInfo>(DEFAULT_VECTOR_SIZE);
	m_clients.push_back(client);
}

void Server::AddClient(const ClientInfo& client)
{
	//Add client to the list.
	m_accessMutex.lock();
	m_clients.push_back(client);
	m_accessMutex.unlock();
}

int Server::NumberOfClients()
{
	std::lock_guard<shared_mutex> lck(m_accessMutex);

	return static_cast<int>(m_clients.size());
}

void Server::Run()
{

}