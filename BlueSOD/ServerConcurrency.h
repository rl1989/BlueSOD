#pragma once
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <memory>

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
	ThreadSafe();
	~ThreadSafe();

	ThreadSafe<T>& operator=(const T& o);
	ThreadSafe<T>& operator=(T&& o);
	ThreadSafe<T>& operator+(const T& o);
	ThreadSafe<T>& operator+=(const T& o);
	ThreadSafe<T>& operator++();
	operator T() const;

	void ChangeObject(const T& o);
	void ChangeObject(T& o);
	void ChangeObject(T&& o);

	const T& RetrieveObject();

	bool TryLock();
	void Lock();
	void LockChange(const T& o);
	void LockChange(T&& o);
	void Unlock();

	T&& MoveObject();
};