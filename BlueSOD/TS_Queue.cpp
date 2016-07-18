#include "TS_Queue.h"

using std::move;
using std::lock_guard;
using std::mutex;

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue& tsq)
	: m_mutex{},
	m_queue{tsq.m_queue}
{}
template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
	: m_mutex{},
	m_queue{}
{}
template<typename T>
inline void ThreadSafeQueue<T>::push(T&& t)
{
	lock_guard<mutex> lck(m_mutex);

	m_queue.push(t);
}

template<typename T>
void ThreadSafeQueue<T>::push(const T& t)
{
	lock_guard<mutex> lck(m_mutex);

	m_queue.push(t);
}

template<typename T>
inline void ThreadSafeQueue<T>::pop()
{
	lock_guard<mutex> lck(m_mutex);

	m_queue.pop();
}

template<typename T>
inline T& ThreadSafeQueue<T>::front()
{
	lock_guard<mutex> lck(m_mutex);
	
	return m_queue.front();
}

template<typename T>
int ThreadSafeQueue<T>::size()
{
	lock_guard<mutex> lck(m_mutex);

	return static_cast<int>(m_queue.size());
}

template<typename T>
inline bool ThreadSafeQueue<T>::empty()
{
	lock_guard<mutex> lck(m_mutex);

	return m_queue.empty();
}
