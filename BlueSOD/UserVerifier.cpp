#pragma once
#include "UserVerifier.h"

void UserVerifier::AddPendingConnection(const Connection& c)
{
	ConnectionInfo ci{ c };
	m_pending.PushBack(ci);
}

ConnectionInfo& UserVerifier::GetVerifiedConnection()
{
	ConnectionInfo info = std::move(m_verified.Front());
	m_verified.PopFront();
	return std::move(info);
}

void UserVerifier::Run(string dbName)
{
	/*Prevent the UserVerifier from running more than once.*/
	if (IsRunning())
		return;
	else
	{
		lock_guard<mutex> lck(m_runningMutex);
		m_running = true;
	}

	/* Create a connection to a database dbName. */
	{
		lock_guard<mutex> lck(m_dbMutex);
		m_db = std::make_unique<SQLiteDb>(SQLiteDb(dbName));
	}

	while (IsRunning())
	{
		if (!m_pending.Empty())
		{
			ConnectionInfo ci = GetPendingConnection();
			/*
				TO DO:
				  1) Read from the connection.
				  2) Validate the information sent.
					a) Make sure it's a valid login message.
					b) Validate the user information
			*/
		}
	}
}

void UserVerifier::Stop()
{
	lock_guard<mutex> lck(m_runningMutex);
	m_running = false;
}

bool UserVerifier::IsRunning()
{
	lock_guard<mutex> lck(m_runningMutex);
	return m_running;
}

ConnectionInfo& UserVerifier::GetPendingConnection()
{
	ConnectionInfo ci = m_pending.Front();
	m_pending.PopFront();
	return std::move(ci);
}

void UserVerifier::AddVerifiedConnection(const Connection& c)
{
	m_verified.PushBack(c);
}
