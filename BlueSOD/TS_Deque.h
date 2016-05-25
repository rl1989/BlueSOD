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
	void PushBack(const T& e);
	void PushBack(T&& e);
	void PopBack();
	T& Front();
	void PushFront(const T& e);
	void PushFront(T&& e);
	void PopFront();
	int Size();
	bool Empty();
};

template<typename T>
T& TS_Deque<T>::Back()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.back();
}

template<typename T>
void TS_Deque<T>::PushBack(const T& e)
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.push_back(e);
}

template<typename T>
void TS_Deque<T>::PushBack(T&& e)
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.push_back(e);
}

template<typename T>
void TS_Deque<T>::PopBack()
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.pop_back();
}

template<typename T>
T& TS_Deque<T>::Front()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.front();
}

template<typename T>
void TS_Deque<T>::PushFront(const T & e)
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.push_front(e);
}

template<typename T>
void TS_Deque<T>::PushFront(T && e)
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.push_front(e);
}

template<typename T>
void TS_Deque<T>::PopFront()
{
	lock_guard<mutex> lck(m_mutex);

	m_deque.pop_front();
}

template<typename T>
int TS_Deque<T>::Size()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.size();
}

template<typename T>
bool TS_Deque<T>::Empty()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.empty();
}