#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Ray.hpp"
struct RaycastResult2D
{
public:
	RaycastResult2D(){}
	RaycastResult2D(Vec2 fwdNormal, Vec2 startPos, float len);
	RaycastResult2D(Ray const& ray);
public:
	bool m_didImpact = false;
	float m_impactDist=100000.f;
	Vec2 m_impactPos=Vec2(0.f,0.f);
	Vec2 m_impactNormal = Vec2(0.f, 0.f);

	Vec2 m_rayFwdNormal;
	Vec2 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

RaycastResult2D RaycastVsDisc2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist,
	Vec2 discCenter, float discRadius);


