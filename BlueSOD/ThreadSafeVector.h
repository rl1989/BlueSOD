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
	explicit ThreadSafeVector(int size);
	ThreadSafeVector();
	ThreadSafeVector(const ThreadSafeVector& tsv);
	ThreadSafeVector& operator=(const ThreadSafeVector<T>& tsv);
	ThreadSafeVector(ThreadSafeVector<T>&& tsv);
	ThreadSafeVector& operator=(ThreadSafeVector<T>&& tsv);

	void reserve(int space);
	T& elem(int i);
	T& operator[](int i);
	void clear();
	void push_back(T&& val);
	bool empty() const noexcept;
	int size() const noexcept;

	typename std::vector<T>::iterator end();
	typename std::vector<T>::const_iterator end() const;
	typename std::vector<T>::iterator begin();
	typename std::vector<T>::const_iterator begin() const;
	typename std::vector<T>::iterator erase(typename std::vector<T>::const_iterator position);
};