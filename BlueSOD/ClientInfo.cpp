#include "ClientInfo.h"

using std::chrono::steady_clock;
using std::move;
using std::string;

ClientInfo::ClientInfo(const ConnectionInfo& ci, const string& username, int id)
	: m_connectionInfo{ci},
	m_username{username},
	m_id{id}
{}

ClientInfo::ClientInfo(ConnectionInfo&& ci, const string& username, int id)
	:m_connectionInfo{ move(ci) },
	m_username{ username },
	m_id{ id }
{}

ClientInfo::ClientInfo(const ClientInfo& ci)
	: m_connectionInfo{ci.m_connectionInfo},
	m_username{ci.m_username},
	m_id{ci.m_id}
{}

ClientInfo& ClientInfo::operator=(const ClientInfo& ci)
{
	m_connectionInfo = ci.m_connectionInfo;
	m_username = ci.m_username;
	m_id = ci.m_id;

	return *this;
}

ClientInfo::ClientInfo(ClientInfo&& move)
	: m_username{move.GetUsername()},
	m_connectionInfo{std::move(*(move.GetConnectionInfo()))},
	m_lastAccessed{move.LastAccessed()}
{
	move.SetUsername("");
}

ClientInfo& ClientInfo::operator=(ClientInfo && move)
{
	m_username = move.GetUsername();
	m_connectionInfo = std::move(*(move.GetConnectionInfo()));
	m_lastAccessed = move.LastAccessed();

	move.SetUsername("");

	return *this;
}

const std::string& ClientInfo::GetUsername()
{
	return m_username;
}

const std::string & ClientInfo::GetUsername() const
{
	return m_username;
}

int ClientInfo::GetId()
{
	return m_id;
}

int ClientInfo::GetId() const
{
	return m_id;
}

ConnectionInfo* ClientInfo::GetConnectionInfo()
{
	return &m_connectionInfo;
}

const ConnectionInfo* ClientInfo::GetConnectionInfo() const
{
	return &m_connectionInfo;
}

std::chrono::time_point<std::chrono::steady_clock> ClientInfo::LastAccessed()
{
	return m_lastAccessed;
}

std::chrono::time_point<std::chrono::steady_clock> ClientInfo::LastAccessed() const
{
	return m_lastAccessed;
}

void ClientInfo::CloseConnection()
{
	m_connectionInfo.Shutdown();
}

ConnectionState ClientInfo::SendMsg(const std::string& msg)
{
	return m_connectionInfo.Send(msg);
}

ConnectionState ClientInfo::ReceiveMsg(std::string& msg)
{
	ConnectionState state = m_connectionInfo.Receive(msg);

	if (state == ConnectionState::RECEIVED)
		m_lastAccessed = steady_clock::now();

	return state;
}

void ClientInfo::SetUsername(const std::string& username)
{
	m_username = username;
}

void ClientInfo::Accessed()
{
	m_lastAccessed = steady_clock::now();
}
