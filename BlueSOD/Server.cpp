#pragma once
#include "Server.h"

#define COMMON_TABLE_NAME "Messages"
#define TO_COLUMN         "To"
#define FROM_COLUMN       "From"
#define MSG_COLUMN        "Message"
#define DATE_COLUMN       "Date"
#define STATUS_COLUMN     "Status"
#define UNSENT_STATUS     "Unsent"

#define TO_COL_POS     0
#define FROM_COL_POS   1
#define MSG_COL_POS    2
#define DATE_COL_POS   3
#define STATUS_COL_POS 4

inline void Server::SetState(ServerState state)
{
	m_state.ChangeObject(state);
}

inline ServerState Server::GetState()
{
	return m_state.RetrieveObject();
}

int Server::NumberOfClients()
{
	return m_clientMessages.size();
}
