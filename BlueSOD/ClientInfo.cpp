#include "ClientInfo.h"

using std::chrono::steady_clock;
using std::move;

ClientInfo::ClientInfo(NewConnectionInfo&& ci, const std::string& username)
	:m_connectionInfo{move(ci)},
	m_username{username}
{}

const std::string& ClientInfo::GetUsername()
{
	return m_username;
}

NewConnectionInfo* ClientInfo::GetConnectionInfo()
{
	return &m_connectionInfo;
}

std::chrono::time_point<std::chrono::steady_clock> ClientInfo::LastAccessed()
{
	return m_lastAccessed;
}

void ClientInfo::CloseConnection()
{
	m_connectionInfo.Shutdown();
}

connect_s ClientInfo::SendMsg(const std::string& msg)
{
	return m_connectionInfo.Send(msg);
}

connect_s ClientInfo::ReceiveMsg(std::string & msg)
{
	return m_connectionInfo.Receive(msg);
}

void ClientInfo::Accessed()
{
	m_lastAccessed = steady_clock::now();
}
