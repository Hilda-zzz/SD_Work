#pragma once
#include "Vec2.hpp"
class Plane3
{
public:
	Plane3()=default;
	~Plane3()=default;

	Plane3(Vec3 const& normal, float dist):m_normal(normal),m_distance(dist){}

	Vec3 m_normal = Vec3(0.f,0.f,0.f);
	float m_distance;
};