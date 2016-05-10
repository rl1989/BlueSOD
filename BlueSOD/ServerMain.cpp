#include "ServerConcurrency.h"
#include "ServerManager.h"
#include <cstring>
#include <Windows.h>

#define NUM_CMDS 2
#define RUN_CMD  0
#define QUIT_CMD 1

#define APPLICATION_MUTEX_NAME "Global\\BSODApplication"
#define SERVER_MANAGER_MUTEX_NAME "Global\\BSOD\\ServerManager"

HANDLE applicationMutex;

void RunServer();

int main(int argc, char *argv[])
{
	applicationMutex = CreateMutex(nullptr, true, APPLICATION_MUTEX_NAME);
	int result = 0;
	//An element is true if the command has already been passed. This prevents duplicate
	//commands from running.
	bool cmdFlags[NUM_CMDS];

	//Init command flags
	ZeroMemory(cmdFlags, NUM_CMDS);
	
	for (int i = 0; i < argc; i++)
	{
		// LOOP THROUGH COMMANDS
		if (strcmp(argv[i], "-r") == 0 && !cmdFlags[RUN_CMD])
		{
			//Run ServerManager
			cmdFlags[RUN_CMD] = true;
		}
		else if (strcmp(argv[i], "--run") == 0 && !cmdFlags[RUN_CMD])
		{
			//Run ServerManager
			cmdFlags[RUN_CMD] = true;
		}
		else if (strcmp(argv[i], "-q") == 0 && !cmdFlags[QUIT_CMD])
		{
			//Stop ServerManager
			cmdFlags[QUIT_CMD] = true;
		}
		else if (strcmp(argv[i], "--quit") == 0 && !cmdFlags[QUIT_CMD])
		{
			//Stop ServerManager
			cmdFlags[QUIT_CMD] = true;
		}
	}
	CloseHandle(applicationMutex);
	return result;
}