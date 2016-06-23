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
	NewConnectionInfo m_connectionInfo{};
	std::chrono::time_point<std::chrono::steady_clock> m_lastAccessed{ std::chrono::steady_clock::now() };
public:
	ClientInfo(NewConnectionInfo&& ci, const std::string& username);
	~ClientInfo() = default;

	const std::string& GetUsername();
	NewConnectionInfo* GetConnectionInfo();
	std::chrono::time_point<std::chrono::steady_clock> LastAccessed();
	void CloseConnection();
	connect_s SendMsg(const std::string& msg);
	connect_s ReceiveMsg(std::string& msg);
private:
	void Accessed();
};