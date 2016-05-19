#pragma once
#include <stack>
#include <shared_mutex>

using std::stack;
using std::shared_mutex;
using std::lock_guard;

/*
	A thread safe stack that allows the ServerManager to communicate with the thread that authenticates
	users. The producer (ServerManager) will push the object into the TS_Stack object and the 
	consumer (the other thread) will pop objects to retrieve them.

	The consumer will wait in a loop checking Size() to see if there are objects available.
*/
template<typename T>
class TS_Stack :
	public stack<T>
{
private:
	shared_mutex m;
public:
	TS_Stack()
		: m{},
		stack<T>{}
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
	T& top()
	{
		lock_guard<shared_mutex> lck(m);

		return static_cast<stack<T>*>(this)->top();
	}
	int size()
	{
		lock_guard<shared_mutex> lck(m);

		return static_cast<stack<T>*>(this)->size();
	}
	bool empty()
	{
		lock_guard<shared_mutex> lck(m);

		return static_cast<stack<T>*>(this)->empty();
	}
};

