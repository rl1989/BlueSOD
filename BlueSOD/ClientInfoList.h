#pragma once
#include <WinSock2.h>
#include <openssl\ssl.h>
#include <list>
#include <mutex>
#include <string>
#include "ClientInfo.h"
#include "ServerConnections.h"

class ClientInfoList
{
private:
	std::list<ClientInfo> m_list{};
	std::mutex m_mutex{};
public:
	void Add(const ConnectionInfo& ci, const std::string& username);
	void Remove(SOCKET socket);
	void Remove(SSL* ssl);
	void Remove(const std::string& username);
};

