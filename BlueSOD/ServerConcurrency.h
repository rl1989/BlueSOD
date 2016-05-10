#pragma once
#include <shared_mutex>

using std::shared_mutex;

//This guarantees that the Server will be modified by one thread at a time.
shared_mutex serverMutex;
//This guarantees that the state of the Server will be modified by one thread at a time.
shared_mutex serverStateMutex;