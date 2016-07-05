#pragma once
#include <mutex>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
private:
	std::mutex m_mutex{};
	std::queue<T> m_queue{};
public:
	ThreadSafeQueue(const ThreadSafeQueue& tsq);
	void push(T&& t);
	void pop();

	T&& front();
	T&& back();
	int size();
	bool empty();
};