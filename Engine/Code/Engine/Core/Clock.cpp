#include "Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Time.hpp"
#include <algorithm>	

extern Clock* g_systemClock;
Clock::Clock()
{
	m_parent = &GetSystemClock();
	m_children.reserve(5);
	if (m_parent)
	{
		m_parent->AddChild(this);
	}
	
}

Clock::Clock(Clock& parent)
{
	m_parent = &parent;
	if (m_parent)
	{
		m_parent->AddChild(this);
	}
}

Clock::~Clock()
{
	for (Clock* child : m_children)
	{
		child->m_parent = nullptr;
		//RemoveChild(child);
	}
	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}
	
	m_parent = nullptr;
}

void Clock::Reset()
{
	m_lastUpdateTimeInSeconds = 0.0;
	m_totalSeconds = 0.0;
	m_deltaSeconds = 0.0;
	m_frameCount = 0;

	//?
	m_lastUpdateTimeInSeconds = GetCurrentTimeSeconds();
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Unpause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_stepSingleFrame = true;
	Unpause();
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

double Clock::GetTimeScale() const
{
	return m_timeScale;
}

double Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

double Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

int Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return *g_systemClock;
}

void Clock::TickSystemClock()
{
	g_systemClock->Tick();
}

void Clock::Tick()
{
	float deltaTime=GetClamped((float)(GetCurrentTimeSeconds()-m_lastUpdateTimeInSeconds), 0.f, m_maxDeltaSeconds);
	m_lastUpdateTimeInSeconds = GetCurrentTimeSeconds();
	Advance((double)deltaTime);
}

void Clock::Advance(double deltaTimeSeconds)
{
	if (m_isPaused)
	{
		deltaTimeSeconds = 0.0;
	}

	if (m_stepSingleFrame)
	{
		Pause();
		m_stepSingleFrame = false;
	}
	m_deltaSeconds = m_timeScale * deltaTimeSeconds;
	m_totalSeconds += m_deltaSeconds;
	m_frameCount ++;
	for (Clock* childClock : m_children)
	{
		childClock->Advance(m_deltaSeconds);
	}

}

void Clock::AddChild(Clock* childClock)
{
	if (childClock->m_parent == nullptr)
	{
		childClock->m_parent = this;
	}
	m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
	m_children.erase(
		std::remove(m_children.begin(), m_children.end(), childClock),
		m_children.end()
	);

	childClock->m_parent = nullptr;
}


