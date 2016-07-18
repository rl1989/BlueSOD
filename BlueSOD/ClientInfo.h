#pragma once
#include "ServerConnections.h"
#include <string>
#include <mutex>
#include <chrono>
#include "Flags.h"

class ClientInfo
{
private:
	int m_id{};
	std::string m_username{};
	ConnectionInfo m_connectionInfo{};
	std::chrono::time_point<std::chrono::steady_clock> m_lastAccessed{ std::chrono::steady_clock::now() };
public:
	ClientInfo(const ConnectionInfo& ci, const std::string& username, int id);
	ClientInfo(ConnectionInfo&& ci, const std::string& username, int id);
	~ClientInfo() = default;
	ClientInfo(const ClientInfo& ci);
	ClientInfo& operator=(const ClientInfo& ci);
	ClientInfo(ClientInfo&& move);
	ClientInfo& operator=(ClientInfo&& move);

	const std::string& GetUsername();
	const std::string& GetUsername() const;
	int GetId();
	int GetId() const;
	ConnectionInfo* GetConnectionInfo();
	const ConnectionInfo* GetConnectionInfo() const;
	std::chrono::time_point<std::chrono::steady_clock> LastAccessed();
	std::chrono::time_point<std::chrono::steady_clock> LastAccessed() const;
	void CloseConnection();
	ConnectionState SendMsg(const std::string& msg);
	ConnectionState ReceiveMsg(std::string& msg);

	void SetUsername(const std::string& username);
private:
	void Accessed();
};