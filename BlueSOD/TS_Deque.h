#pragma once
#include <deque>
#include <mutex>

using std::deque;
using std::mutex;
using std::lock_guard;

/*
	A thread safe stack that allows the ServerManager to communicate with the thread that authenticates
	users. The producer (ServerManager) will push the object into the TS_Queue object and the 
	consumer (the other thread) will pop objects to retrieve them.

	The consumer will wait in a loop checking Empty() or Size() to see if there are objects available.
*/
template<typename T>
class TS_Deque
{
private:
	mutex m_mutex;
	deque<T> m_deque;
public:
	TS_Deque() = default;
	~TS_Deque() = default;

	T& Back();
	T&& MoveBack();
	void PushBack(const T& e);
	void PushBack(T&& e);
	void PopBack();
	T& Front();
	T&& MoveFront();
	void PushFront(const T& e);
	void PushFront(T&& e);
	void PopFront();
	int Size();
	bool Empty();
};