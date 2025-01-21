#pragma once
#include "Vec2.hpp"

class LineSegment2
{
public:
	Vec2 m_start;
	Vec2 m_end;
	Vec2 m_center;
public:
	explicit LineSegment2(Vec2 start, Vec2 end);
	~LineSegment2();
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
};
