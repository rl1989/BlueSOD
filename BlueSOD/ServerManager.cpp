#pragma once
#include "ServerManager.h"
#include "TS_Stack.h"
#include <stdio.h>
#include <time.h>
#include <openssl/crypto.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

using std::string;
using std::fstream;
using std::stack;

void AcceptConnections(ServerManager* sm, SOCKET s)
{
	
}

ServerManager::~ServerManager()
{
	Cleanup();
}

bool ServerManager::Init()
{
	if (!m_bOpenSSL)
	{
		
	}
	

	//WSA isn't initialized so initialize it.
	if (!m_bWSA)
	{
		//WSA is not available. Cannot proceed without WSA.
		if (!InitWSA())
		{
			string error = string();
			error += time(nullptr);
			error += " WSA is not initialized.";
			LogError(CONNECTION_ERROR_LOG, error);
			return false;
		}
	}
	//OpenSSL isn't initialized so initialize it.
	if (!m_bOpenSSL)
	{
		thread_setup();
		if (!InitOpenSSL())
		{
			string error = string();
			error += time(nullptr);
			error += " OpenSSL is not initialized.";
			LogError(CONNECTION_ERROR_LOG, error);
		}
	}
	
	return true;
}

void ServerManager::Cleanup()
{
	//Need to close connections and cleanup OpenSSL as well as WSA.
	CloseConnections();
	thread_cleanup();
	CleanupOpenSSL();
	CleanupWSA();
}

bool ServerManager::Reset()
{
	closesocket(m_listenerSocket);
	if (!OpenForConnections(GetPortNumber()))
	{
		string errorMsg = string();
		errorMsg += time(nullptr);
		errorMsg += " Could not open port ";
		errorMsg += GetPortNumber();
		errorMsg += " for listening on ServerManager.";
		LogError(CONNECTION_ERROR_LOG, errorMsg);

		return false;
	}
	return true;
}

bool ServerManager::Running()
{
	if (!IsListening())
	{
		if (!OpenForConnections(GetPortNumber()))
		{
			string errorMsg = string();
			errorMsg += time(nullptr);
			errorMsg += " Could not open port ";
			errorMsg += GetPortNumber();
			errorMsg += " for listening on ServerManager.";
			LogError(CONNECTION_ERROR_LOG, errorMsg);

			return false;
		}
	}

	Connection info = AcceptIncomingConnection();
	if (info.ssl != nullptr)
	{
		//Pawn info off to another thread for user authentication.
		if (m_userVerifier)
		{
			
		}
		else
		{
			m_userVerifier = shared_ptr<UserVerifier>();
			//m_server = new Server(info.socket, info.ssl);
		}
	}
	return true;
}

bool ServerManager::OpenForConnections(int port)
{
	//WSA must be initialized before creating sockets.
	if (!m_bWSA)
		return false;
	
	if (m_listenerSocket != INVALID_SOCKET)
		closesocket(m_listenerSocket);

	m_listenerSocket = CreateSocket(port);

	return m_listenerSocket != INVALID_SOCKET;
}

bool ServerManager::IsListening()
{
	return m_listenerSocket != INVALID_SOCKET;
}

Connection ServerManager::AcceptIncomingConnection()
{
	//WSA must be initialized and the server must be in a running state to accept connections.
	if (!m_bWSA || GetState() != ServerState::RUNNING)
		return Connection{};

	struct sockaddr_in addr;
	int len = sizeof(addr);
	Connection connection;

	//Generate the connection details.
	connection.socket = accept(m_listenerSocket, (struct sockaddr*)&addr, &len);
	connection.address = addr.sin_addr.S_un.S_addr;
	if (connection.socket == INVALID_SOCKET)
	{
		string fileName = string(CONNECTION_ERROR_LOG);
		string msg = string();
		msg += time(nullptr);
		msg += " Could not accept connection";
		LogConnection(fileName, msg, addr.sin_addr.S_un.S_addr);

		return Connection{};
	}
	
	//Associate the accepted socket with a new ssl object.
	if (m_sslContext != nullptr)
	{
		connection.ssl = SSL_new(m_sslContext);
		SSL_set_fd(connection.ssl, connection.socket);
		if (SSL_accept(connection.ssl) <= 0)
		{
			LogSSLError();

			SSL_free(connection.ssl);
			connection.ssl = nullptr;
		}
	}

	return connection;
}

void ServerManager::StopAcceptingConnections()
{
	//Close the client and listener sockets if they were opened.
	if (m_listenerSocket != INVALID_SOCKET)
		closesocket(m_listenerSocket);
	//Signal that the ServerManager is no longer accepting connections.
	SetState(ServerState::NOT_ACCEPTING_CONNECTIONS);
}

bool ServerManager::Run(ServerState state)
{
	m_runMutex.lock();

	//Initialize the OpenSSL and WSA libraries first.
	if (!Init())
	{
		return false;
	}

	//A Server is already running, so set its state to state.
	//This may be unnecessary whenever Server is fully functional.
	if (m_server != nullptr)
	{
		m_server->SetState(state);
	}

	SetState(state);
	ServerState curState = state;
	while (curState != ServerState::OFF || curState != ServerState::NOT_ACCEPTING_CONNECTIONS)
	{
		switch (curState)
		{
			//A new port was requested to be used by the administrator. Close the current socket and
			//open a new one. Then send a message to any clients telling them to use the new port.
			case ServerState::RESET:
				//m_server->CloseConnections();
				if (!Reset())
					return false;
				//m_server->ReconnectWithClientsOn(GetPortNumber());
				break;
				//Connect with any incoming clients.
			case ServerState::RUNNING:
				

				break;
		}

		/*
			TO DO: Check for any fulfilled requests.
		*/

		curState = GetState();
	}
	if (curState == ServerState::OFF)
	{
		Cleanup();
	}

	m_runMutex.unlock();

	return true;
}

ServerState ServerManager::GetState()
{
	lock_guard<shared_mutex> lck(m_stateMutex);
	return m_state;
}

void ServerManager::SetState(ServerState state)
{
	m_stateMutex.lock();
	m_state = state;
	m_stateMutex.unlock();
}

int ServerManager::GetPortNumber()
{
	lock_guard<shared_mutex> lck(m_portMutex);
	return m_portNumber;
}

void ServerManager::SetPortNumber(int port)
{
	m_portMutex.lock();
	m_portNumber = port;
	m_portMutex.unlock();

	if (GetState() == ServerState::RUNNING)
	{
		//Signal to reset connections.
		SetState(ServerState::RESET);
	}
}

SOCKET ServerManager::CreateSocket(int port)
{
	SOCKET socket;
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	addr.sin_port = htons(port);
	
	//Create socket.
	socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socket == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not create the listening socket for ServerManager.";
		LogError(CONNECTION_ERROR_LOG, error);

		return INVALID_SOCKET;
	}

	//Bind socket.
	if (::bind(socket, (struct sockaddr*)&addr, sizeof(addr)) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not bind socket " + static_cast<int>(socket);
		LogError(CONNECTION_ERROR_LOG, error);

		closesocket(socket);
		return INVALID_SOCKET;
	}

	//Listen on socket.
	if (listen(socket, 0) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not listen on socket " + static_cast<int>(socket);
		LogError(CONNECTION_ERROR_LOG, error);

		closesocket(socket);
		return INVALID_SOCKET;
	}

	return socket;
}

SSL_CTX* ServerManager::CreateSSLContext()
{
	SSL_CTX* ctx;

	//Create the SSL context using the Lv2 and Lv3 methods.
	ctx = SSL_CTX_new(SSLv23_server_method());
	if (!ctx)
	{
		LogSSLError();
		return nullptr;
	}

	return ctx;
}

bool ServerManager::ConfigureSSLContext(SSL_CTX *ctx)
{
	//Path to the certificate.
	string cPath = string(SSL_PEM_LOCATIONS);
	cPath += CERTIFICATE_FILE;
	//Path to the private key.
	string pkPath = string(SSL_PEM_LOCATIONS);
	pkPath += PRIVATE_KEY_FILE;

	//I'm not sure what this does, I just know it has to be done.
	SSL_CTX_set_ecdh_auto(ctx, 1);
	//Tell OpenSSL where to find our certificate.
	if (SSL_CTX_use_certificate_file(ctx, cPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		LogSSLError();

		return false;
	}
	//Tell OpenSSL where to find our password.
	SSL_CTX_set_default_passwd_cb(ctx, PasswordCallBack);
	//Tell OpenSSL where to find our private key.
	if (SSL_CTX_use_PrivateKey_file(ctx, pkPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		LogSSLError();

		return false;
	}

	return true;
}

bool ServerManager::InitOpenSSL()
{
	//Load the OpenSSL library.
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_ssl_algorithms();

	//Create the SSL context.
	if (m_sslContext)
		SSL_CTX_free(m_sslContext);
	m_sslContext = SSL_CTX_new(SSLv23_server_method());
	if (m_sslContext == nullptr)
	{
		LogSSLError();
		return m_bOpenSSL = false;
	}
	//Configure the SSL context.
	return m_bOpenSSL = ConfigureSSLContext(m_sslContext);
}
void ServerManager::CleanupOpenSSL()
{
	//OpenSSL wasn't initialized, so this function does nothing.
	if (!m_bOpenSSL)
		return;
	SSL_CTX_free(m_sslContext);
	EVP_cleanup();
}

bool ServerManager::InitWSA()
{
	WSADATA wsaData;
	//Start up WSA.
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//There was an error with WSA. Might be best to throw an error here?
	if (result != 0)
	{
		string error = string();
		error += time(nullptr);
		error += " WSAStartup failed with error: ";
		error += result;
		LogError(CONNECTION_ERROR_LOG, error);
		CleanupWSA();
		return m_bWSA = false;
	}
	return m_bWSA = true;
}

void ServerManager::CleanupWSA()
{
	if (!m_bWSA)
		WSACleanup();
}

void ServerManager::CloseConnections()
{
	if (m_listenerSocket)
		closesocket(m_listenerSocket);
}

void ServerManager::LogSSLError()
{
	string fName = string(ERROR_LOGS_LOCATION);
	fName += SSL_ERROR_LOG;
	FILE* err = fopen(fName.c_str(), "w");
	//Log the OpenSSL error.
	ERR_print_errors_fp(err);
	fclose(err);
}

void ServerManager::LogError(const std::string& fileName, const std::string& errorMessage)
{
	fstream file = fstream(fileName, std::ios_base::in);
	FILE* err = fopen(fileName.c_str(), "w");
	//Log the error. Might be best to change this to C++ style to avoid any future issues.
	if (file.is_open())
	{
		file << errorMessage.c_str() << std::endl;
		file.close();
	}
}

void ServerManager::LogConnection(const std::string & fileName, const std::string & message, int ip)
{
	fstream file = fstream(fileName, std::ios_base::in);
	string fullMessage = string();
	fullMessage += time(nullptr);
	fullMessage += " " + message + " from IP: ";
	fullMessage += ip;
	
	if (file.is_open())
	{
		file << fullMessage.c_str() << std::endl;
		file.close();
	}
}

void ServerManager::LogLoginAttemp(const std::string & fileName, const std::string & user, bool successful, int ip)
{
	fstream file = fstream(fileName, std::ios_base::in);
	string message = string(successful ? "Successful " : "Unsuccessful ");
	message += " login attemp by " + user + " from IP: ";
	message += ip;

	if (file.is_open())
	{
		file << message.c_str() << std::endl;
		file.close();
	}
}

int ServerManager::PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data)
{
	FILE* file;
	//Load the name of the password file.
	string fName = string(SSL_PASSWORD_FILE_LOCATION);
	fName += PASSWORD_FILE;

	file = fopen(fName.c_str(), "r");
	//Get the password.
	fgets(buffer, sizeOfBuffer - 2, file);
	buffer[sizeOfBuffer - 1] = '\0';
	
	fclose(file);

	return static_cast<int>(strlen(buffer));
}

void StartServer(std::shared_ptr<Server> server)
{
	server->Run();
}