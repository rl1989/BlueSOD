#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include "Server.h"
#include "ServerConcurrency.h"
#include "ServerConnections.h"
#include "SSL_concurrency.h"
#include "UserVerifier.h"
#include "TS_Deque.h"
#include "LogManager.h"

extern void thread_cleanup();
extern void thread_setup();
extern void win32_locking_callback(int, int, char*, int);

using std::string;
using std::unique_ptr;
using std::shared_ptr;

//The port the server will isten on.
#define SERVER_PORT 2048

//Locations of the certification and private key files.
#define SSL_PEM_LOCATIONS "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define SSL_PASSWORD_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSL\\PrivateKeys\\"
#define CERTIFICATE_FILE "ricky-anthony.asuscomm.com.cert.pem"
#define PRIVATE_KEY_FILE "ricky-anthony.asuscomm.com.key.pem"
#define PASSWORD_FILE "password.txt"
//Location of user DB
#define USER_DB "G:\\Ricky\\Documents\\Programming\\BlueSOD\\Databases\\UserInfo.db"

/*
	The ServerManager will manage initial connections. That is, when it receives a connection request from
	a client for the first time, it will handle the user authentication. If the user is authorized, ServerManager
	will pass off the connection to a Server object which will then handle the messages sent between clients.

	The ServerManager will be established on its own thread, as not to impose any delays on message delivery.
	Ideally, it will be hosted on its own machine and then it will communicate with the Server on a different
	machine.
*/
class ServerManager
{
private:
	//The socket that the ServerManager listens for connections on.
	SOCKET m_listenerSocket;
	/*
		The port to listen on. Will possibly be modified by more than one thread so it needs to be thread safe.
	*/
	ThreadSafe<int> m_tsPortNumber;
	//The SSL context to be used.
	SSL_CTX* m_sslContext;
	//Determines if WSA was properly initialized. Connections cannot be made on a Windows
	//machine unless WSA was properly initialized.
	bool m_bWSA;
	//Determines if OpenSSL was properly initialized. SSL connections cannot be made unless
	//the OpenSSL library was properly initialized.
	bool m_bOpenSSL;
	/*
		The state of the ServerManager. Will possibly be accessed by more than one thread, so it needes
		to be thread safe.
	*/
	ThreadSafe<ServerState> m_tsState;
	//The Server. This is where connections will be passed to once they are initialized and the
	//user is logged in.
	unique_ptr<Server> m_server;
	//This guarantees that Run() is not called more than once at a time. Calling Run()
	//from more than one thread will cause problems. However, this may be unnecessary.
	mutex m_runMutex;
	/*
		Verifies user login information on a separate thread.
	*/
	UserVerifier m_userVerifier;

public:
	//The default constructor.
	ServerManager()
		: ServerManager(SERVER_PORT, USER_DB)
	{}
	//This constructor allows the administrator to listen on whatever port
	//he/she deems necessary.
	ServerManager(int port, const string& dbName)
		: m_listenerSocket{ INVALID_SOCKET },
		m_tsPortNumber{ port },
		m_sslContext{ nullptr },
		m_bWSA{ false },
		m_bOpenSSL{ false },
		m_tsState{ ServerState::OFF },
		m_server{ nullptr },
		m_runMutex{},
		m_userVerifier{ dbName }
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
		Reset the listening socket.
	*/
	bool Reset();
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
		Accept an incoming connection.

		Arguments:
			ci - The information regarding the incoming connection.
		Return value:
			CONNECTION_ACCEPTED - A connection was successfully accepted and ci contains valid information.
			NOT_OK - An error occurred and ci is not valid.
			NO_CONNECTION_PRESENT - There was no connection present.
	*/
	ConnectionInfo AcceptIncomingConnection();

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
	/*
		Send a message to the client.
	*/
	void Send(ConnectionInfo* ci, const std::string& msg);
};

int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data);