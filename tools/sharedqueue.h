#pragma once

#include <queue>
#include <boost/thread.hpp>

template <typename T>
class SynchronisedQueue
{
public:

	SynchronisedQueue()
	{
	}
	void Enqueue(const T& data)
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
        m_queue.push(data);
	}

	bool TryDequeue(T& result)
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
        if (!m_queue.empty())
        {
            result = m_queue.front();
            m_queue.pop();
            return true;
        }
		return false;;
	}

	int Size()
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
		return m_queue.size();
	}

private:

	std::queue<T> m_queue;              // Use STL queue to store data
	boost::mutex m_mutex;               // The mutex to synchronise on
};
