#pragma once
#include "Engine/Math/Vec2.hpp"

struct Ray
{
public:
	Ray() = default;
	Ray(Vec2 fwdNormal,Vec2 startPos, float maxLen);
	Vec2 m_rayFwdNormal;
	Vec2 m_rayStartPos;
	float m_rayMaxLength;
};