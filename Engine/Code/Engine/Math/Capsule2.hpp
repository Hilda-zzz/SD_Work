#pragma once

#include "LineSegment2.hpp"

class Capsule2
{
public:
	Capsule2():m_bone(LineSegment2(Vec2::ZERO, Vec2::ZERO))
	{
		
	}

	Capsule2(LineSegment2 const& bone, float radius)
		:m_bone(bone),m_radius(radius)
	{
		m_boundRadius = m_radius + (m_bone.m_end - m_bone.m_start).GetLength() / 2.f;
	}

	LineSegment2 m_bone;
	float m_radius;
	float m_boundRadius;
public:
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float ratationDeltaDegrees);
};