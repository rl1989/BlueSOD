#pragma once
#include <string>
#include <fstream>
#include <openssl/err.h>

using std::fstream;
using std::string;

//Locations of error files
#define ERROR_LOGS_LOCATION "G:\\Ricky\\Documents\\Programming\\BlueSOD\\Error Logs\\"
#define ERROR_LOG "ErrorLog.txt"
#define SSL_ERROR_LOG "SSLServerErrorLog.txt"
#define CONNECTION_ERROR_LOG "ConnectionErrorLog.txt"

#define LOGIN_LOG_LOCATION "G:\\Ricky\\Documents\\Programming\\BlueSOD\\Login Attemps\\LoginAttempts.txt"

class LogManager
{
public:
	//Log the OpenSSL error into the SSL Error logfile.
	static void LogSSLError();
	//Log any non-OpenSSL errorMessage into fileName.
	//Arguments:
	//  const std::string& fileName - The location of the file to write errorMessage.
	//  const std::string& errorMessage - The error.
	static void LogError(const string& fileName, const string& errorMessage);
	/*
	Log connections into fileName.
	Arguments:
	const std::string& fileName - The location of the file to write message.
	const std::string& message - The message sent by ip.
	int ip - The ip address that sent the message.
	*/
	static void LogConnection(const string& fileName, int time, int ip);
	/*
	Log login attempts (whether they are successful or not) into fileName.
	Arguments:
	const std::string& fileName - The location of the file to write the message.
	const std::string& user - The user who attempted login.
	bool successful - Was the attempt successful?
	*/
	static void LogLoginAttemp(const string& fileName, const string& user, bool successful, int ip);
	static void LogSuccessfulLoginAttemp(const string& fileName, const string& user, int ip);
	static void LogUnsuccesfulLoginAttempt(const string& fileName, const string& user, int ip);
};

