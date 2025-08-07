#pragma once
#include <string>

class DayTimeSystem 
{
public:
	DayTimeSystem(float realSecPerDay);
	void Update(float deltaTime);

	void SetPeriod(float realSecondsPerDay);

	void StartTime() { m_isRunning = true; }
	void PauseTime() { m_isRunning = false; }
	bool IsRunning() const { return m_isRunning; }

	int GetCurrentDay() const { return m_cachedDay; }
	int GetCurrentHour() const { return m_cachedHour; }
	int GetCurrentMinute() const { return m_cachedMinute; }

	float GetDayProgress() const; // (0.0 - 1.0)
	float GetTotalGameTimeSeconds() const { return m_totalGameTimeSeconds; }

	std::string GetFormatDay() const;
	std::string GetFormattedTime() const; 

	bool IsNewDay(); 	// 检查是否是新的一天

private:
	void UpdateTimeCache(); 

	void InvalidateCache();


private:
	float m_realSecondsPerGameDay = 1200.0f;
	float m_totalGameTimeSeconds = 0.0f;
	bool m_isRunning = false;

	int m_prevDay = 0;
	int m_cachedDay = 0;
	int m_cachedHour = 0;
	int m_cachedMinute = 0;
	float m_lastUpdateTime = 0.0f;
};