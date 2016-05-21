#pragma once
#include "UserVerifier.h"

void UserVerifier::OpenDb(const string & newDb)
{
	m_dbMutex.lock();
	if (m_db)
		delete m_db;
	m_db = new SQLiteDb(newDb);
	m_dbMutex.unlock();
}

void UserVerifier::AddRequest(const ClientInfo & info)
{
	m_queue.push(info);
}

ClientInfo UserVerifier::GetFulfilledRequest()
{
	if (m_finished.size() == 0)
		return ClientInfo{};
	ClientInfo info = std::move(m_finished.Top());
	m_finished.pop();
	return std::move(info);
}

bool UserVerifier::HasFulfilledRequest()
{
	return !m_finished.Empty();
}
