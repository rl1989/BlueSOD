#include "ServerConcurrency.h"

using std::move;

template<typename T>
inline ThreadSafe<T>::ThreadSafe(const T & o)
	:m_object{ o }
{}

template<typename T>
inline ThreadSafe<T>::ThreadSafe(T && o)
	: m_object{ o }
{}

template<typename T>
ThreadSafe<T>::ThreadSafe()
	: m_object{}
{}

template<typename T>
inline ThreadSafe<T>::~ThreadSafe()
{
	lock_guard<mutex> lck(m_mutex);
}

template<typename T>
ThreadSafe<T>& ThreadSafe<T>::operator=(const T& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object = o;

	return *this;
}

template<typename T>
ThreadSafe<T>& ThreadSafe<T>::operator=(T&& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object = o;

	return *this;
}

template<typename T>
ThreadSafe<T>& ThreadSafe<T>::operator+(const T& o)
{
	return *this += o;
}

template<typename T>
ThreadSafe<T>& ThreadSafe<T>::operator+=(const T& o)
{
	lock_guard<mutex> lck(m_mutex);

	m_object += o;

	return *this;
}

template<typename T>
ThreadSafe<T>& ThreadSafe<T>::operator++()
{
	return *this += 1;
}

template<typename T>
ThreadSafe<T>::operator T() const
{
	lock_guard<mutex> lck(m_mutex);

	return m_object;
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

template<typename T>
bool ThreadSafe<T>::TryLock()
{
	return m_mutex.try_lock();
}

template<typename T>
void ThreadSafe<T>::Lock()
{
	m_mutex.lock();
}

template<typename T>
void ThreadSafe<T>::LockChange(const T& o)
{
	m_mutex.lock();
	
	m_object = o;
}

template<typename T>
void ThreadSafe<T>::LockChange(T&& o)
{
	m_mutex.lock();

	m_object = o;
}

template<typename T>
void ThreadSafe<T>::Unlock()
{
	m_mutex.unlock();
}

template<typename T>
T&& ThreadSafe<T>::MoveObject()
{
	lock_guard<mutex> lck(m_mutex);

	return move(m_object);
}
