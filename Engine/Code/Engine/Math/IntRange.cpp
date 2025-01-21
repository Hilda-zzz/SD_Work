#include "IntRange.hpp"

const  IntRange IntRange::ZERO=IntRange(0,0);
const IntRange IntRange::ONE = IntRange(1, 1);
const IntRange IntRange::ZERO_TO_ONE = IntRange(0, 1);
IntRange::IntRange()
{
	m_min = 0;
	m_max = 0;
}


IntRange::IntRange(int min, int max):m_min(min),m_max(max)
{
	
}

bool IntRange::IsOnRange(int inputInt)
{
	return (inputInt <= m_max && inputInt >= m_min) ? 1 : 0;
}

bool IntRange::IsOverlappipngWith(IntRange const& inputIntRange)
{
	if (inputIntRange.m_min < m_min)
	{
		if (inputIntRange.m_max >= m_min)
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
		if (inputIntRange.m_min <= m_max)
		{
			return true;
		}
		return false;
	}
}

void IntRange::operator=(IntRange const& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool IntRange::operator==(IntRange const& copyFrom)
{
	if (copyFrom.m_min == m_min && copyFrom.m_max == m_max)
	{
		return true;
	}
	else
		return false;
}

bool IntRange::operator!=(IntRange const& copyFrom)
{
	if (copyFrom.m_min != m_min)
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
