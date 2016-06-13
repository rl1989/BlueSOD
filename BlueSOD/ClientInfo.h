#pragma once
#include "ServerConnections.h"
#include <string>
#include <mutex>

class ClientInfo
{
private:
	std::string m_username;
	ConnectionInfo m_connectionInfo;
	bool active;
public:
	ClientInfo(const ConnectionInfo& ci, const std::string& username);
	ClientInfo();
	~ClientInfo();
};