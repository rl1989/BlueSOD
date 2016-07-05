#pragma once
#include "UserVerifier.h"

using std::move;
using std::string;
using std::list;
using std::vector;

void UserVerifier::AddPendingConnection(ConnectionInfo&& ci)
{
	m_pendingConnections.PushBack(ci);
}

inline void UserVerifier::AddPendingConnection(const ConnectionInfo& ci)
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

ConnectionInfo UserVerifier::PopRejectedConnection()
{
	ConnectionInfo ci = move(m_rejectedConnections.Front());

	m_rejectedConnections.PopFront();

	return ci;
}

void UserVerifier::Run(ServerState state)
{
	if (!m_runMutex.try_lock())
		return;
	SetState(state);

	while (state != ServerState::OFF)
	{
		switch (state)
		{
			case ServerState::RUNNING:
				if (HasPendingConnections())
				{
					ConnectionInfo ci = move(PopPendingConnection());
					if (ci.IsValid())
					{
						string msg;
						switch (ci.Receive(msg))
						{
							case ConnectionState::NO_DATA_PRESENT:
							case ConnectionState::WANT_READ:
								AddPendingConnection(move(ci));
								break;
							case ConnectionState::ERR:
								/*Log error and shut down the connection.*/
								ci.Shutdown();
								break;
							case ConnectionState::RECEIVED:
								LoginMessage message = LoginMessage::ParseLoginMsg(msg);

								if (message.IsValid())
								{
									int id;
									if (VerifyLoginInformation(message.Username(), message.Password(), &id))
									{
										ClientInfo client{ move(ci), message.Username(), id };
										AddVerifiedConnectionToBack(move(client));
									}
									else
									{
										AddRejectedConnection(move(ci));
									}
								}
								else
								{
									AddInvalidConnection(move(ci));
								}
								
								break;
						}
					}
				}
				break;
			case ServerState::RESET:
				/*Remove all connections and clients.*/
				while (HasPendingConnections())
				{
					PopPendingConnection().Shutdown();
				}
				while (HasVerifiedConnections())
				{
					PopVerifiedConnection().CloseConnection();
				}
				while (HasRejectedConnections())
				{
					PopRejectedConnection().Shutdown();
				}
				while (HasInvalidConnections())
				{
					PopInvalidConnection().Shutdown();
				}

				SetState(ServerState::RUNNING);
				break;
		}

		state = GetState();
	}

	m_runMutex.unlock();
}

void UserVerifier::SetState(ServerState state)
{
	m_state.ChangeObject(state);
}

ServerState UserVerifier::GetState()
{
	return m_state.RetrieveObject();
}

ClientInfo UserVerifier::PopVerifiedConnection()
{
	ClientInfo ci = move(m_verifiedConnections.Front());

	m_verifiedConnections.PopFront();

	return ci;
}

ConnectionInfo UserVerifier::PopPendingConnection()
{
	ConnectionInfo ci = move(m_pendingConnections.Front());

	m_pendingConnections.PopFront();

	return ci;
}

void UserVerifier::AddVerifiedConnectionToBack(ClientInfo&& ci)
{
	m_verifiedConnections.PushBack(ci);
}

inline void UserVerifier::AddVerifiedConnectionToBack(const ClientInfo & ci)
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
inline void UserVerifier::AddRejectedConnection(const ConnectionInfo & ci)
{
	m_rejectedConnections.PushBack(ci);
}
inline bool UserVerifier::HasInvalidConnections()
{
	return m_invalidConnections.Empty();
}
inline int UserVerifier::NumInvalidConnections()
{
	return m_invalidConnections.Size();
}
ConnectionInfo UserVerifier::PopInvalidConnection()
{
	ConnectionInfo ci = move(m_invalidConnections.Front());

	m_invalidConnections.PopFront();

	return ci;
}
void UserVerifier::AddInvalidConnection(ConnectionInfo && ci)
{
	m_invalidConnections.PushBack(ci);
}

inline void UserVerifier::AddInvalidConnectionToBack(const ConnectionInfo & ci)
{
	m_invalidConnections.PushBack(ci);
}

bool UserVerifier::VerifyLoginInformation(const string& username, const string& password, int* id)
{
	string sqlStatement{ "SELECT " };
	sqlStatement += ID_COL;
	sqlStatement += ",";
	sqlStatement += USERNAME_COL;
	sqlStatement += ",";
	sqlStatement += PASSWORD_COL;
	sqlStatement += " FROM ";
	sqlStatement += USER_INFO_DB;
	sqlStatement += " WHERE ";
	sqlStatement += USERNAME_COL;
	sqlStatement += "=" + username;
	sqlStatement += " AND ";
	sqlStatement += PASSWORD_COL;
	sqlStatement += "=" + password;
	sqlStatement += ";";

	if (m_db.ExecuteStatement(sqlStatement))
	{
		*id = m_db.GetColumnInt(0);
		return true;
	}
	else
	{
		return false;
	}
}