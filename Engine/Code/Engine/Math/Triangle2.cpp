#include "Triangle2.hpp"

void Triangle2::Translate(Vec2 translation)
{
	for (int i = 0; i < 3; i++)
	{
		m_pointsCounterClockwise[i] += translation;
	}
}