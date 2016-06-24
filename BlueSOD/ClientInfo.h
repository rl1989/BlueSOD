#pragma once
#include "ServerConnections.h"
#include <string>
#include <mutex>
#include <chrono>
#include "Flags.h"

class ClientInfo
{
private:
	std::string m_username{};
	ConnectionInfo m_connectionInfo{};
	std::chrono::time_point<std::chrono::steady_clock> m_lastAccessed{ std::chrono::steady_clock::now() };
public:
	ClientInfo(ConnectionInfo&& ci, const std::string& username);
	~ClientInfo() = default;

	const std::string& GetUsername();
	ConnectionInfo* GetConnectionInfo();
	std::chrono::time_point<std::chrono::steady_clock> LastAccessed();
	void CloseConnection();
	ConnectionState SendMsg(const std::string& msg);
	ConnectionState ReceiveMsg(std::string& msg);
private:
	void Accessed();
};