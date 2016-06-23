#pragma once
#include "UserVerifier.h"

using std::move;
using std::string;
using std::list;
using std::vector;

void UserVerifier::AddPendingConnection(NewConnectionInfo&& ci)
{
	m_pendingConnections.PushBack(ci);
}

bool UserVerifier::HasVerifiedConnections()
{
	return !m_verifiedConnections.Empty();
}

int UserVerifier::NumVerifiedConnections()
{
	return m_verifiedConnections.Size();
}

bool UserVerifier::HasRejectedConnections()
{
	return !m_rejectedConnections.Empty();
}

int UserVerifier::NumRejectedConnections()
{
	return m_rejectedConnections.Size();
}

NewConnectionInfo UserVerifier::PopRejectedConnection()
{
	NewConnectionInfo ci = move(m_rejectedConnections.Front());

	m_rejectedConnections.PopFront();

	return ci;
}

void UserVerifier::Run(ServerState state)
{
	SetState(state);

	while (GetState() == ServerState::RUNNING)
	{
		if (HasPendingConnections())
		{
			NewConnectionInfo ci = move(PopPendingConnection());
			if (ci.IsValid())
			{
				string msg;
				switch (ci.Receive(msg))
				{
					case connect_s::NO_DATA_PRESENT:
					case connect_s::WANT_READ:
						AddPendingConnection(move(ci));
						break;
					case connect_s::ERR:
						/*Log error and let ci go out of scope to shutdown the connection.*/
						break;
					case connect_s::RECEIVED:
						if (VerifyLoginAttempt(msg))
						{
							if (VerifyLoginInformation(msg))
							{
								//AddVerifiedConnection(move(ci));
							}
							else
							{
								//AddRejectedConnection(move(ci));
							}

						}
						else
						{
							AddInvalidConnection(move(ci));
						}
						break;
				}
			}
			/*DO NOT USE ci HERE!!!*/
		}
	}
}

void UserVerifier::SetState(ServerState state)
{
	m_state.ChangeObject(state);
}

ServerState UserVerifier::GetState()
{
	return m_state.RetrieveObject();
}

NewConnectionInfo UserVerifier::PopVerifiedConnection()
{
	NewConnectionInfo ci = move(m_verifiedConnections.Front());

	m_verifiedConnections.PopFront();

	return ci;
}

NewConnectionInfo UserVerifier::PopPendingConnection()
{
	NewConnectionInfo ci = move(m_pendingConnections.Front());

	m_pendingConnections.PopFront();

	return ci;
}

void UserVerifier::AddVerifiedConnection(NewConnectionInfo&& ci)
{
	m_verifiedConnections.PushBack(ci);
}

bool UserVerifier::HasPendingConnections()
{
	return !m_pendingConnections.Empty();
}

int UserVerifier::NumOfPendingConnections()
{
	return m_pendingConnections.Size();
}

void UserVerifier::AddRejectedConnection(NewConnectionInfo&& ci)
{
	m_rejectedConnections.PushBack(ci);
}
void UserVerifier::AddInvalidConnection(NewConnectionInfo && ci)
{
	m_invalidConnections.PushBack(ci);
}

bool UserVerifier::VerifyLoginAttempt(const string& msg)
{
	return msg.compare(LOGIN_MSG) == 0;
}

bool UserVerifier::VerifyLoginInformation(const string& msg)
{
	int delimiter = msg.find_first_of(DELIMITER, 1);
	string sqlStatement{ "SELECT " };
	sqlStatement += USERNAME_COL;
	sqlStatement += ",";
	sqlStatement += PASSWORD_COL;
	sqlStatement += " FROM ";
	sqlStatement += USER_INFO_DB;
	sqlStatement += " WHERE ";
	sqlStatement += USERNAME_COL;
	sqlStatement += "=" + SQLiteDb::CleanStatement(msg.substr(1, delimiter));
	sqlStatement += " AND ";
	sqlStatement += PASSWORD_COL;
	sqlStatement += "=" + SQLiteDb::CleanStatement(msg.substr(delimiter, msg.size()));
	sqlStatement += ";";

	return m_db.ExecuteStatement(sqlStatement) == SQLITE_ROW;
}
