#pragma once
#include "Engine/Math/Vec3.hpp"

class ZCylinder
{
public:
	ZCylinder() {};
	ZCylinder(Vec3 const& position, float radius, float halfHeight)
		:m_center(position),m_radius(radius),m_halfHeight(halfHeight){}
	~ZCylinder() {};

public:
	Vec3	m_center=Vec3(0.f,0.f,0.f);
	float	m_radius;
	float	m_halfHeight;
};