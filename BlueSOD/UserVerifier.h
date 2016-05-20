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
	TS_Stack<ClientInfo> queue;
	TS_Stack<ClientInfo> finished;
	SQLiteDb *db;
	mutex dbMutex;
public:
	UserVerifier(UserVerifier&& uv)
		:db{uv.db},
		dbMutex{}
	{
		for (int i = 0; i < uv.queue.size(); i++)
		{
			queue.push(uv.queue.top());
			uv.queue.pop();
		}
		for (int i = 0; i < uv.finished.size(); i++)
		{
			finished.push(uv.finished.top());
			uv.finished.pop();
		}
		uv.db = nullptr;
	}
	UserVerifier(const string& dbName)
		:db{ new SQLiteDb(dbName) },
		queue{},
		finished{},
		dbMutex{}
	{}
	UserVerifier()
		: db{ nullptr },
		queue{},
		finished{},
		dbMutex{}
	{}
	~UserVerifier()
	{
		if (db)
			delete db;
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
	bool HasFulfilledRequest();
};

