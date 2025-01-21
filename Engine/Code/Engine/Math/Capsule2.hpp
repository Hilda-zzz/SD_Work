#pragma once

#include "LineSegment2.hpp"

class Capsule2
{
public:
	LineSegment2 m_bone;
	float radius;
public:
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float ratationDeltaDegrees);
};