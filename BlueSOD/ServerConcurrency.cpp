#include "ServerConcurrency.h"

template<typename T>
inline ThreadSafe<T>::ThreadSafe(const T & o)
	:m_object{ o }
{
}

template<typename T>
inline ThreadSafe<T>::ThreadSafe(T && o)
	: m_object{ o }
{
}

template<typename T>
inline ThreadSafe<T>::~ThreadSafe()
{
	lock_guard<mutex> lck(m_mutex);
}

template<typename T>
inline void ThreadSafe<T>::ChangeObject(const T& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object = o;
}

template<typename T>
inline void ThreadSafe<T>::ChangeObject(T& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object = o;
}

template<typename T>
inline void ThreadSafe<T>::ChangeObject(T&& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object = o;
}

template<typename T>
inline const T& ThreadSafe<T>::RetrieveObject()
{
	lock_guard<mutex> lck(m_mutex);

	return m_object;
}