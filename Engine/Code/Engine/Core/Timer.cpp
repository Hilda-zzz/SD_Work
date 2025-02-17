#include "Timer.hpp"
#include "Clock.hpp"
extern Clock* g_systemClock;
Timer::Timer(float period, const Clock* clock):m_period(period)
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

void Timer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void Timer::Stop()
{
	m_startTime = 0.0;
}

double Timer::GetElapsedTime() const
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

float Timer::GetElapsedFraction() const
{
	return (float)GetElapsedTime() / (float)m_period;
}

bool Timer::IsStopped() const
{
	if (m_startTime == 0.0)
	{
		return true;
	}
	return false;
}

bool Timer::HasPeriodElapsed() const
{
	if (GetElapsedTime() > m_period&&!IsStopped())
	{
		return true;
	}
	return false;
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed() &&!IsStopped())
	{
		m_startTime += m_period;
		return true;
	}
	return false;
}
