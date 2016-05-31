#pragma once
#include "UserVerifier.h"

void UserVerifier::AddPendingConnection(const ConnectionInfo& ci)
{
	m_pendingConnections.PushBack(ci);
}

void UserVerifier::AddPendingConnection(ConnectionInfo&& ci)
{
	m_pendingConnections.PushBack(ci);
}

bool UserVerifier::CheckForVerifiedConnections()
{
	return m_verifiedConnections.Empty();
}

int UserVerifier::NumVerifiedConnections()
{
	return m_verifiedConnections.Size();
}

bool UserVerifier::CheckRejectedConnections()
{
	return !m_rejectedConnections.Empty();
}

int UserVerifier::NumRejectedConnections()
{
	return m_rejectedConnections.Size();
}

unique_ptr<ConnectionInfo> UserVerifier::PopRejectedConnection()
{
	unique_ptr<ConnectionInfo> ci{ nullptr };

	if (m_rejectedConnections.Empty())
		return ci;

	ci = make_unique(m_rejectedConnections.Front());
	m_rejectedConnections.PopFront();

	return ci;
}

void UserVerifier::Run(ServerState state)
{
	SetState(state);

	while (GetState() == ServerState::RUNNING)
	{
		if (CheckForPendingConnections())
		{
			while (NumOfPendingConnections() > 0)
			{
				unique_ptr<ConnectionInfo> pending = PopPendingConnection();

				ReadFromSSL(pending.get());

				if (VerifyLoginAttempt(pending.get()))
				{
					if (VerifyLoginInformation(pending.get()))
					{
						AddVerifiedConnection(move(*pending));
					}
					else
					{
						AddRejectedConnection(move(*pending));
					}
				}
				else
				{
					AddInvalidRequest(move(*pending));
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

unique_ptr<ConnectionInfo> UserVerifier::PopVerifiedConnection()
{
	unique_ptr<ConnectionInfo> ci{ nullptr };
	
	if (m_verifiedConnections.Empty())
		return ci;

	ci = make_unique(m_verifiedConnections.Front());
	m_verifiedConnections.PopFront();

	return ci;
}

unique_ptr<ConnectionInfo> UserVerifier::PopPendingConnection()
{
	unique_ptr<ConnectionInfo> ci{ nullptr };

	if (m_pendingConnections.Empty())
		return ci;

	ci = make_unique(m_pendingConnections.Front());
	m_pendingConnections.PopFront();

	return ci;
}

void UserVerifier::AddVerifiedConnection(ConnectionInfo && ci)
{
	m_verifiedConnections.PushBack(ci);
}

bool UserVerifier::CheckForPendingConnections()
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

bool UserVerifier::RequestingLogin(ConnectionInfo* ci, string* userName, string* password)
{
	return false;
}

bool UserVerifier::VerifyLoginAttempt(ConnectionInfo* ci)
{
	string message{ ci->buffer->buffer.buf };
	
	if (message.substr(0, 1) == LOGIN_MSG)
	{
		return true;
	}

	return false;
}

void UserVerifier::RespondSuccessfulLogin(ConnectionInfo* ci)
{
	strcpy(ci->buffer->buffer.buf, SUCCESSFUL_LOGIN);
	SendToSSL(ci);
}

void UserVerifier::RespondUnsuccesfulLogin(ConnectionInfo* ci)
{
	strcpy(ci->buffer->buffer.buf, UNSUCCESSFUL_LOGIN);
	SendToSSL(ci);
}

bool UserVerifier::VerifyLoginInformation(ConnectionInfo * ci)
{
	string msg{ci->buffer->buffer.buf};
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
