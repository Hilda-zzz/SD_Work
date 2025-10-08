#pragma once
#include <vector>
#include <condition_variable>
#include <deque>

class Job;
class JobWorkerThread;

class JobSystem
{
public:
	JobSystem();
	~JobSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	// Job submission
	void QueueJob(Job* job);

	// Job retrieval (called by main thread)
	Job* RetrieveCompletedJob();
	void RetrieveCompletedJobs(std::vector<Job*>& out_completedJobs);
	int GetNumCompletedJobs() const;

	// Thread-safe job queue access (called by worker threads)
	Job* ClaimPendingJob();
	void MoveJobToCompleted(Job* job);
	bool HasPendingJobs();

	bool CancelPendingJob(Job* job);
	void CancelAllPendingJobs();

	// Signal workers that new work is available
	//void NotifyWorkersOfNewWork();

	// Condition variable for workers to wait on
	std::condition_variable& GetWorkAvailableCondition() { return m_workAvailableCondition; }
	std::mutex& GetQueueMutex() { return m_queueMutex; }

	bool IsShuttingDown() const { return m_isShuttingDown; }

private:
	// Worker threads
	std::vector<JobWorkerThread*> m_workerThreads;
	int m_numWorkerThreads = 0;

	// Job queues
	std::deque<Job*> m_pendingJobs;      // Jobs waiting to be claimed
	std::deque<Job*> m_processingJobs;   // Jobs currently being executed
	std::deque<Job*> m_completedJobs;    // Jobs finished and ready for retrieval

	// Thread synchronization
	mutable std::mutex m_queueMutex;     // Protects all three job queues
	std::condition_variable m_workAvailableCondition; // Workers wait on this

	std::atomic<bool> m_isShuttingDown = false;

};