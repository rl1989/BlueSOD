#pragma once
#include <deque>
#include <mutex>

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
	std::mutex m_mutex;
	std::deque<T> m_deque;
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

	std::deque<T>::const_reference operator[](int n);
	std::deque<T>::const_iterator begin();
	std::deque<T>::const_iterator end();
};