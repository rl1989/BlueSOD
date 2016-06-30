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
#include "ClientInfo.h"
#include "Messages.h"

class UserVerifier
{
private:
	TS_Deque<ConnectionInfo> m_pendingConnections{};
	TS_Deque<ClientInfo> m_verifiedConnections{};
	TS_Deque<ConnectionInfo> m_rejectedConnections{};
	TS_Deque<ConnectionInfo> m_invalidConnections{};
	ThreadSafe<ServerState> m_state{ServerState::OFF};
	SQLiteDb m_db{};
	std::mutex m_runMutex{};

public:
	UserVerifier(const std::string& userInfoDbLocation, ServerState state = ServerState::OFF)
		: m_state{ state },
		m_db{ userInfoDbLocation }
	{}
	inline void AddPendingConnection(ConnectionInfo&& ci);
	inline bool HasVerifiedConnections();
	inline int NumVerifiedConnections();
	ClientInfo PopVerifiedConnection();
	inline bool HasRejectedConnections();
	inline int NumRejectedConnections();
	ConnectionInfo PopRejectedConnection();
	inline void AddRejectedConnection(ConnectionInfo&& ci);
	inline bool HasInvalidConnections();
	inline int NumInvalidConnections();
	ConnectionInfo PopInvalidConnection();
	inline void AddInvalidConnection(ConnectionInfo&& ci);
	inline void AddVerifiedConnection(ClientInfo&& ci);
	void Run(ServerState state = ServerState::RUNNING);
	inline void SetState(ServerState state);
	inline ServerState GetState();

private:
	ConnectionInfo PopPendingConnection();
	inline bool HasPendingConnections();
	inline int NumOfPendingConnections();
	bool VerifyLoginInformation(const std::string& username, const std::string& password);
};