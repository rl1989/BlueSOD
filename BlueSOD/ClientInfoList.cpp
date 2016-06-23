#include "ClientInfoList.h"

using std::lock_guard;
using std::mutex;
using std::move;

bool ClientInfoList::Add(NewConnectionInfo&& ci, const std::string & username)
{
	lock_guard<mutex> lck(m_mutex);

	for (auto it = m_list.begin(); it != m_list.end(); it++)
	{
		if (it->GetUsername() == username)
			return false;
	}

	m_list.push_back(ClientInfo{ move(ci), username });

	return true;
}

bool ClientInfoList::Add(ClientInfo&& ci)
{
	string username = ci.GetUsername();
	lock_guard<mutex> lck(m_mutex);

	for (auto it = m_list.begin(); it != m_list.end(); it++)
	{
		if (it->GetUsername() == username)
			return false;
	}

	m_list.push_back(move(ci));

	return true;
}

void ClientInfoList::RemoveAll()
{
	m_list.clear();
}
