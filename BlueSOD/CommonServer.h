#pragma once
#include <memory>
//Defines the state of the server. Names are self explanatory.
enum ServerState
{
	OFF, RUNNING, NOT_ACCEPTING_CONNECTIONS, RESET
};