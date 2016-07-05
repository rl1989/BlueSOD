#pragma once
#include <deque>
#include <mutex>

/*
	A thread safe stack that allows the ServerManager to communicate with the thread that authenticates
	users. The producer (ServerManager) will push the object into the ThreadSafeDEQueue object and the 
	consumer (the other thread) will pop objects to retrieve them.

	The consumer will wait in a loop checking Empty() or Size() to see if there are objects available.
*/
template<typename T>
class ThreadSafeDEQueue
{
private:
	std::mutex m_mutex{};
	std::deque<T> m_deque{};
public:
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

	T& operator[](int n);
	typename std::deque<T>::const_iterator begin();
	typename std::deque<T>::const_iterator end();
};