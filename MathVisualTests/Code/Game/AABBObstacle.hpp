#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

class AABBObstacle
{
public:
	AABBObstacle() {};
	AABBObstacle(AABB2& box);
	~AABBObstacle() {}
	void Render();
public:
	AABB2 m_AABB2;
	bool m_isRaycast = false;
};