#pragma once
#include "TS_Deque.h"
using std::deque;
using std::mutex;
using std::lock_guard;

template<typename T>
T& TS_Deque<T>::Back()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.back();
}

template<typename T>
inline T&& TS_Deque<T>::MoveBack()
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
inline T&& TS_Deque<T>::MoveFront()
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

template<typename T>
T& TS_Deque<T>::operator[](int n)
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque[n];
}

template<typename T>
typename std::deque<T>::const_iterator TS_Deque<T>::begin()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.begin();
}

template<typename T>
typename std::deque<T>::const_iterator TS_Deque<T>::end()
{
	lock_guard<mutex> lck(m_mutex);

	return m_deque.end();
}
