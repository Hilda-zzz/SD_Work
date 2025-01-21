#pragma once
#include "Engine/Math/Vec2.hpp"
class Triangle2
{
public:
	Vec2 m_pointsCounterClockwise[3];
	void Translate(Vec2 translation);
};