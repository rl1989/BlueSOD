#include "ThreadSafeList.h"

using std::mutex;
using std::lock_guard;

template<typename T>
void ThreadSafeList<T>::push_back(const T& elem)
{
	lock_guard<mutex> lck(m_mutex);

	m_list.push_back(elem);
}

template<typename T>
void ThreadSafeList<T>::push_back(T&& elem)
{
	lock_guard<mutex> lck(m_mutex);

	m_list.push_back(elem);
}

template<typename T>
void ThreadSafeList<T>::erase(typename std::list<T>::iterator elem)
{
	lock_guard<mutex> lck(m_mutex);

	m_list.erase(elem);
}

template<typename T>
int ThreadSafeList<T>::size()
{
	lock_guard<mutex> lck(m_mutex);

	return static_cast<int>(m_list.size());
}

template<typename T>
bool ThreadSafeList<T>::empty()
{
	lock_guard<mutex> lck(m_mutex);

	return m_list.empty();
}

template<typename T>
typename std::list<T>::iterator ThreadSafeList<T>::begin()
{
	lock_guard<mutex> lck(m_mutex);

	return m_list.begin();
}

template<typename T>
typename std::list<T>::iterator ThreadSafeList<T>::end()
{
	lock_guard<mutex> lck(m_mutex);

	return m_list.end();
}

template<typename T>
void ThreadSafeList<T>::clear()
{
	lock_guard<mutex> lck(m_mutex);

	m_list.clear()
}

template<typename T>
void ThreadSafeList<T>::lock()
{
	m_mutex.lock();
}

template<typename T>
void ThreadSafeList<T>::unlock()
{
	m_mutex.unlock();
}
