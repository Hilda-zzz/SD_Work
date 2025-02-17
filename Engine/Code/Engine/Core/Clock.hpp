#pragma once
#include <vector>

//extern Clock* s_systemClock;
class Clock
{
public:
	Clock();
	explicit Clock(Clock& parent);
	~Clock();                               //? should delete?
	Clock(const Clock& copy) = delete;

	void Reset();							//? last updated time ??

	bool IsPaused() const;
	void Pause();
	void Unpause();
	void TogglePause();                     //not sure

	void StepSingleFrame();					//not sure

	void	SetTimeScale(float timeScale);
	double	GetTimeScale() const;

	double	GetDeltaSeconds() const;
	double	GetTotalSeconds() const;
	int		GetFrameCount() const;

public:
	static Clock& GetSystemClock();			//?
	static void TickSystemClock();			//?
	static const Clock s_systemClock;
protected:
	void Tick();							//?

	void Advance(double deltaTimeSeconds);

	void AddChild(Clock* childClock);
	void RemoveChild(Clock* childClock);	//?

protected:
	Clock* m_parent = nullptr;
	std::vector<Clock*> m_children;

	//Book keeping variables
	double m_lastUpdateTimeInSeconds = 0.0;
	double m_totalSeconds = 0.0;
	double m_deltaSeconds = 0.0;
	int m_frameCount = 0;

	double m_timeScale = 1.0;
	bool m_isPaused = false;
	bool m_stepSingleFrame = false;
	float m_maxDeltaSeconds = 0.1f;
};