#pragma once
#include <functional>
#include <chrono>

enum class TaskPriority {
	LOW,
	MEDIUM,
	HIGH,
	CRITICAL
};

enum class TaskStatus
{
	WAITING,
	RUNNING,
	COMPLETED,
	FAILED,
	CANCELED
};

struct TaskInfo
{
	std::function<void()> m_task;
	std::string m_taskID;
	std::string m_taskDescription;
	TaskPriority m_priority;
	std::chrono::steady_clock::time_point m_submitTime;
	std::atomic<TaskStatus> status{ TaskStatus::WAITING };
	std::string errorMessage;

	TaskInfo(std::function<void()> t,
		TaskPriority priority=TaskPriority::MEDIUM,
		std::string id="",
		std::string desc="")
		: m_task(std::move(t))
		, m_taskID(std::move(id))
		, m_taskDescription(std::move(desc))
		, m_priority(priority)
		, m_submitTime(std::chrono::steady_clock::now())
	{}

	bool operator< (const TaskInfo& other) const
	{
		if (m_priority != other.m_priority)
		{
			return m_priority < other.m_priority;
		}
		return m_submitTime > other.m_submitTime;
	}
	
};