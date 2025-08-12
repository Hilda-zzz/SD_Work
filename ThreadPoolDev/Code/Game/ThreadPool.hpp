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

	template<class F, class...Args>
	auto Enqueue(F&& f, Args&&...args)
		-> std::future<typename std::invoke_result<F,Args...>::type>;

	bool IsStopped();
	int GetTasksCount();

private:
	void WorkerThread();

private:
	std::vector<std::thread> m_workers;

	std::queue<std::function<void()>> m_tasks;

	std::mutex m_queueMutex;
	std::condition_variable m_condition;

	std::atomic<bool> m_stop{ false };
};

/**
 * @brief Enqueues a callable object with its arguments to be executed by the thread pool
 * @tparam F Type of the callable object (function, lambda, functor, etc.)
 * @tparam Args Types of the arguments to be passed to the callable
 * @param f The callable object to be executed (forwarded as universal reference)
 * @param args Arguments to be passed to the callable (forwarded as universal references)
 * @return std::future containing the result of the callable execution
 */
template<class F, class ...Args>
inline auto ThreadPool::Enqueue(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F, Args ...>::type>
{
	// func return type
	using return_type = typename std::invoke_result<F, Args...>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>( // a shared ptr  // is that safe?
		std::bind(std::forward<F>(f),std::forward<Args>(args)...)  //bind f and args
	);

	std::future<return_type> result = task->get_future();

	{
		std::unique_lock<std::mutex> lock(m_queueMutex);

		if (m_stop)
		{
			throw std::runtime_error("enqueue on stopped ThreadPool");
		}

		m_tasks.emplace([task]() {(*task)(); });
	}
	
	m_condition.notify_one();
	return result;
}
