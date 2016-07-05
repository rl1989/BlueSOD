#include "ThreadSafeVector.h"

using std::vector;
using std::mutex;
using std::lock_guard;
using std::move;

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
inline vector<T>::iterator ThreadSafeVector<T>::end()
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.end();
}

template<typename T>
inline vector<T>::const_iterator ThreadSafeVector<T>::end() const
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.end();
}

template<typename T>
inline vector<T>::iterator ThreadSafeVector<T>::begin()
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.begin();
}

template<typename T>
inline vector<T>::const_iterator ThreadSafeVector<T>::begin() const
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.begin();
}

template<typename T>
inline vector<T>::iterator ThreadSafeVector<T>::erase(vector<T>::const_iterator position)
{
	lock_guard<mutex> lck(m_mutex);

	return m_vector.erase(position);
}
