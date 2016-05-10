#include "Server.h"

//Sizes the vector with the anticipation that more connections will come.
#define DEFAULT_VECTOR_SIZE 21

Server::Server(SOCKET client, SSL* ssl)
{
	//Build the initial connecting client.
	ClientInfo clientInfo;
	clientInfo.socket = client;
	clientInfo.ssl = ssl;

	//Add the client to the list.
	m_clients = vector<ClientInfo>(DEFAULT_VECTOR_SIZE);
	m_clients.push_back(clientInfo);
}

void Server::AddClient(SOCKET client, SSL* ssl)
{
	//Obtain the client information.
	ClientInfo clientInfo;
	clientInfo.socket = client;
	clientInfo.ssl = ssl;

	//Add client to the list.
	serverMutex.lock_shared();
	m_clients.push_back(clientInfo);
	serverMutex.unlock_shared();
}

int Server::NumberOfClients()
{
	int size;
	
	//I don't know if locking the resource is necessary for a read.
	serverMutex.lock_shared();
	size = m_clients.size();
	serverMutex.unlock_shared();
	return size;
}

void Server::Run()
{

}