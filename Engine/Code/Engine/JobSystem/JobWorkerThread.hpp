#pragma once
#include <thread>

class JobSystem;

class JobWorkerThread
{
public:
	JobWorkerThread(int threadID, JobSystem* jobSystem);
	~JobWorkerThread();

	void Start();
	void Join();

	int GetThreadID() const { return m_threadID; }

private:
	void ThreadMain(); // Entry point for the thread

private:
	int m_threadID = -1;
	JobSystem* m_jobSystem = nullptr;
	std::thread* m_thread = nullptr;    // #TODO: pointer or object??
};