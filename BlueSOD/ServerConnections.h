#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include <string>
#include <shared_mutex>
#include <chrono>
#include "LogManager.h"
#include "Flags.h"

//The maximum size of a message a client can send/receive at a time.
#define BUFFER_SIZE 1024

//Defines the state of the server. Names are self explanatory.
enum ServerState
{
	OFF, RUNNING, NOT_ACCEPTING_CONNECTIONS, RESET, START_UP
};

enum ConnectionState
{
	OK, ERR, NOT_FULLY_SENT, NOT_INITIATED, NOT_SENT, SHUTDOWN, SENT, RECEIVED,
	WANT_READ, WANT_WRITE, NO_DATA_PRESENT
};

//#else

#define SOCKET_OK 0

class ConnectionInfo
{
private:
	SOCKET m_socket{ INVALID_SOCKET };
	SSL* m_ssl{ nullptr };
	int m_sslStatus{SSL_ERROR_NONE};
	int m_socketStatus{SOCKET_OK};
	int m_bytesSent{ 0 };

public:
	ConnectionInfo() = default;
	explicit ConnectionInfo(SOCKET socket);
	ConnectionInfo(SOCKET socket, SSL* ssl);
	ConnectionInfo(ConnectionInfo&& move);
	ConnectionInfo& operator=(ConnectionInfo&& move);
	~ConnectionInfo();
	
	ConnectionState Send(const std::string& msg);
	ConnectionState Receive(std::string& buffer, bool file = false, int expectedLength = 0);
	void Shutdown(int how = SD_BOTH);
	inline bool IsValid();
	ConnectionState Accept(SOCKET listener, SSL_CTX* ssl_ctx);

	SOCKET GetSocket();
	SSL* GetSSL();
	int GetSocketStatus();
	int GetSSLStatus();
	int BytesSent();

	void SetSocket(SOCKET socket);
	void SetSSL(SSL* ssl);

private:
	ConnectionState SendSSL(const std::string& msg);
	ConnectionState SendSocket(const std::string& msg);
	ConnectionState ReceiveSSL(std::string& buffer, bool file = false, int expectedLength = 0);
	ConnectionState ReceiveSocket(std::string& buffer, bool file = false, int expectedLength = 0);
	int SelectForRead();
	int SelectForWrite();

	int GetSocketError();
	int GetSocketError(SOCKET socket);
};