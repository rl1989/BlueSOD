#pragma once
#include "Server.h"

//Sizes the vector with the anticipation that more connections will come.
#define DEFAULT_VECTOR_SIZE 21

void Server::AddClient(const Connection& client)
{
	
}

int Server::NumberOfClients()
{
	std::lock_guard<shared_mutex> lck(m_accessMutex);

	return static_cast<int>(m_clients.size());
}

void Server::Run()
{

}