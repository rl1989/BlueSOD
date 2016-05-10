#include "ServerManager.h"
#include <stdio.h>
#include <thread>
#include <cstring>
#include <time.h>

extern std::shared_mutex serverMutex;
extern std::shared_mutex serverStateMutex;

using std::string;

ServerManager::~ServerManager()
{
	Cleanup();
}

bool ServerManager::Init()
{
	//WSA isn't initialized so initialize it.
	if (!m_bWSA)
		InitWSA();
	//OpenSSL isn't initialized so initialize it.
	if (!m_bOpenSSL)
		InitOpenSSL();
	
	return m_bWSA;
}

void ServerManager::Cleanup()
{
	//Need to cleanup OpenSSL and WSA.
	CleanupOpenSSL();
	CleanupWSA();
}

bool ServerManager::OpenForConnections(int port)
{
	//WSA must be initialized before creating sockets.
	if (!m_bWSA)
		return false;
	m_listenerSocket = CreateSocket(port);

	return m_listenerSocket != INVALID_SOCKET;
}

SOCKET ServerManager::AcceptIncomingConnection()
{
	//WSA must be initialized and the server must be in a running state to accept connections.
	if (!m_bWSA || GetState() != ServerState::RUNNING)
		return INVALID_SOCKET;

	//struct sockaddr_in addr;
	//int len = sizeof(addr);
	//m_clientSocket = accept(m_listenerSocket, (struct sockaddr*)&addr, &len);
	//Accept a connection and check for errors.
	m_clientSocket = accept(m_listenerSocket, nullptr, nullptr);
	if (m_clientSocket == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error +=  "Could not accept connection. Last WSA error: ";
		error += WSAGetLastError();
		LogError(CONNECTION_ERROR_FILE, error.c_str());
		return INVALID_SOCKET;
	}

	//Generate the SSL object if an SSL context is available.
	if (m_sslContext != nullptr)
	{
		m_clientSSL = SSL_new(m_sslContext);
		if (SSL_set_fd(m_clientSSL, m_clientSocket) == 0)
		{
			string fileLoc = string(SSL_ERROR_FILE_LOCATION);
			fileLoc += SSL_ERROR_FILE;
			m_bClientHasSSLConnection = false;
			LogSSLError(fileLoc);
		}
		else
		{
			m_bClientHasSSLConnection = SSL_accept(m_clientSSL) > 0;
		}
	}
	else
	{
		m_clientSSL = nullptr;
		m_bClientHasSSLConnection = false;
	}

	return m_clientSocket;
}

void ServerManager::StopAcceptingConnections()
{
	//Close the client and listener sockets if they were opened.
	if (m_clientSocket != INVALID_SOCKET)
		closesocket(m_clientSocket);
	if (m_listenerSocket != INVALID_SOCKET)
		closesocket(m_listenerSocket);
	//Signal that the ServerManager is no longer accepting connections.
	SetState(ServerState::NOT_ACCEPTING_CONNECTIONS);
}

void ServerManager::Run(ServerState state)
{
	//Prevents more than one thread from calling Run(). May be unnecessary.
	m_runMutex.lock_shared();
	//Initialize the OpenSSL and WSA libraries first.
	if (!Init())
	{
		//OpenSSL is not available.
		if (!m_bOpenSSL)
		{
			string error = string();
			error += time(nullptr);
			error += " OpenSSL is not initialized.";
			LogError(CONNECTION_ERROR_FILE, error.c_str());
		}
		//WSA is not available. Cannot proceed without WSA.
		//Will probably end up throwing an error here if WSA is not available.
		if (!m_bWSA)
		{
			string error = string();
			error += time(nullptr);
			error += " WSA is not initialized.";
			LogError(CONNECTION_ERROR_FILE, error.c_str());
			return;
		}
	}

	//A Server is already running, so set its state to Running.
	//This may be unnecessary whenever Server is fully functional.
	if (m_server != nullptr)
	{
		m_server->SetServerState(state);
	}

	SetState(state);
	while (GetState() == ServerState::RUNNING)
	{
		//Open and accept connections
		if (OpenForConnections(GetPortNumber()))
		{
			if (AcceptIncomingConnection() == INVALID_SOCKET)
			{
				string errorMessage = string();
				errorMessage += time(nullptr);
				errorMessage += " There was a problem on port ";
				errorMessage += GetPortNumber();
				LogError(CONNECTION_ERROR_FILE, errorMessage.c_str());
			}
			//The incoming connection was accepted so add the client connection and SSL
			//information to the Server (and, additionally, create the Server if necessary).
			else if (m_server == nullptr)
			{
				m_server = std::shared_ptr<Server>(new Server(m_clientSocket, m_clientSSL));
				std::thread serverThread = std::thread(StartServer, m_server);
			}
			else
			{
				m_server->AddClient(m_clientSocket, m_clientSSL);
			}
		}
		//Something happened
		else
		{
			StopAcceptingConnections();
		}
	}
	if (GetState() == ServerState::OFF)
	{
		Cleanup();
	}

	m_runMutex.unlock_shared();
}

void ServerManager::Stop()
{
	SetState(ServerState::OFF);
}

ServerState ServerManager::GetState()
{
	ServerState state;

	//I do not know if it is necessary to initiate a lock in order
	//to read a shared variable.
	m_stateMutex.lock_shared();
	state = m_state;
	m_stateMutex.unlock_shared();
	return state;
}

void ServerManager::SetState(ServerState state)
{
	//Must lock and unlock the mutex to modify the state.
	m_stateMutex.lock_shared();
	m_state = state;
	m_stateMutex.unlock_shared();
}

int ServerManager::GetPortNumber()
{
	int port;
	//Lock and unlock the mutex to modify the port number.
	m_portMutex.lock_shared();
	port = m_portNumber;
	m_portMutex.unlock_shared();
	return port;
}

void ServerManager::SetPortNumber(int port)
{
	m_portMutex.lock_shared();
	m_portNumber = port;
	m_portMutex.unlock_shared();
	if (GetState() == ServerState::RUNNING)
	{
		//TO DO: Send a reset connection along with the new port number to clients
	}
}

SOCKET ServerManager::CreateSocket(int port)
{
	SOCKET socket;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	addr.sin_port = htons(port);
	
	socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socket == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not create the listening socket for ServerManager.";
		LogError(CONNECTION_ERROR_FILE, error.c_str());
		return INVALID_SOCKET;
	}

	if (bind(socket, (struct sockaddr*)&addr, sizeof(addr)) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not bind socket " + static_cast<int>(socket);
		LogError(CONNECTION_ERROR_FILE, error.c_str());
		closesocket(socket);
		return INVALID_SOCKET;
	}

	if (listen(socket, 0) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not listen on socket " + static_cast<int>(socket);
		LogError(CONNECTION_ERROR_FILE, error.c_str());
		closesocket(socket);
		return INVALID_SOCKET;
	}

	return socket;
}

SSL_CTX* ServerManager::CreateSSLContext()
{
	SSL_CTX* ctx;

	ctx = SSL_CTX_new(SSLv23_server_method());
	if (!ctx)
	{
		LogSSLError(SSL_ERROR_FILE);
		return nullptr;
	}

	return ctx;
}

bool ServerManager::ConfigureSSLContext()
{
	string cPath = string(SSL_PEM_LOCATIONS);
	cPath += CERTIFICATE_FILE;
	string pkPath = string(SSL_PEM_LOCATIONS);
	pkPath += PRIVATE_KEY_FILE;

	SSL_CTX_set_ecdh_auto(m_sslContext, 1);
	if (SSL_CTX_use_certificate_file(m_sslContext, cPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		string errLoc = string(SSL_ERROR_FILE_LOCATION);
		errLoc += SSL_ERROR_FILE;
		LogSSLError(errLoc);

		return false;
	}
	SSL_CTX_set_default_passwd_cb(m_sslContext, PasswordCallBack);
	if (SSL_CTX_use_PrivateKey_file(m_sslContext, pkPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		string errLoc = string(SSL_ERROR_FILE_LOCATION);
		errLoc += SSL_ERROR_FILE;
		LogSSLError(errLoc);

		return false;
	}

	return true;
}

bool ServerManager::InitOpenSSL()
{
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_ssl_algorithms();

	m_sslContext = CreateSSLContext();
	if (m_sslContext == nullptr)
	{
		string fileLoc = string(SSL_ERROR_FILE_LOCATION);
		fileLoc += SSL_ERROR_FILE;
		LogSSLError(fileLoc);
		return m_bOpenSSL = false;
	}
	return m_bOpenSSL = ConfigureSSLContext();
}
void ServerManager::CleanupOpenSSL()
{
	if (!m_bOpenSSL)
		return;
	SSL_CTX_free(m_sslContext);
	EVP_cleanup();
}

bool ServerManager::InitWSA()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		string error = string();
		error += time(nullptr);
		error += " WSAStartup failed with error: ";
		error += result;
		LogError(CONNECTION_ERROR_FILE, error);
		CleanupWSA();
		return m_bWSA = false;
	}
	return m_bWSA = true;
}

void ServerManager::CleanupWSA()
{
	WSACleanup();
}

void ServerManager::LogSSLError(const std::string& fileName)
{
	FILE* err = fopen(fileName.c_str(), "w");
	ERR_print_errors_fp(err);
	fclose(err);
}

void ServerManager::LogError(const std::string& fileName, const std::string& errorMessage)
{
	FILE* err = fopen(fileName.c_str(), "w");
	fputs(errorMessage.c_str(), err);
	fclose(err);
}

int ServerManager::PasswordCallBack(char * buffer, int sizeOfBuffer, int rwflag, void * data)
{
	int dirLen;
	char* fileName;
	FILE* file;

	dirLen = strlen(SSL_PASSWORD_FILE_LOCATION) + strlen(PASSWORD_FILE) + 1;
	fileName = new char[dirLen];
	strcat(fileName, SSL_PASSWORD_FILE_LOCATION);
	strcat(fileName, PASSWORD_FILE);
	fileName[dirLen - 1] = '\0';

	file = fopen(fileName, "r");
	fgets(buffer, sizeOfBuffer - 2, file);
	buffer[sizeOfBuffer - 1] = '\0';

	fclose(file);
	delete[] fileName;

	return strlen(buffer);
}

void StartServer(std::shared_ptr<Server> server)
{
	server->Run();
}