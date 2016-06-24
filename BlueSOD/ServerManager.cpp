#pragma once
#include "ServerManager.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

#define REJECTION_MESSAGE ""
#define VERIFIED_MESSAGE ""

using std::string;
using std::fstream;
using std::move;
using std::thread;

inline void StartServer(Server* server, ServerState state)
{
	server->Run(state);
}

inline void StartUserVerifier(UserVerifier* uv, ServerState state)
{
	uv->Run(state);
}

ServerManager::~ServerManager()
{
	Shutdown();
}

bool ServerManager::Init()
{
	//Save the state to change it back after everything is initialized
	ServerState state = GetState();
	SetState(ServerState::START_UP);

	//WSA isn't initialized so initialize it.
	if (!m_bWSA)
	{
		//WSA is not available. Cannot proceed without WSA.
		if (!InitWSA())
		{
			string error = string();
			error += time(nullptr);
			error += " WSA is not initialized.";
			LogManager::LogError(CONNECTION_ERROR_LOG, error);
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
			LogManager::LogError(CONNECTION_ERROR_LOG, error);
		}
	}

	SetState(state);

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
	Shutdown();
	Init();
	closesocket(m_listenerSocket);
	if (!OpenForConnections(GetPortNumber()))
	{
		string errorMsg = string();
		errorMsg += time(nullptr);
		errorMsg += " Could not open port ";
		errorMsg += GetPortNumber();
		errorMsg += " for listening on ServerManager.";
		LogManager::LogError(CONNECTION_ERROR_LOG, errorMsg);

		SetState(ServerState::NOT_ACCEPTING_CONNECTIONS);

		return false;
	}
	SetState(ServerState::RUNNING);
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

ConnectionInfo ServerManager::AcceptIncomingNewConnection()
{
	ConnectionInfo ci{};

	if (!m_bWSA || GetState() != ServerState::RUNNING)
	{
		return ci;
	}

	ci.Accept(m_listenerSocket, m_sslContext);

	return ci;
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

	SetState(state);
	ServerState curState = state;
	while (curState != ServerState::OFF)
	{
		switch (curState)
		{
			case ServerState::START_UP:
				{
					int port = GetPortNumber();
					if (!OpenForConnections(port))
					{
						string fName, msg;
						fName = ERROR_LOGS_LOCATION;
						fName += CONNECTION_ERROR_LOG;
						msg = time(nullptr);
						msg += " Could not open for connection on port ";
						msg += port;
						LogManager::LogError(fName, msg);

						return false;
					}
					else
					{
						SetState(ServerState::RUNNING);
					}
				}
				break;

			/*A new port was requested to be used by the administrator. Close the current socket and
			open a new one. Then send a message to any clients telling them to use the new port.*/
			case ServerState::RESET:
				if (!Reset())
					return false;
				break;
			
			/*Connect with any incoming clients.*/
			case ServerState::RUNNING:
				ConnectionInfo ci = move(AcceptIncomingNewConnection());

				if ( !(ci.IsValid()) )
				{
					string fileName = string(ERROR_LOGS_LOCATION);
					fileName += ERROR_LOG;
					string message;
					message += time(nullptr);
					message += " Could not accept connection on port ";
					message += GetPortNumber();
					message += ".";
					LogManager::LogError(fileName, message);

					break;
				}
				else
				{
					/* Send Connection off to be verified. */
					//m_userVerifier.AddPendingConnection(move(ci));
					m_userVerifier.AddPendingConnection(move(ci));
				}

				if (m_userVerifier.HasVerifiedConnections())
				{
					for (int i = 0; i < m_userVerifier.NumVerifiedConnections(); i++)
					{
						//NewConnectionInfo ci = move(m_userVerifier.PopVerifiedConnection());
						//string msg = VERIFIED_MESSAGE;
						///* Send a verified message. */
						//ci.Send(msg);
						///* Send verified connection to m_server. */
						////m_server.AddClient(move(ci));
					}
				}
				if (m_userVerifier.HasRejectedConnections())
				{
					for (int i = 0; i < m_userVerifier.NumRejectedConnections(); i++)
					{
						//NewConnectionInfo ci = move(m_userVerifier.PopRejectedConnection());
						//string msg = REJECTION_MESSAGE;
						///* Send a rejection message. */
						//ci.Send(msg);
						///* NewConnectionInfo's destructor calls Close() on ci.connection, so no need to 
						//   explicitly call it. */
					}
				}
				break;
		}

		//m_server.SetState(curState);
		m_userVerifier.SetState(curState);

		curState = GetState();
	}

	m_runMutex.unlock();

	return true;
}

inline void ServerManager::Shutdown()
{
	SetState(ServerState::OFF);
	//m_server.SetState(ServerState::OFF);
	m_userVerifier.SetState(ServerState::OFF);

	if (m_serverThread.joinable())
		m_serverThread.join();
	if (m_uvThread.joinable())
		m_uvThread.join();
	Cleanup();
}

ServerState ServerManager::GetState()
{
	return m_tsState.RetrieveObject();
}

void ServerManager::SetState(ServerState state)
{
	m_tsState.ChangeObject(state);
}

int ServerManager::GetPortNumber()
{
	return m_tsPortNumber.RetrieveObject();
}

void ServerManager::SetPortNumber(int port)
{
	m_tsPortNumber.ChangeObject(port);

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
		LogManager::LogError(CONNECTION_ERROR_LOG, error);

		return INVALID_SOCKET;
	}

	/* Make socket non blocking. */
	{
		u_long argp = 1;
		long cmd = FIONBIO;

		ioctlsocket(socket, cmd, &argp);
	}

	//Bind socket.
	if (::bind(socket, (struct sockaddr*)&addr, sizeof(addr)) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not bind socket " + static_cast<int>(socket);
		LogManager::LogError(CONNECTION_ERROR_LOG, error);

		closesocket(socket);
		return INVALID_SOCKET;
	}

	//Listen on socket.
	if (listen(socket, 0) == INVALID_SOCKET)
	{
		string error = string();
		error += time(nullptr);
		error += " Could not listen on socket " + static_cast<int>(socket);
		LogManager::LogError(CONNECTION_ERROR_LOG, error);

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
		LogManager::LogSSLError();
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
		LogManager::LogSSLError();

		return false;
	}
	//Tell OpenSSL where to find our password.
	SSL_CTX_set_default_passwd_cb(ctx, PasswordCallBack);
	//Tell OpenSSL where to find our private key.
	if (SSL_CTX_use_PrivateKey_file(ctx, pkPath.c_str(), SSL_FILETYPE_PEM) < 0)
	{
		LogManager::LogSSLError();

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
		LogManager::LogSSLError();
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
	m_bOpenSSL = false;
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
		LogManager::LogError(CONNECTION_ERROR_LOG, error);
		CleanupWSA();
		return m_bWSA = false;
	}
	return m_bWSA = true;
}

void ServerManager::CleanupWSA()
{
	if (!m_bWSA)
		WSACleanup();
	m_bWSA = false;
}

void ServerManager::CloseConnections()
{
	if (m_listenerSocket)
		closesocket(m_listenerSocket);
}

int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data)
{
	FILE* file;
	//Load the name of the password file.
	string fName = string(SSL_PASSWORD_FILE_LOCATION);
	fName += PASSWORD_FILE;

	file = fopen(fName.c_str(), "r");
	//Get the password.
	fgets(buffer, sizeOfBuffer - 1, file);
	buffer[sizeOfBuffer] = 0;

	fclose(file);

	return static_cast<int>(strlen(buffer));
}