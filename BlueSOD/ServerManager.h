#pragma once
#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <list>
#include <vector>
#include "Server.h"
#include "ServerConcurrency.h"
#include "CommonServer.h"

//The port the server will isten on.
#define SERVER_PORT 2048

//Locations of the certification and private key files.
#define SSL_PEM_LOCATIONS "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define SSL_PASSWORD_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define CERTIFICATE_FILE "ricky-anthony.asuscomm.com.cert.pem"
#define PRIVATE_KEY_FILE "ricky-anthony.asuscomm.com.key.pem"
//Locations of error files
#define SSL_ERROR_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\ErrorLogs\\"
#define CONNECTION_ERROR_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\ErrorLogs\\"
#define SSL_ERROR_FILE "SSLServerErrors.txt"
#define PASSWORD_FILE "password.txt"
#define CONNECTION_ERROR_FILE "ConnectionErrors.txt"

//Will be used to start the server. The function is passed off to std::thread with the server as an argument.
void StartServer(std::shared_ptr<Server> server);

/*
	The ServerManager will manage initial connections. That is, when it receives a connection request from
	a client for the first time, it will handle the user authentication. If the user is authorized, ServerManager
	will pass off the connection to a Server object which will then handle the messages sent between clients.

	The ServerManager will be established on its own thread, as not to impose any delays on message delivery.
	Ideally, it will be hosted on its own machine and then it will communicate with the Server on a different
	machine.
*/
//TO DO: Allow the administrator to have the ServerManager listen on multiple ports (and how many?).
//TO DO: Throw error from Run() if WSA is not initialized?
//TO DO: Determine if it is necessary to provide a mutex for Run().
//TO DO: Change any FILE uses to fstream.
//TO DO: Changed shared_mutex to unique_mutex. I do not foresee sharing these mutexes between threads.
//       This can always be changed in the future.
class ServerManager
{
private:
	//The socket that the ServerManager listens for connections on.
	SOCKET m_listenerSocket;
	//The port ServerManager will be listening on.
	int m_portNumber;
	//This socket will be passed to the Server once a connection has been made.
	SOCKET m_clientSocket;
	//The SSL context to be used.
	SSL_CTX* m_sslContext;
	//This represents the SSL connection. It will be passed to the Server.
	SSL* m_clientSSL;
	//Determines if the client is connecting through SSL.
	//May be removed in the future.
	bool m_bClientHasSSLConnection;
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
	shared_mutex m_runMutex;


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
		m_clientSocket{ INVALID_SOCKET },
		m_sslContext{ nullptr },
		m_clientSSL{ nullptr },
		m_bClientHasSSLConnection{ false },
		m_bWSA{ false },
		m_bOpenSSL{ false },
		m_state{ ServerState::OFF },
		m_server{ nullptr },
		m_stateMutex{},
		m_serverManagerMutex{},
		m_portMutex{}

	{}
	~ServerManager();
	//Run the ServerManager with the specified state.
	void Run(ServerState state = ServerState::RUNNING);
	//Stops the ServerManager. The ServerManager will not immediately shut down, but a signal
	//will be sent to inform it to shut down.
	void Stop();
	//Returns the state of the ServerManager.
	ServerState GetState();
	//Modifies the state of the ServerManager.
	void SetState(ServerState state);
	//Returns the port number the ServerManger is listening on.
	int GetPortNumber();
	//Modifies the port that the ServerManager will listen to. Will send a reset connection
	//signal to clients along with the new port number.
	void SetPortNumber(int port);

private:
	//Initializes WSA and OpenSSL.
	//Returns false if OpenSSL or both are not ready, true otherwise. i.e. WSA may be ready
	//but not OpenSSL and this will still return true. The ServerManager is ready to listen
	//for connections.
	bool Init();
	//Cleans up WSA and OpenSSL libraries. Required if they were initialized.
	void Cleanup();
	//Open the ServerManager for connections on port.
	//Arguments:
	//  int port - The port to listen on.
	//Return value:
	//  true  - Listening on port.
	//  false - An error occurred. Must consult WSA for error information.
	bool OpenForConnections(int port);

	//Accept an incoming connection.
	//Return value:
	//  The socket of the accepted connection (i.e. the client).
	//  INVALID_SOCKET is returned if there was an error connecting to the client.
	SOCKET AcceptIncomingConnection();
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
	bool ConfigureSSLContext();
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
	//Log the OpenSSL error into fileName.
	//Argument:
	//  const std::string& fileName - The location of the file to write the error message.
	void LogSSLError(const std::string& fileName);
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
	void LogLoginAttemp(const std::string& fileName, const std::string& user, bool successful);

	//A callback function used in the OpenSSL library. May be replaced with a lambda 
	//function in the future.
	static int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data);
};

