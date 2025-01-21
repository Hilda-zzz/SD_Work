#pragma once

class FloatRange
{
public:
	float m_min;
	float m_max;
	static const  FloatRange ZERO;
	static const  FloatRange ONE;
	static const  FloatRange ZERO_TO_ONE;
public:
	~FloatRange() {}
	FloatRange();
	explicit FloatRange(float min, float max);

	bool IsOnRange(float inputFloat);
	bool IsOverlappipngWith(FloatRange const& inputFloatRange);

	void operator=(FloatRange const& copyFrom);
	bool operator==(FloatRange const& copyFrom);
	bool operator!=(FloatRange const& copyFrom);
};