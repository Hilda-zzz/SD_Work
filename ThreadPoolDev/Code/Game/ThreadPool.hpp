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
#include <unordered_set>
#include "TaskInfo.hpp"

class ThreadPool
{
public:
	ThreadPool(size_t threadsCount);

	ThreadPool(ThreadPool const&) = delete;
	ThreadPool& operator=(ThreadPool const&) = delete;

	~ThreadPool();

	template<class F, class...Args>
	auto Enqueue(F&& f, Args&&...args)
		-> std::future<typename std::invoke_result<F,Args...>::type>;

	template<class F,class...Args>
	auto EqueueWithPriority(TaskPriority priority, F&& f, Args&&...args)
		-> std::future<typename std::invoke_result<F, Args...>::type>;

	template<class F, class... Args>
	auto EnqueueWithInfo(std::string taskId, std::string description,
		TaskPriority priority, F&& f, Args&&... args)
		-> std::future<typename std::invoke_result<F, Args...>::type>;

	template<class F, class... Args>
	auto CreateTaskFunction(std::shared_ptr<std::promise<typename std::invoke_result<F, Args...>::type>> promise,
		F&& f, Args&&... args)
		->std::function<void()>;

	bool IsStopped();

	size_t GetThreadCount() const;
	size_t GetActiveThreadCount() const;
	size_t GetWaitingThreadCount() const;

	size_t GetTasksCount();
	size_t GetCompletedTaskCount() const;
	size_t GetFailedTaskCount() const;

	void Resize(size_t newSize);
	void Pause();
	void Resume();
	void WaitForTasks();
	void ClearTasks();

private:
	void WorkerThread(size_t id);

private:
	std::vector<std::thread> m_workers;

	//std::queue<std::function<void()>> m_tasks;
	std::priority_queue<TaskInfo> m_tasks;

	std::unordered_set<size_t> m_threadsToStop;

	std::mutex m_queueMutex;
	std::condition_variable m_condition;
	std::condition_variable m_waitCondition;

	std::atomic<bool> m_stop{ false };
	std::atomic<bool> m_paused{ false };

	std::atomic<size_t> m_completedTasks{ 0 }; // safe in the thread even without mutex
	std::atomic<size_t> m_failedTasks{ 0 };
	std::atomic<size_t> m_activeThreads{ 0 };
};

template<class F, class ...Args>
inline auto ThreadPool::Enqueue(F&& f, Args && ...args) 
-> std::future<typename std::invoke_result<F, Args ...>::type>
{
	return enqueueWithPriority(TaskPriority::MEDIUM,
		std::forward<F>(f),
		std::forward<Args>(args)...);
}

template<class F, class ...Args>
inline auto ThreadPool::EqueueWithPriority(TaskPriority priority, F&& f, Args && ...args) 
-> std::future<typename std::invoke_result<F, Args ...>::type>
{
	return EnqueueWithInfo("", "", priority,
		std::forward<F>(f), std::forward<Args>(args)...);
}

template<class F, class ...Args>
inline auto ThreadPool::EnqueueWithInfo(std::string taskId, std::string description, TaskPriority priority, F&& f, Args && ...args) 
-> std::future<typename std::invoke_result<F, Args ...>::type>
{
	using return_type = typename std::invoke_result<F, Args...>::type;

	// out of lock
	auto promise = std::make_shared<std::promise<return_type>>();
	std::future<return_type> result = promise->get_future();

	auto taskFunction = createTaskFunction(promise, std::forward<F>(f), std::forward<Args>(args)...);

	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		if (m_stop)
		{
			throw std::runtime_error("enqueue on stopped ThreadPool");
		}

		// logTaskSubmission(taskId, description, priority);

		m_tasks.emplace(
			std::move(taskFunction),
			priority,
			taskId,
			description
		);

		if (!taskId.empty()) {
			taskIdMap[taskId] = tasks.size();
		}

// 		metrics.totalTasks++;
// 		metrics.updateQueueSize(tasks.size());
	}
	condition.notify_one();
	return result;
}

template<class F, class ...Args>
inline auto ThreadPool::CreateTaskFunction(std::shared_ptr<std::promise<typename std::invoke_result<F, Args...>::type>> promise, F&& f, Args && ...args) 
-> std::function<void()>
{
	using return_type = typename std::invoke_result<F, Args...>::type;

	return [promise,
		f = std::forward<F>(f),
		args = std::make_tuple(std::forward<Args>(args)...)]() mutable
		{
			try
			{
				if constexpr (std:is_void_v<return_type>)
				{
					std::apply(f, args);
					promise->set_value();
				}
				else
				{
					promise->set_value(std::apply(f, args));
				}
			}
			catch (std::exception const&)
			{
				promise->set_exception(std::current_exception());
				throw;
			}
			catch (...)
			{
				promise->set_exception(std::current_exception());
				throw;
			}
		}
}
