#pragma once
#include <string>
#include <utility>
#include <mutex>
#include <memory>
#include "CommonServer.h"
#include "TS_Deque.h"
#include "SQLiteDB.h"

using std::string;
using std::mutex;
using std::unique_ptr;

class UserVerifier
{
private:
	unique_ptr<SQLiteDb> m_db;
	TS_Deque<ConnectionInfo> m_verified;
	TS_Deque<ConnectionInfo> m_pending;
	bool m_running;
	mutex m_runningMutex;
	mutex m_dbMutex;
public:
	
	void AddPendingConnection(const Connection& ci);
	ConnectionInfo& GetVerifiedConnection();

	void Run(string dbName);
	void Run();
	void Stop();
	bool IsRunning();
private:
	ConnectionInfo& GetPendingConnection();
	void AddVerifiedConnection(const Connection& c);
};