#include "ServerManager.h"
#include <stdio.h>
#include <thread>
#include <cstring>
#include <time.h>

using std::string;
using std::fstream;
using std::list;

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

bool ServerManager::IsListening()
{
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

bool ServerManager::Run(ServerState state)
{
	LockRun();
	//Initialize the OpenSSL and WSA libraries first.
	if (!Init())
	{
		//OpenSSL is not available.
		if (!m_bOpenSSL)
		{
			string error = string();
			error += time(nullptr);
			error += " OpenSSL is not initialized.";
			string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
			fileName += CONNECTION_ERROR_FILE;
			LogError(fileName, error);
		}
		//WSA is not available. Cannot proceed without WSA.
		//Will probably end up throwing an error here if WSA is not available.
		if (!m_bWSA)
		{
			string error = string();
			error += time(nullptr);
			error += " WSA is not initialized.";
			string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
			fileName += CONNECTION_ERROR_FILE;
			LogError(fileName, error);
			return false;
		}
	}

	//A Server is already running, so set its state to Running.
	//This may be unnecessary whenever Server is fully functional.
	if (m_server != nullptr)
	{
		m_server->SetServerState(state);
	}

	SetState(state);
	ServerState curState = GetState();
	while (curState != ServerState::OFF || curState != ServerState::NOT_ACCEPTING_CONNECTIONS)
	{
		switch (curState)
		{
			//A new port was requested to be used by the administrator. Close the current socket and
			//open a new one. Then send a message to any clients attached to use the new port.
			case ServerState::RESET:
				closesocket(m_listenerSocket);
				if (!OpenForConnections(GetPortNumber()))
				{
					string errorMsg = string();
					errorMsg += time(nullptr);
					errorMsg += " Could not open port ";
					errorMsg += GetPortNumber();
					errorMsg += " for listening on ServerManager.";
					string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
					fileName += CONNECTION_ERROR_FILE;
					LogError(fileName, errorMsg);

					return false;
				}
				m_server->ReconnectWithClientsOn(GetPortNumber());
				break;
			//Connect with any incoming clients.
			case ServerState::RUNNING:
				if (!IsListening())
				{
					if (!OpenForConnections(GetPortNumber()))
					{
						string errorMsg = string();
						errorMsg += time(nullptr);
						errorMsg += " Could not open port ";
						errorMsg += GetPortNumber();
						errorMsg += " for listening on ServerManager.";
						string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
						fileName += CONNECTION_ERROR_FILE;
						LogError(fileName, errorMsg);

						return false;
					}
				}
				/*
					TO DO:
					  1) Need to change this so that ServerManager is not blocking here.
					  2) Testing... again
				*/
				if (AcceptIncomingConnection() == INVALID_SOCKET)
				{
					string errorMessage = string();
					errorMessage += time(nullptr);
					errorMessage += " There was a problem on port ";
					errorMessage += GetPortNumber();
					string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
					fileName += CONNECTION_ERROR_FILE;
					LogError(fileName, errorMessage);
				}
				else
				{
					/*
						TO DO:
						  1) Pawn the connection off to another thread that will dictate the validity of the user.
						  2) If it is a valid user,
						    a) Respond to the user with an acceptance message.
							b) Pass the connection off to the Server.
						  3) If not, send the user a message indicating invalid credentials were sent.
					*/
				}
				//The incoming connection was accepted so add the client connection and SSL
				//information to the Server (and, additionally, create the Server if necessary).
				/*
					TO DO:
					  1) Verify the client's login information first.
				*/
				if (m_server == nullptr)
				{
					for (int i = 0; i < requests.size(); i++)
					{
						if (requests.at(i).get().auth == UserAuthentication::PENDING)
							continue;
					}
					m_server = std::shared_ptr<Server>(new Server(m_clientSocket, m_clientSSL));
					std::thread serverThread = std::thread(StartServer, m_server);
				}
				else
				{
					m_server->AddClient(m_clientSocket, m_clientSSL);
				}
				break;
		}

		curState = GetState();
	}
	if (GetState() == ServerState::OFF)
	{
		Cleanup();
	}

	UnlockRun();
}

ServerState ServerManager::GetState()
{
	ServerState state;

	//I do not know if it is necessary to initiate a lock in order
	//to read a shared variable.
	LockState();
	state = m_state;
	UnlockState();

	return state;
}

void ServerManager::SetState(ServerState state)
{
	//Must lock and unlock the mutex to modify the state.
	LockState();
	m_state = state;
	UnlockState();
}

int ServerManager::GetPortNumber()
{
	int port;
	
	LockPort();
	port = m_portNumber;
	UnlockPort();

	return port;
}

void ServerManager::SetPortNumber(int port)
{
	LockPort();
	m_portNumber = port;
	UnlockPort();

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
		string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
		fileName += CONNECTION_ERROR_FILE;
		LogError(fileName, error);
		return INVALID_SOCKET;
	}

	//Bind socket.
	if (bind(socket, (struct sockaddr*)&addr, sizeof(addr)) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not bind socket " + static_cast<int>(socket);
		string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
		fileName += CONNECTION_ERROR_FILE;
		LogError(fileName, error);
		closesocket(socket);
		return INVALID_SOCKET;
	}

	//Listen on socket.
	if (listen(socket, 0) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not listen on socket " + static_cast<int>(socket);
		string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
		fileName += CONNECTION_ERROR_FILE;
		LogError(fileName, error);
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
		LogSSLError(SSL_ERROR_FILE);
		return nullptr;
	}

	return ctx;
}

bool ServerManager::ConfigureSSLContext()
{
	//Path to the certificate.
	string cPath = string(SSL_PEM_LOCATIONS);
	cPath += CERTIFICATE_FILE;
	//Path to the private key.
	string pkPath = string(SSL_PEM_LOCATIONS);
	pkPath += PRIVATE_KEY_FILE;

	//I'm not sure what this does, I just know it has to be done.
	SSL_CTX_set_ecdh_auto(m_sslContext, 1);
	//Tell OpenSSL where to find our certificate.
	if (SSL_CTX_use_certificate_file(m_sslContext, cPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		string errLoc = string(SSL_ERROR_FILE_LOCATION);
		errLoc += SSL_ERROR_FILE;
		LogSSLError(errLoc);

		return false;
	}
	//Tell OpenSSL where to find our password.
	SSL_CTX_set_default_passwd_cb(m_sslContext, PasswordCallBack);
	//Tell OpenSSL where to find our private key.
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
	//Load the OpenSSL library.
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_ssl_algorithms();

	//Create the SSL context.
	m_sslContext = CreateSSLContext();
	if (m_sslContext == nullptr)
	{
		string fileLoc = string(SSL_ERROR_FILE_LOCATION);
		fileLoc += SSL_ERROR_FILE;
		LogSSLError(fileLoc);
		return m_bOpenSSL = false;
	}
	//Configure the SSL context.
	return m_bOpenSSL = ConfigureSSLContext();
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
		string fileName = string(CONNECTION_ERROR_FILE_LOCATION);
		fileName += CONNECTION_ERROR_FILE;
		LogError(fileName, error);
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
	//Log the OpenSSL error.
	ERR_print_errors_fp(err);
	fclose(err);
}

void ServerManager::LogError(const std::string& fileName, const std::string& errorMessage)
{
	FILE* err = fopen(fileName.c_str(), "w");
	//Log the error. Might be best to change this to C++ style to avoid any future issues.
	fputs(errorMessage.c_str(), err);
	fclose(err);
}

void ServerManager::LogConnection(const std::string & fileName, const std::string & message, int ip)
{
	fstream file = fstream(fileName, std::ios_base::in);
	string fullMessage = string(message);
	fullMessage += " from IP:" + ip;
	
	if (file.is_open())
	{
		file.write(fullMessage.c_str(), fullMessage.size() + 1);
		file.close();
	}
}

void ServerManager::LogLoginAttemp(const std::string & fileName, const std::string & user, bool successful)
{
	fstream file = fstream(fileName, std::ios_base::in);
	string message = string(successful ? "Successful " : "Unsuccessful ");
	message += " login attemp by " + user;

	if (file.is_open())
	{
		file.write(message.c_str(), message.size() + 1);
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

	return strlen(buffer);
}

void StartServer(std::shared_ptr<Server> server)
{
	server->Run();
}