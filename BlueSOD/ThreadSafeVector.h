#pragma once
#include <vector>
#include <mutex>

template<typename T>
class ThreadSafeVector
{
private:
	std::vector<T> m_vector{};
	std::mutex m_mutex{};
public:
	ThreadSafeVector() = default;
	ThreadSafeVector(const ThreadSafeVector<T>& tsv);
	ThreadSafeVector& operator=(const ThreadSafeVector<T>& tsv);
	ThreadSafeVector(ThreadSafeVector<T>&& tsv);
	ThreadSafeVector& operator=(ThreadSafeVector<T>&& tsv);
	ThreadSafeVector(const T& t);
	ThreadSafeVector(T&& t);

	void push_back(T&& val);
	bool empty() const noexcept;
	int size() const noexcept;
	std::vector<T>::iterator end();
	std::vector<T>::const_iterator end() const;
	std::vector<T>::iterator begin();
	std::vector<T>::const_iterator begin() const;
	std::vector<T>::iterator erase(std::vector<T>::const_iterator position);
};