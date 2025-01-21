#pragma once
#include <Engine/Math/Vec2.hpp>

class RaycastArrow
{
public:
	RaycastArrow() {};
	RaycastArrow(Vec2 startPos, Vec2 direction, float maxLen);
	~RaycastArrow() {};
public:
	Vec2 m_startPos;
	Vec2 m_endPos_whole;
	Vec2 m_direction;
	float m_maxLen;
};