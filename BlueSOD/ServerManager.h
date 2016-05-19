#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Server.h"
#include "ServerConcurrency.h"
#include "CommonServer.h"
#include "SSL_concurrency.h"

using std::string;

/*
	FINALLY FOUND HOW IT WORKS:

	SSL_accept does not call accept for you. It expects that accept has already been called.

	The correct sequence of calls is:

	socket
	bind
	listen
	accept
	SSL_new
	SSL_set_fd
	SSL_accept
*/

//The port the server will isten on.
#define SERVER_PORT 2048

//Locations of the certification and private key files.
#define SSL_PEM_LOCATIONS "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define SSL_PASSWORD_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define CERTIFICATE_FILE "ricky-anthony.asuscomm.com.cert.pem"
#define PRIVATE_KEY_FILE "ricky-anthony.asuscomm.com.key.pem"
#define PASSWORD_FILE "password.txt"
//Locations of error files

#define ERROR_LOGS_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\ErrorLogs\\"
#define ERROR_LOG "ErrorLog.txt"
#define SSL_ERROR_LOG "SSLServerErrorLog.txt"
#define CONNECTION_ERROR_LOG "ConnectionErrorLog.txt"

//Will be used to start the server. The function is passed off to std::thread with the server as an argument.
void StartServer(std::shared_ptr<Server> server);

/*
	Tells whether a connection was accepted or if a connection is still waiting.
*/
enum class SocketStatus
{ ACCEPTED, LISTENING };

/*
	The ServerManager will manage initial connections. That is, when it receives a connection request from
	a client for the first time, it will handle the user authentication. If the user is authorized, ServerManager
	will pass off the connection to a Server object which will then handle the messages sent between clients.

	The ServerManager will be established on its own thread, as not to impose any delays on message delivery.
	Ideally, it will be hosted on its own machine and then it will communicate with the Server on a different
	machine.
*/

//TO DO: Throw error from Run() if WSA is not initialized?
//TO DO: Change any FILE uses to fstream.
class ServerManager
{
private:
	//The socket that the ServerManager listens for connections on.
	SOCKET m_listenerSocket;
	//The port ServerManager will be listening on.
	int m_portNumber;
	//The SSL context to be used.
	SSL_CTX* m_sslContext;
	//Determines if WSA was properly initialized. Connections cannot be made on a Windows
	//machine unless WSA was properly initialized.
	bool m_bWSA;
	//Determines if OpenSSL was properly initialized. SSL connections cannot be made unless
	//the OpenSSL library was properly initialized.
	bool m_bOpenSSL;
	//The state of the ServerManager.
	ServerState m_state;
	//The Server. This is where connections will be passed to once they are initialized and the
	//user is logged in.
	std::shared_ptr<Server> m_server;
	//This guarantees that the state of the ServerManager will be modified by one thread at a time.
	shared_mutex m_stateMutex;
	//This guarantees that the ServerManager will be modified by one thread at a time.
	shared_mutex m_serverManagerMutex;
	//This guarantees that the port will only be modified by one thread at a time.
	shared_mutex m_portMutex;
	//This guarantees that Run() is not called more than once at a time. Calling Run()
	//from more than one thread will cause problems. However, this may be unnecessary.
	mutex m_runMutex;

public:
	//The default constructor.
	ServerManager()
		: ServerManager(SERVER_PORT)
	{}
	//This constructor allows the administrator to listen on whatever port
	//he/she deems necessary.
	ServerManager(int port)
		: m_listenerSocket{ INVALID_SOCKET },
		m_portNumber{ port },
		m_sslContext{ nullptr },
		m_bWSA{ false },
		m_bOpenSSL{ false },
		m_state{ ServerState::OFF },
		m_server{ nullptr },
		m_stateMutex{},
		m_serverManagerMutex{},
		m_portMutex{},
		m_runMutex{}

	{}
	~ServerManager();
	//Run the ServerManager with the specified state.
	bool Run(ServerState state = ServerState::RUNNING);
	//Stops the ServerManager. The ServerManager will not immediately shut down, but a signal
	//will be sent to inform it to shut down.
	inline void Stop() { SetState(ServerState::OFF); }
	//Returns the state of the ServerManager.
	inline ServerState GetState();
	//Modifies the state of the ServerManager.
	inline void SetState(ServerState state);
	//Returns the port number the ServerManger is listening on.
	inline int GetPortNumber();
	//Modifies the port that the ServerManager will listen to. Will send a reset connection
	//signal to clients along with the new port number.
	inline void SetPortNumber(int port);

private:
	/*
		Initializes WSA and OpenSSL.
		
		Return value:
			false - OpenSSL or both (OpenSSL and WSA) are not ready, true otherwise. i.e. WSA may be ready
			but not OpenSSL and this will still return true. The ServerManager is ready to listen
			for connections. If OpenSSL does not initialize, then SSL connections will not be accepted.
			Call Encrypted() to determine OpenSSL initialization status.
	*/
	bool Init();
	/*
		Closes any connections, shuts down multithreading support in OpenSSL, cleans up OpenSSL
		and WSA.
	*/
	void Cleanup();
	/*
		Open the ServerManager for connections on port. If there is another open socket, it
		will be closed. MAY GET DELETED.

		Arguments:
			int port - The port to listen on.
		Return value:
		    true  - Listening on port.
			false - An error occurred. Must consult WSA for error information.
	*/
	bool OpenForConnections(int port);
	inline bool IsListening();
	/*
		Accept an incoming connection. MAY GET DELETED.
		
		Return value:
			The socket of the accepted connection (i.e. the client).
			INVALID_SOCKET is returned if there was an error connecting to the client.
	*/
	ClientInfo AcceptIncomingConnection();
	//Stop accepting connections. Sets the state of the ServerManager to NOT_ACCEPTING_CONNECTIONS
	//and closes the client socket and the listening socket (if available).
	void StopAcceptingConnections();
	//Creates the listening socket on port.
	//Arguments:
	//  int port - The port to create the listening socket on.
	//Return value:
	//  The socket of the listening socket.
	//  INVALID_SOCKET is returned if there was an error creating the socket.
	SOCKET CreateSocket(int port);
	//Creates the SSL context object needed to create SSL objects.
	//Return value:
	//  The SSL context object.
	//  nullptr if there was an error.
	SSL_CTX* CreateSSLContext();
	//Configures the SSL context.
	//Return value:
	//  true - The context was properly configured.
	//  false - There was an error. The error will be logged.
	bool ConfigureSSLContext(SSL_CTX* ctx);
	//Initialize OpenSSL. This will create an SSL context object and will configure it. Thus,
	//calls to CreateSSLContext() and ConfigureSSLContext() are made redundant unless something
	//changes.
	//Return value:
	//  true - OpenSSL was properly intialized.
	//  false - OpenSSL was not properly intialized. The error will be logged.
	bool InitOpenSSL();
	//Cleanup the OpenSSL library. This must be made upon closure if OpenSSL was
	//first properly initialized.
	void CleanupOpenSSL();
	//Initialize WSA. This must be called before any attempt to connect to an unsecure connection.
	//Return value:
	//  true - WSA was initialized.
	//  false - WSA was not initialized. The error will be logged.
	bool InitWSA();
	//Cleanup WSA. This must be called before the program ends if WSA was previously
	//initialized.
	void CleanupWSA();
	/*
		Close any connections.
	*/
	void CloseConnections();
	//Log the OpenSSL error into the SSL Error logfile.
	void LogSSLError();
	//Log any non-OpenSSL errorMessage into fileName.
	//Arguments:
	//  const std::string& fileName - The location of the file to write errorMessage.
	//  const std::string& errorMessage - The error.
	void LogError(const std::string& fileName, const std::string& errorMessage);
	/*
		Log connections into fileName.
		Arguments:
		  const std::string& fileName - The location of the file to write message.
		  const std::string& message - The message sent by ip.
		  int ip - The ip address that sent the message.
	*/
	void LogConnection(const std::string& fileName, const std::string& message, int ip);
	/*
		Log login attempts (whether they are successful or not) into fileName.
		Arguments:
		  const std::string& fileName - The location of the file to write the message.
		  const std::string& user - The user who attempted login.
		  bool successful - Was the attempt successful?
	*/
	void LogLoginAttemp(const std::string& fileName, const std::string& user, bool successful, int ip);

	/*
		Adds a client to m_server. If m_server is not initialized, then this function will
		initialize it.
		
		Arguments:
		  SOCKET socket - The client being added.
	*/
	void AddClient(SOCKET socket);

	//A callback function used in the OpenSSL library. May be replaced with a lambda 
	//function in the future.
	static int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data);
};

