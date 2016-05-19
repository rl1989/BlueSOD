#pragma once
#include <string>
#include <utility>
#include "CommonServer.h"
#include "TS_Stack.h"

using std::string;

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
public:
	UserVerifier(const string& db);
	~UserVerifier();

	void ChangeDb(const string& newDb);
	void AddRequest(const ClientInfo& info);
	ClientInfo GetFulfilledRequest();
	bool HasFulFilledRequest();
private:
	bool OpenDb(const string& db);
	void CloseDb();
};

