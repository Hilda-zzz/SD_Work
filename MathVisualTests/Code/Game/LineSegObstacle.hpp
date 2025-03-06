#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/LineSegment2.hpp"

class LineSegObstacle
{
public:
	LineSegObstacle();
	LineSegObstacle(LineSegment2& lineSeg);
	~LineSegObstacle() {}
	void Render();
public:
	LineSegment2 m_lineSeg;
	bool m_isRaycast = false;
};