#include "TS_Queue.h"
template<typename T>
inline void ThreadSafeQueue<T>::push(T&& t)
{
	std::lock_guard<std::mutex> lck(m_mutex);

	m_queue.push(t);
}

template<typename T>
inline void ThreadSafeQueue<T>::pop()
{
	std::lock_guard<std::mutex> lck(m_mutex);

	m_queue.pop();
}

template<typename T>
inline T&& ThreadSafeQueue<T>::front()
{
	std::lock_guard<std::mutex> lck(m_mutex);

	return m_queue.front();
}

template<typename T>
inline T&& ThreadSafeQueue<T>::back()
{
	std::lock_guard<std::mutex> lck(m_mutex);

	return m_queue.back();
}

template<typename T>
int ThreadSafeQueue<T>::size()
{
	std::lock_guard<std::mutex> lck(m_mutex);

	return static_cast<int>(m_queue.size());
}

template<typename T>
inline bool ThreadSafeQueue<T>::empty()
{
	std::lock_guard<std::mutex> lck(m_mutex);

	return m_queue.size();
}
