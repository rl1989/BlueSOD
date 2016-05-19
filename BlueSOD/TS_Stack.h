#pragma once
#include <stack>
#include <shared_mutex>

using std::stack;
using std::shared_mutex;
using std::lock_guard;

template<typename T>
class TS_Stack :
	public stack<T>
{
private:
	shared_mutex m;
public:
	TS_Stack()
		: m{},
		base{}
	{}

	void push(const T& e)
	{
		lock_guard<shared_mutex> lck(m);

		static_cast<stack<T>*>(this)->push(e);
	}
	void push(T&& e)
	{
		lock_guard<shared_mutex> lck(m);

		static_cast<stack<T>*>(this)->push(e);
	}
	void pop()
	{
		lock_guard<shared_mutex> lck(m);

		static_cast<stack<T>*>(this)->pop();
	}
	int& top()
	{
		lock_guard<shared_mutex> lck(m);

		return static_cast<stack<T>*>(this)->top();
	}
	int size()
	{
		lock_guard<shared_mutex> lck(m);

		return static_cast<stack<T>*>(this)->size();
	}
};

