#pragma once
#include <WinSock2.h>
#include <openssl\ssl.h>
#include <vector>
#include <list>
#include <mutex>
#include <string>
#include "ClientInfo.h"
#include "ServerConnections.h"

class ClientInfoList
{
private:
	std::vector<ClientInfo> m_list{};
	std::mutex m_mutex{};
public:
	bool Add(ConnectionInfo&& ci, const std::string& username);
	bool Add(ClientInfo&& ci);
	void Remove(SOCKET socket);
	void Remove(SSL* ssl);
	void Remove(const std::string& username);
	void Remove(ClientInfo& ci);
	void Remove(ConnectionInfo& ci);
	void RemoveAll();

	ClientInfo& operator[](int pos);
	ClientInfo& operator[](SOCKET socket);
	ClientInfo& operator[](SSL* ssl);
	ClientInfo& operator[](const std::string& username);

	int Size();
	typename std::vector<ClientInfo>::iterator Begin();
	typename std::vector<ClientInfo>::iterator End();
	void Erase(typename std::vector<ClientInfo>::iterator e);
};