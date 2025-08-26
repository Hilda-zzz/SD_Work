#pragma once
#include <atomic>
#include <chrono>
#include <ostream>
#include <sstream>

struct ThreadPoolMetrics {
	std::atomic<size_t> m_totalTasks{ 0 };
	std::atomic<size_t> m_completedTasks{ 0 };
	std::atomic<size_t> m_failedTasks{ 0 };
	std::atomic<size_t> m_activeThreads{ 0 };
	std::atomic<size_t> m_peakThreads{ 0 };
	std::atomic<size_t> m_peakQueueSize{ 0 };
	std::chrono::steady_clock::time_point m_startTime;
	std::atomic<uint64_t> m_totalTaskTimeNs{ 0 };

	ThreadPoolMetrics() : m_startTime(std::chrono::steady_clock::now()) {}

	void UpdateQueueSize(size_t size) 
	{
		size_t currentPeak = m_peakQueueSize.load();
		while (size > currentPeak && !m_peakQueueSize.compare_exchange_weak(currentPeak, size)) {
			// repeat
		}
	}

	void UpdateActiveThreads(size_t count)
	{
		m_activeThreads.store(count);
		size_t currentPeak = m_peakThreads.load();
		while (count > currentPeak && !m_peakThreads.compare_exchange_weak(currentPeak, count)) {
			// repeat
		}
	}

	void AddTaskTime(uint64_t timeNs) 
	{
		m_totalTaskTimeNs.fetch_add(timeNs);
	}

	// ms
	double GetAverageTaskTime() const 
	{
		size_t completed = m_completedTasks.load();
		if (completed == 0) return 0.0;
		return static_cast<double>(m_totalTaskTimeNs.load()) / completed / 1000000.0;
	}

	double GetUptime() const 
	{
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration<double>(now - m_startTime).count();
	}

	double GetThroughput() const {
		double uptime = GetUptime();
		if (uptime <= 0.0) return 0.0;
		return static_cast<double>(m_completedTasks.load()) / uptime;
	}

	std::string GetReport() const {
		std::stringstream ss;
		ss << "Thread Pool Performance Report:" << std::endl;
		ss << " Uptime: " << GetUptime() << " seconds" << std::endl;
		ss << " Total Tasks: " << m_totalTasks.load() << std::endl;
		ss << " Completed Tasks: " << m_completedTasks.load() << std::endl;
		ss << " Failed Tasks: " << m_failedTasks.load() << std::endl;
		ss << " Active Threads: " << m_activeThreads.load() << std::endl;
		ss << " Peak Threads: " << m_peakThreads.load() << std::endl;
		ss << " Peak Queue Size: " << m_peakQueueSize.load() << std::endl;
		ss << " Average Task Time: " << GetAverageTaskTime() << " ms" << std::endl;
		ss << " Task Throughput: " << GetThroughput() << " tasks/sec" << std::endl;
		return ss.str();
	}
};