#include "DayTimeSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

extern EventSystem* g_theEventSystem;

DayTimeSystem::DayTimeSystem(float realSecPerDay)
	:m_realSecondsPerGameDay(realSecPerDay)
{
}

void DayTimeSystem::Update(float deltaTime)
{
	if (!m_isRunning) return;

	m_totalGameTimeSeconds += deltaTime;

	if (m_totalGameTimeSeconds != m_lastUpdateTime) 
	{
		UpdateTimeCache();
		m_lastUpdateTime = m_totalGameTimeSeconds;

		if (IsNewDay())
		{
			g_theEventSystem->FireEvent("StartNewDay");
		}
	}
}

void DayTimeSystem::SetPeriod(float realSecondsPerDay)
{
	m_realSecondsPerGameDay = realSecondsPerDay;
	InvalidateCache();
}


float DayTimeSystem::GetDayProgress() const
{
	float secondsInCurrentDay = (float)fmod(m_totalGameTimeSeconds, m_realSecondsPerGameDay);
	return secondsInCurrentDay / m_realSecondsPerGameDay;
}

std::string DayTimeSystem::GetFormatDay() const
{
	std::string day = std::to_string(m_cachedDay + 1);
	return "Day " + day;
}

std::string DayTimeSystem::GetFormattedTime() const
{
	std::string hour = (m_cachedHour < 10 ? "0" : "") + std::to_string(m_cachedHour);
	std::string minute = (m_cachedMinute < 10 ? "0" : "") + std::to_string(m_cachedMinute);

	return hour + ":" + minute;
}

bool DayTimeSystem::IsNewDay()
{
	if (m_cachedDay != m_prevDay)
	{
		m_prevDay = m_cachedDay;
		return true;
	}
	return false;
}

void DayTimeSystem::UpdateTimeCache()
{
	// day
	m_cachedDay = static_cast<int>(m_totalGameTimeSeconds / m_realSecondsPerGameDay);

	// today sec
	float secondsInCurrentDay =(float) fmod(m_totalGameTimeSeconds, m_realSecondsPerGameDay);

	// today hour
	float hoursInDay = (secondsInCurrentDay / m_realSecondsPerGameDay) * 24.0f + 6.0f;

	if (hoursInDay >= 24.0f)
	{
		hoursInDay -= 24.0f;
	}

	m_cachedHour = static_cast<int>(hoursInDay);
	float minutesInHour = (hoursInDay - m_cachedHour) * 60.0f;
	m_cachedMinute = static_cast<int>(minutesInHour);
}

void DayTimeSystem::InvalidateCache()
{
	UpdateTimeCache();
}