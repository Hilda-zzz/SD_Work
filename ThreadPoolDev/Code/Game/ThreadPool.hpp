#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

class ThreadPool
{
public:
	ThreadPool(size_t threadsCount);

	ThreadPool(ThreadPool const&) = delete;
	ThreadPool& operator=(ThreadPool const&) = delete;

	~ThreadPool();

private:
	std::vector<std::thread> m_workers;

	std::queue<std::function<void()>> m_tasks;

	std::mutex m_queueMutex;
	std::condition_variable m_condition;

	std::atomic<bool> stop{ false };
};