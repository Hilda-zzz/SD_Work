#include "JobSystem.hpp"
#include "JobWorkerThread.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "../Core/StringUtils.hpp"
#include "../Core/ErrorWarningAssert.hpp"
#include "Engine/JobSystem/Job.hpp"

extern DevConsole* g_theDevConsole;

JobSystem::JobSystem()
{
}

JobSystem::~JobSystem()
{
}

void JobSystem::Startup()
{
	// determine by hardware cores
	int numCores = (int)std::thread::hardware_concurrency();
	m_numWorkerThreads = numCores - 1;
	//m_numWorkerThreads = 20;
	if (m_numWorkerThreads < 1)
		m_numWorkerThreads = 1;

	// create worker thread
	m_workerThreads.reserve(m_numWorkerThreads);
	for (int i = 0; i < m_numWorkerThreads; ++i)
	{
		JobWorkerThread* worker = new JobWorkerThread(i, this);
		m_workerThreads.push_back(worker);
		worker->Start();        
		
		// #TODO: what start should do? 
		//Re: Each worker creates its own thread
	}
}

void JobSystem::Shutdown()
{
	if (m_workerThreads.empty())
	{
		return; // Already shut down
	}
	m_isShuttingDown = true;

	// Wake up all waiting workers
	m_workAvailableCondition.notify_all();   

	// Wait for all workers to finish and delete them
	for (JobWorkerThread* worker : m_workerThreads)
	{
		worker->Join();
		delete worker;
	}
	m_workerThreads.clear();

	// Clean up any remaining jobs
	{
		std::scoped_lock lock(m_queueMutex);  

		// Warning if there are still pending jobs
		if (!m_pendingJobs.empty())
		{
			g_theDevConsole->AddLine(DevConsole::UNKNOWN,
				Stringf("JobSystem shutdown with %d pending jobs still queued",
					(int)m_pendingJobs.size()));
		}

		m_pendingJobs.clear();
		m_processingJobs.clear();
		m_completedJobs.clear();
	}

	g_theDevConsole->AddLine(DevConsole::TIPS, "JobSystem shut down");
}

void JobSystem::BeginFrame()
{
}

void JobSystem::EndFrame()
{
}

void JobSystem::QueueJob(Job* job)
{
	GUARANTEE_OR_DIE(job != nullptr, "Cannot queue null job");

	{
		std::scoped_lock lock(m_queueMutex);
		m_pendingJobs.push_back(job);
	}

	// Wake up one waiting worker
	m_workAvailableCondition.notify_one();
}

Job* JobSystem::RetrieveCompletedJob()
{
	std::scoped_lock lock(m_queueMutex);

	if (m_completedJobs.empty())
	{
		return nullptr;
	}

	Job* completedJob = m_completedJobs.front();
	m_completedJobs.pop_front();
	return completedJob;
}

void JobSystem::RetrieveCompletedJobs(std::vector<Job*>& out_completedJobs)
{
	std::scoped_lock lock(m_queueMutex);

	while (!m_completedJobs.empty())
	{
		Job* completedJob = m_completedJobs.front();
		m_completedJobs.pop_front();
		out_completedJobs.push_back(completedJob);
	}
}

int JobSystem::GetNumCompletedJobs() const
{
	std::scoped_lock lock(m_queueMutex);   // #TODO: what's the aim for that
	return (int)m_completedJobs.size();  
}

Job* JobSystem::ClaimPendingJob()
{
	//std::scoped_lock lock(m_queueMutex);  // double lock!!

	if (m_pendingJobs.empty())
	{
		return nullptr;
	}

	// Claim the job
	Job* job = m_pendingJobs.front();
	m_pendingJobs.pop_front();

	// Move to processing queue
	m_processingJobs.push_back(job);

	return job;
}

void JobSystem::MoveJobToCompleted(Job* job)
{
	std::scoped_lock lock(m_queueMutex);

	// Remove from processing queue
	for (auto iter = m_processingJobs.begin(); iter != m_processingJobs.end(); ++iter)
	{
		if (*iter == job)
		{
			m_processingJobs.erase(iter);
			break;
		}
	}

	// Add to completed queue
	m_completedJobs.push_back(job);
}

bool JobSystem::HasPendingJobs()
{
	return !m_pendingJobs.empty();
}

bool JobSystem::CancelPendingJob(Job* job)
{
	std::scoped_lock lock(m_queueMutex);
	for (auto iter = m_pendingJobs.begin(); iter != m_pendingJobs.end(); ++iter)
	{
		if (*iter == job)
		{
			m_pendingJobs.erase(iter);
			delete job;
			return true;
		}
	}

	return false;
}

void JobSystem::CancelAllPendingJobs()
{
	std::scoped_lock lock(m_queueMutex);

	for (Job* job : m_pendingJobs)
	{
		delete job;  
	}

	m_pendingJobs.clear();
}

//void JobSystem::NotifyWorkersOfNewWork()
//{
//	m_workAvailableCondition.notify_all(); // #TODO: what's the aim for that?
//}
