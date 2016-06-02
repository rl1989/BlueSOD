#pragma once
#include <memory>
#include <thread>
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
	TS_Deque<ConnectionInfo> m_pendingConnections;
	TS_Deque<ConnectionInfo> m_verifiedConnections;
	TS_Deque<ConnectionInfo> m_rejectedConnections;
	TS_Deque<ConnectionInfo> m_invalidRequests;
	ThreadSafe<ServerState> m_state;
	SQLiteDb m_db;
public:
	UserVerifier(const std::string& userInfoDbLocation, ServerState state = ServerState::OFF)
		: m_state{ state },
		m_db{ userInfoDbLocation }
	{}
	void AddPendingConnection(ConnectionInfo&& ci);
	bool HasVerifiedConnections();
	int NumVerifiedConnections();
	ConnectionInfo PopVerifiedConnection();
	bool HasRejectedConnections();
	int NumRejectedConnections();
	ConnectionInfo PopRejectedConnection();
	void Run(ServerState state = ServerState::RUNNING);
	void SetState(ServerState state);
	ServerState GetState();

private:
	ConnectionInfo PopPendingConnection();
	void AddVerifiedConnection(ConnectionInfo&& ci);
	bool HasPendingConnections();
	int NumOfPendingConnections();
	void AddRejectedConnection(ConnectionInfo&& ci);
	void AddInvalidRequest(ConnectionInfo&& ci);
	bool VerifyLoginAttempt(ConnectionInfo* ci);
	bool VerifyLoginInformation(ConnectionInfo* ci);
};