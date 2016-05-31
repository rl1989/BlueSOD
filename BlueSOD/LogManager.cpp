#include "LogManager.h"

void LogManager::LogSSLError()
{
	string fName = string(ERROR_LOGS_LOCATION);
	fName += SSL_ERROR_LOG;
	FILE* err = fopen(fName.c_str(), "w");
	//Log the OpenSSL error.
	ERR_print_errors_fp(err);
	fclose(err);
}

void LogManager::LogError(const string & fileName, const string & errorMessage)
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

void LogManager::LogConnection(const string & fileName, int time, int ip)
{
	fstream file = fstream(fileName, std::ios_base::in);
	string fullMessage;
	fullMessage += time;
	fullMessage += " Could not accept connection from IP address:";
	fullMessage += ip;

	if (file.is_open())
	{
		file << fullMessage.c_str() << std::endl;
		file.close();
	}
}

void LogManager::LogLoginAttemp(const string & fileName, const string & user, bool successful, int ip)
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

