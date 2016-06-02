#pragma once
#include "UserVerifier.h"

using std::move;
using std::string;

void UserVerifier::AddPendingConnection(ConnectionInfo&& ci)
{
	m_pendingConnections.PushBack(move(ci));
}

bool UserVerifier::HasVerifiedConnections()
{
	return m_verifiedConnections.Empty();
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

ConnectionInfo UserVerifier::PopRejectedConnection()
{
	ConnectionInfo ci{};

	ci = move(m_rejectedConnections.Front());
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
			while (NumOfPendingConnections() > 0)
			{
				ConnectionInfo ci = move(PopPendingConnection());

				ReadFromSSL(&ci);
				if (VerifyLoginAttempt(&ci))
				{
					if (VerifyLoginInformation(&ci))
					{
						AddVerifiedConnection(move(ci));
					}
					else
					{
						AddRejectedConnection(move(ci));
					}
				}
				else
				{
					AddInvalidRequest(move(ci));
				}
			}
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

ConnectionInfo UserVerifier::PopVerifiedConnection()
{
	ConnectionInfo ci{};

	ci = move(m_verifiedConnections.Front());
	m_verifiedConnections.PopFront();

	return ci;
}

ConnectionInfo UserVerifier::PopPendingConnection()
{
	ConnectionInfo ci{};

	if (m_pendingConnections.Empty())
		return move(ci);

	ci = move(m_pendingConnections.Front());
	m_pendingConnections.PopFront();

	return ci;
}

void UserVerifier::AddVerifiedConnection(ConnectionInfo&& ci)
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

void UserVerifier::AddRejectedConnection(ConnectionInfo&& ci)
{
	m_rejectedConnections.PushBack(ci);
}

void UserVerifier::AddInvalidRequest(ConnectionInfo&& ci)
{
	m_invalidRequests.PushBack(ci);
}

bool UserVerifier::VerifyLoginAttempt(ConnectionInfo* ci)
{
	string message{ ci->buffer.buffer.buf };
	
	if (message.substr(0, 1) == LOGIN_MSG)
	{
		return true;
	}

	return false;
}

bool UserVerifier::VerifyLoginInformation(ConnectionInfo * ci)
{
	string msg{ci->buffer.buffer.buf};
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
