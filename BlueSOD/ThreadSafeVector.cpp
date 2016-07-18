#include "ThreadSafeVector.h"

using std::vector;
using std::mutex;
using std::lock_guard;
using std::move;

template<typename T>
void ThreadSafeVector<T>::reserve(int space)
{
	lock_guard<mutex> lck(m_mutex);

	m_vector.reserve(space);
}

template<typename T>
T& ThreadSafeVector<T>::elem(int i)
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.at(i);
}

template<typename T>
T & ThreadSafeVector<T>::operator[](int i)
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector[i];
}

template<typename T>
void ThreadSafeVector<T>::clear()
{
	lock_guard<mutex> lck(m_mutex);

	m_vector.clear();
}

template<typename T>
inline void ThreadSafeVector<T>::push_back(T&& val)
{
	lock_guard<mutex> lck(m_mutex);

	m_vector.push_back(move(val));
}

template<typename T>
inline bool ThreadSafeVector<T>::empty() const noexcept
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.empty();
}

template<typename T>
inline int ThreadSafeVector<T>::size() const noexcept
{
	lock_guard<mutex> lck(m_mutex);

	return static_cast<int>(m_vector.size());
}

template<typename T>
inline typename vector<T>::iterator ThreadSafeVector<T>::end()
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.end();
}

template<typename T>
inline typename vector<T>::const_iterator ThreadSafeVector<T>::end() const
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.end();
}

template<typename T>
inline typename vector<T>::iterator ThreadSafeVector<T>::begin()
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.begin();
}

template<typename T>
inline typename vector<T>::const_iterator ThreadSafeVector<T>::begin() const
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.begin();
}

template<typename T>
inline typename vector<T>::iterator ThreadSafeVector<T>::erase(typename vector<T>::const_iterator position)
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.erase(position);
}

template<typename T>
inline ThreadSafeVector<T>::ThreadSafeVector(int size)
	: m_mutex{},
	m_vector{ size }
{}

template<typename T>
inline ThreadSafeVector<T>::ThreadSafeVector()
	: m_mutex{},
	m_vector{}
{}

template<typename T>
inline ThreadSafeVector<T>::ThreadSafeVector(const ThreadSafeVector& tsv)
	: m_mutex{},
	m_vector{ tsv.m_vector }
{}

template<typename T>
inline ThreadSafeVector<T>& ThreadSafeVector<T>::operator=(const ThreadSafeVector<T>& tsv)
{
	lock_guard<mutex> lck(m_mutex);

	m_vector = tsv.m_vector;

	return *this;
}

template<typename T>
inline ThreadSafeVector<T>::ThreadSafeVector(ThreadSafeVector<T>&& tsv)
	: m_mutex{ move(tsv.m_mutex) },
	m_vector{ move(tsv.m_vector) }
{}

template<typename T>
inline ThreadSafeVector<T>& ThreadSafeVector<T>::operator=(ThreadSafeVector<T>&& tsv)
{
	lock_guard<mutex> lck(m_mutex);

	m_mutex = move(tsv.m_mutex);
	m_vector = move(tsv.m_vector);

	return *this;
}
