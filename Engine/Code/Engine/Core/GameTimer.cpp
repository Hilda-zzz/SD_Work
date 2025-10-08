#include "GameTimer.hpp"
#include "Clock.hpp"
extern Clock* g_systemClock;
GameTimer::GameTimer(float period, const Clock* clock):m_period(period)
{
	if (clock == nullptr)
	{
		m_clock= g_systemClock;
	}
	else
	{
		m_clock = clock;
	}
}

void GameTimer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void GameTimer::Stop()
{
	m_startTime = -1.1f;
}

double GameTimer::GetElapsedTime() const
{
	if (IsStopped())
	{
		return 0.f;		//?
	}
	else
	{
		return m_clock->GetTotalSeconds() - m_startTime;
	}
}

float GameTimer::GetElapsedFraction() const
{
	return (float)GetElapsedTime() / (float)m_period;
}

bool GameTimer::IsStopped() const
{
	if (m_startTime <= -1.f)
	{
		return true;
	}
	return false;
}

bool GameTimer::HasPeriodElapsed() const
{
	if (GetElapsedTime() > m_period&&!IsStopped())
	{
		return true;
	}
	return false;
}

bool GameTimer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed() &&!IsStopped())
	{
		m_startTime += m_period;
		return true;
	}
	return false;
}
