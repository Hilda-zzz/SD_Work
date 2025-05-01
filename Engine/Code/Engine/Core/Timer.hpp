#pragma once
class Clock;

class Timer
{
public:
	Timer(){}
	explicit Timer(float period, const Clock* clock = nullptr);

	void	Start();
	void	Stop();

	double	GetElapsedTime() const;
	float	GetElapsedFraction() const;
	bool	IsStopped() const;
	bool	HasPeriodElapsed() const;
	bool	DecrementPeriodIfElapsed();

	const	Clock* m_clock = nullptr;
	double	m_startTime = -1.f;
	double	m_period = 0.f;
};