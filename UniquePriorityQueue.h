#pragma once
#include <set>
#include <queue>

template<typename T>
class UniquePriorityQueue
{
private:	
	std::priority_queue<T> m_queue;
	std::set<T> m_set;

public:
	bool push(const T& t)
	{
		if (m_set.insert(t).second)
		{
			m_queue.push(t);
			return true;
		}
		return false;
	}
	T top() {
		return m_queue.top();
	}
	T pop() {
		T element = top();		
		m_queue.pop();
		return element;
	}
	bool empty() {
		return m_queue.empty();
	}
};