#pragma once
#include <mutex>
#include <thread>
#include <shared_mutex>

using std::mutex;
using std::shared_mutex;
using std::lock_guard;
using std::thread;

template<typename T>
class ThreadSafe
{
private:
	T m_object;
	mutex m_mutex;
public:
	ThreadSafe(const T& o);
	ThreadSafe(T& o);
	ThreadSafe(T&& o);
	~ThreadSafe();

	void ChangeObject(const T& o);
	void ChangeObject(T& o);
	void ChangeObject(T&& o);

	const T& RetrieveObject();

	T&& MoveObject();
};