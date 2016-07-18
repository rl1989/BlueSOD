#pragma once
#include <list>
#include <mutex>

template<typename T>
class ThreadSafeList
{
private:
	std::mutex m_mutex{};
	std::list<T> m_list{};
public:
	void push_back(const T& elem);
	void push_back(T&& elem);
	void erase(typename std::list::iterator<T> elem);
	int size();
	bool empty();
	typename std::list<T>::iterator begin();
	typename std::list<T>::iterator end();
	void clear();
	void lock();
	void unlock();
};