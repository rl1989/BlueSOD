#pragma once
#include <string>
#include <utility>
#include <mutex>
#include "CommonServer.h"
#include "TS_Stack.h"
#include "SQLiteDB.h"

using std::string;
using std::mutex;

/*
	This class opens the User Information database and verifies that the information passed to it is
	valid. To allow for cross thread communication and for a backlog of requests, a TS_Stack
	object is used to hold the unverified and verified ClientInfo objects. A new request is placed
	into queue and a completed request is placed into finished.
	The requests are completed in a FIFO method. This methodology is used just so the ServerManager
	does not have to wait for requests to be fulfilled. It can "come by later" and check to see if the
	request has been fulfilled.
*/
class UserVerifier
{
private:
	TS_Stack<ClientInfo> m_queue;
	TS_Stack<ClientInfo> m_finished;
	SQLiteDb *m_db;
	mutex m_dbMutex;
	bool m_isRunning;
	mutex m_runningMutex;
public:
	UserVerifier(const string& dbName)
		:m_db{ new SQLiteDb(dbName) },
		m_queue{},
		m_finished{},
		m_dbMutex{},
		m_isRunning{false},
		m_runningMutex{}
	{}
	~UserVerifier()
	{
		if (m_db)
			delete m_db;
	}

	/*
		Opens a new database with the name newDb. If one is already open, it is closed and then the new one is
		opened.
	*/
	void OpenDb(const string& newDb);
	/*
		Adds info to the queue for verification.
	*/
	void AddRequest(const ClientInfo& info);
	/*
		Returns the last request that was fulfilled and removes it from the stack. If a request has not been fulfilled
		(i.e. finished.size() == 0) then this function will return a ClientInfo object with default values. To be
		completely efficient, HasFulfilledRequest() should be called first.
	*/
	ClientInfo GetFulfilledRequest();
	/*
		Checks to see if GetFulfilledRequest() will return a valid ClientInfo object.
	*/
	inline bool HasFulfilledRequest();
	inline bool IsRunning()
	{
		lock_guard<mutex> lck(m_runningMutex);

		return m_isRunning;
	}
	inline bool SetRunningState(bool state)
	{
		lock_guard<mutex> lck(m_runningMutex);

		m_isRunning = state;
	}
};

