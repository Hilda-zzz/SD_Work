#include "ThreadPool.hpp"
#include <iostream>
#include <map>

ThreadPool::ThreadPool(size_t threadsCount)
{
	std::cout << "Pool Construction Start:" << threadsCount << std::endl;

	for (size_t i = 0; i < threadsCount; i++)
	{
		m_workers.emplace_back(
			[this,i] {this->WorkerThread(i); }
		);
	}

	std::cout << "Pool Construction Completed." << std::endl;
}

ThreadPool::~ThreadPool()
{
	std::cout << "Pool Deconstruction Start." << std::endl;

	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_stop = true;
	}

	m_condition.notify_all();

	for (std::thread& worker : m_workers)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
	std::cout << "Pool has been destroyed." << std::endl;
}

bool ThreadPool::IsStopped()
{
	return m_stop;
}

size_t ThreadPool::GetTasksCount()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	return m_tasks.size();
}

size_t ThreadPool::GetThreadCount() const
{
	return m_workers.size();
}

size_t ThreadPool::GetCompletedTaskCount() const
{
	return m_completedTasks;
}

size_t ThreadPool::GetActiveThreadCount() const
{
	return m_activeThreads;
}

size_t ThreadPool::GetWaitingThreadCount() const
{
	size_t totalThreads = GetThreadCount();
	size_t activeCount = GetActiveThreadCount();
	return totalThreads - activeCount;
}

size_t ThreadPool::GetFailedTaskCount() const
{
	return m_failedTasks;
}

void ThreadPool::Resize(size_t newSize)
{
	std::unique_lock<std::mutex> lock(m_queueMutex);

	if (m_stop)
	{
		throw std::runtime_error("resize on stopped ThreadPool");
	}

	size_t oldSize = m_workers.size();
	std::cout << "Adjust workers count: " << oldSize << "-> " << newSize << std::endl;

	if (newSize > oldSize)
	{
		m_workers.reserve(newSize);
		for (size_t i = oldSize; i < newSize; ++i) 
		{
			m_workers.emplace_back([this, i] { this->WorkerThread(i); });
		}
		std::cout << "Increase " << (newSize - oldSize) << " thread count" << std::endl;
	}
	else if (newSize < oldSize)
	{
		m_threadsToStop.clear();

		for (size_t i = newSize; i < oldSize; ++i)
		{
			m_threadsToStop.insert(i);
		}

		lock.unlock();
		m_condition.notify_all();

		for (size_t i = newSize; i < oldSize; ++i)
		{
			if (m_workers[i].joinable())
			{
				m_workers[i].join();
			}
		}

		lock.lock();
		m_workers.resize(newSize);
		std::cout << "Decline " << (oldSize - newSize) << " thread count" << std::endl;
	}
}

void ThreadPool::Pause()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_paused = true;
	std::cout << "Thread Pool is Stopped!"<<std::endl;
}

void ThreadPool::Resume()
{
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_paused = false;
		std::cout << "Thread Pool is Resumed!" << std::endl;
	}
	// release lock and then notify all
	m_condition.notify_all();
}

void ThreadPool::WaitForTasks()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	std::cout << "Waiting for all tasks to complete..." << std::endl;
	m_waitCondition.wait(lock, [this] {
		return (m_tasks.empty() && m_activeThreads == 0) || m_stop;
	});
	std::cout << "All tasks completed" << std::endl;
}

void ThreadPool::ClearTasks()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	size_t taskCount = m_tasks.size();

	std::queue<std::function<void()>> emptyQueue;
	// emptyQueue will destroyed itself
	std::swap(m_tasks, emptyQueue);
	// O(1) way to clear the tasks
	std::cout << "Clear task queue: " << taskCount << " tasks have been cleared" << std::endl;
}

void ThreadPool::WorkerThread(size_t id)
{
	while (true)
	{
		std::function<void()> task;

		// 1. Get Task
		// a scope block
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);

			// wait until getting a task or thread pool is stopped
// 			std::cout << "Work Thread: " << std::this_thread::get_id() <<
// 				" Waiting a task or stop signal..." << std::endl;
			m_condition.wait(lock, [this,id] {
				return this->m_stop ||
					(!this->m_paused && !this->m_tasks.empty()) ||
					(this->m_threadsToStop.find(id) != this->m_threadsToStop.end());
			});

			// thread pool stop
			if (this->m_stop)
			{
				return;
			}
			// the thread should be stopped
			if (this->m_threadsToStop.find(id) != this->m_threadsToStop.end())
			{
				this->m_threadsToStop.erase(id);
				return;
			}
			// get task
			if (!this->m_paused && !this->m_tasks.empty())
			{
				task = std::move(this->m_tasks.front());
				this->m_tasks.pop();
			}

		} // lock will release at the end of block
		
		// Execute task out of the lock in case of blocking other threads
		if (task)
		{
			++m_activeThreads;
			try {
				task();
				++m_completedTasks;
			}
			catch(std::exception const& e){
				std::cout << "Exception when executing the task: " 
					<< e.what() << std::endl;
				++m_failedTasks;
			}
			catch (...) {
				std::cout << "Unknown exception when executing the task." << std::endl;
				++m_failedTasks;
			}
			--m_activeThreads;
			m_waitCondition.notify_all();
		}
	}
}
