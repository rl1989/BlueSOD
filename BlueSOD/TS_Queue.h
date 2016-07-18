#pragma once
#include <mutex>
#include <queue>
#include <utility>

template<typename T>
class ThreadSafeQueue
{
private:
	std::mutex m_mutex{};
	std::queue<T> m_queue{};
public:
	ThreadSafeQueue(const ThreadSafeQueue& tsq);
	ThreadSafeQueue();
	void push(T&& t);
	void push(const T& t);
	void pop();
	void clear();

	T& front();
	int size();
	bool empty();
	
};