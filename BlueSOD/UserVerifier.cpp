#include "UserVerifier.h"

void UserVerifier::OpenDb(const string & newDb)
{
	dbMutex.lock();
	if (db)
		delete db;
	db = new SQLiteDb(newDb);
	dbMutex.unlock();
}

void UserVerifier::AddRequest(const ClientInfo & info)
{
	queue.push(info);
}

ClientInfo UserVerifier::GetFulfilledRequest()
{
	if (finished.size() == 0)
		return ClientInfo{};
	ClientInfo info = finished.top();
	finished.pop();
	return std::move(info);
}

bool UserVerifier::HasFulfilledRequest()
{
	return !finished.empty();
}
