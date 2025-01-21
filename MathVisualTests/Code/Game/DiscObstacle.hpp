#pragma once
#include "Engine/Math/Vec2.hpp"

class DiscObstacle
{
public:
	DiscObstacle(){}
	DiscObstacle(Vec2 center,float radius);
	~DiscObstacle(){}
	void Render();
public:
	Vec2 m_center;
	float m_radius;
	bool m_isRaycast = false;
};