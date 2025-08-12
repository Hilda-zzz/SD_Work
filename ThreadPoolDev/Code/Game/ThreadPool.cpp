#include "ThreadPool.hpp"
#include <iostream>

ThreadPool::ThreadPool(size_t threadsCount)
{
	std::cout << "Pool Construction Start:" << threadsCount << std::endl;

	for (size_t i = 0; i < threadsCount; i++)
	{
		m_workers.emplace_back(
			[this] {this->WorkerThread(); }
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

int ThreadPool::GetTasksCount()
{
	return (int)m_tasks.size();
}

void ThreadPool::WorkerThread()
{
	while (true)
	{
		std::function<void()> task;

		// a scope block
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);

			// wait until getting a task or thread pool is stopped
// 			std::cout << "Work Thread: " << std::this_thread::get_id() <<
// 				" Waiting a task or stop signal..." << std::endl;
			m_condition.wait(lock, [this] {
				return this->m_stop || !this->m_tasks.empty();
			});

			if (this->m_stop && this->m_tasks.empty())
			{
				return;
			}

			task = std::move(this->m_tasks.front());
			this->m_tasks.pop();

		} // lock will release at the end of block
		
		// Execute task out of the lock in case of blocking other threads
		if (task)
		{
			try {
				task();
			}
			catch(std::exception const& e){
				std::cout << "Exception when executing the task: " 
					<< e.what() << std::endl;
			}
			catch (...) {
				std::cout << "Unknown exception when executing the task." << std::endl;
			}
		}
	}
}
