#pragma once
#include "TS_Deque.h"

template<class T>
T& TS_Deque<T>::Back()
{
	lock_guard<mutex> lck(m);

	return m_deque.back();
}

template<class T>
void TS_Deque<T>::PushBack(const T & e)
{
	lock_guard<mutex> lck(m);

	m_deque.push_back(e);
}

template<class T>
void TS_Deque<T>::PushBack(T&& e)
{
	lock_guard<mutex> lck(m);

	m_deque.push_back(e);
}

template<class T>
void TS_Deque<T>::PopBack()
{
	lock_guard<mutex> lck(m);

	m_deque.pop_back();
}

template<class T>
T & TS_Deque<T>::Front()
{
	lock_guard<mutex> lck(m);

	return m_deque.front();
}

template<class T>
void TS_Deque<T>::PushFront(const T & e)
{
	lock_guard<mutex> lck(m);

	m_deque.push_front(e);
}

template<class T>
void TS_Deque<T>::PushFront(T && e)
{
	lock_guard<mutex> lck(m);

	m_deque.push_front(e);
}

template<class T>
void TS_Deque<T>::PopFront()
{
	lock_guard<mutex> lck(m);

	m_deque.pop_front();
}

template<class T>
int TS_Deque<T>::Size()
{
	lock_guard<mutex> lck(m);

	return m_deque.size();
}

template<class T>
bool TS_Deque<T>::Empty()
{
	lock_guard<mutex> lck(m);

	return m_deque.empty();
}
