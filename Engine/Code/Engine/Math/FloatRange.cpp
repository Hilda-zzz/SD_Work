#include "Engine/Math/FloatRange.hpp"

 const FloatRange FloatRange::ZERO = FloatRange(0.f, 0.f);
 const FloatRange FloatRange::ONE = FloatRange(1.f, 1.f);
 const FloatRange FloatRange::ZERO_TO_ONE = FloatRange(0.f, 1.f);;

FloatRange::FloatRange() :m_min(0.f), m_max(0.f)
{
}

FloatRange::FloatRange(float min, float max):m_min(min),m_max(max)
{
}

bool FloatRange::IsOnRange(float inputFloat)
{
	return (inputFloat <= m_max && inputFloat >= m_min) ? 1 : 0;
}

bool FloatRange::IsOverlappipngWith(FloatRange const& inputFloatRange)
{
	if (inputFloatRange.m_min < m_min)
	{
		if (inputFloatRange.m_max >= m_min)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (inputFloatRange.m_min <= m_max)
		{
			return true;
		}
		return false;
	}
}

void FloatRange::operator=(FloatRange const& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool FloatRange::operator==(FloatRange const& copyFrom)
{
	if (copyFrom.m_min == m_min && copyFrom.m_max == m_max)
	{
		return true;
	}
	else
		return false;
}

bool FloatRange::operator!=(FloatRange const& copyFrom)
{
	if (copyFrom.m_min != m_min )
	{
		return true;
	}
	else if (copyFrom.m_max != m_max)
	{
		return true;
	}
	else
	{
		return false;
	}
}
