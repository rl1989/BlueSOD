#pragma once
#include <WinSock2.h>
#include <openssl\ssl.h>
#include <memory>
#include <thread>
#include "TS_Deque.h"
#include "ServerConnections.h"
#include "ServerConcurrency.h"
#include "SQLiteDB.h"

#define SUCCESSFUL_LOGIN "1"
#define UNSUCCESSFUL_LOGIN "2"
#define LOGIN_MSG "3"
#define LOGIN_BEGIN 1
#define DELIMITER ";"

using std::unique_ptr;
using std::mutex;

class UserVerifier
{
private:
	TS_Deque<ConnectionInfo> m_pendingConnections;
	TS_Deque<ConnectionInfo> m_verifiedConnections;
	TS_Deque<ConnectionInfo> m_rejectedConnections;
	ThreadSafe<ServerState> m_state;
	SQLiteDb m_db;
public:
	void AddPendingConnection(const ConnectionInfo& connection);
	void AddPendingConnection(ConnectionInfo&& ci);
	bool CheckForVerifiedConnections();
	int NumVerifiedConnections();
	unique_ptr<ConnectionInfo> PopVerifiedConnection();
	bool CheckRejectedConnections();
	int NumRejectedConnections();
	unique_ptr<ConnectionInfo> PopRejectedConnection();
	void Run(ServerState state);
	void SetState(ServerState state);
	ServerState GetState();

	template<typename... Args>
	static unique_ptr<ConnectionInfo> make_unique(Args&&... args)
	{
		return unique_ptr<ConnectionInfo>{new ConnectionInfo{ args... }};
	}

private:
	unique_ptr<ConnectionInfo> PopPendingConnection();
	void AddVerifiedConnection(ConnectionInfo&& ci);
	bool CheckForPendingConnections();
	int NumOfPendingConnections();
	void AddRejectedConnection(ConnectionInfo&& ci);
	bool RequestingLogin(ConnectionInfo* ci, string* userName, string* password);
	bool VerifyLoginAttempt(ConnectionInfo* ci);
	void RespondSuccessfulLogin(ConnectionInfo* ci);
	void RespondUnsuccesfulLogin(ConnectionInfo* ci);
	bool VerifyLoginInformation(ConnectionInfo* ci);
};