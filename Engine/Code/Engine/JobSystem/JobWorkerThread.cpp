#include "JobWorkerThread.hpp"
#include "JobSystem.hpp"
#include "Job.hpp"

JobWorkerThread::JobWorkerThread(int threadID, JobSystem* jobSystem)
	: m_threadID(threadID)
	, m_jobSystem(jobSystem)
	, m_thread(nullptr)
{
}

JobWorkerThread::~JobWorkerThread()
{
	if (m_thread != nullptr)
	{
		delete m_thread;
		m_thread = nullptr;
	}
}

void JobWorkerThread::Start()
{
	m_thread = new std::thread(&JobWorkerThread::ThreadMain, this); 
}

void JobWorkerThread::Join()
{
	if (m_thread != nullptr && m_thread->joinable())
	{
		m_thread->join();
	}
}

void JobWorkerThread::ThreadMain()
{
	while (!m_jobSystem->IsShuttingDown())
	{
		Job* job = nullptr;

		// Try to claim a job
		{
			std::unique_lock<std::mutex> lock(m_jobSystem->GetQueueMutex());

			// Wait until work is available or shutdown is requested
			m_jobSystem->GetWorkAvailableCondition().wait(lock, [this]()
				{
					return m_jobSystem->IsShuttingDown() ||
						(m_jobSystem->HasPendingJobs());
					// when return true, the thread quit the sleeping and process next code in while
				}
			);

			if (m_jobSystem->IsShuttingDown())
			{
				break;
			}

			// Try to claim a job (without holding the lock for long)
			job = m_jobSystem->ClaimPendingJob();
		}

		// Execute the job (outside of mutex lock)
		if (job != nullptr)
		{
			job->Execute();

			// Move to completed queue
			m_jobSystem->MoveJobToCompleted(job);

			std::this_thread::yield();
		}
		else
		{
			// No job available, yield CPU time
			std::this_thread::yield();
		}
	}
}
