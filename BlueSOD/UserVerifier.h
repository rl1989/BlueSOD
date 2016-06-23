#pragma once
#include <memory>
#include <thread>
#include <list>
#include <cstring>
#include <vector>
#include "TS_Deque.h"
#include "ServerConnections.h"
#include "SQLiteDB.h"
#include "ServerConcurrency.h"
#include "TS_Deque.cpp"
#include "ServerConcurrency.cpp"

#define SUCCESSFUL_LOGIN "1"
#define UNSUCCESSFUL_LOGIN "2"
#define LOGIN_MSG "3"
#define LOGIN_BEGIN 1
#define DELIMITER ";"

class UserVerifier
{
private:
	TS_Deque<NewConnectionInfo> m_pendingConnections{};
	TS_Deque<NewConnectionInfo> m_verifiedConnections{};
	TS_Deque<NewConnectionInfo> m_rejectedConnections{};
	TS_Deque<NewConnectionInfo> m_invalidConnections{};
	ThreadSafe<ServerState> m_state{};
	SQLiteDb m_db{};

public:
	UserVerifier(const std::string& userInfoDbLocation, ServerState state = ServerState::OFF)
		: m_state{ state },
		m_db{ userInfoDbLocation }
	{}
	void AddPendingConnection(NewConnectionInfo&& ci);
	bool HasVerifiedConnections();
	int NumVerifiedConnections();
	NewConnectionInfo PopVerifiedConnection();
	bool HasRejectedConnections();
	int NumRejectedConnections();
	NewConnectionInfo PopRejectedConnection();
	void Run(ServerState state = ServerState::RUNNING);
	void SetState(ServerState state);
	ServerState GetState();

private:
	NewConnectionInfo PopPendingConnection();
	void AddVerifiedConnection(NewConnectionInfo&& ci);
	bool HasPendingConnections();
	int NumOfPendingConnections();
	void AddRejectedConnection(NewConnectionInfo&& ci);
	void AddInvalidConnection(NewConnectionInfo&& ci);
	bool VerifyLoginAttempt(const std::string& msg);
	bool VerifyLoginInformation(const std::string& msg);
};