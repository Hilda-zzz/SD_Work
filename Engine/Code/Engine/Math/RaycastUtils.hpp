#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Ray.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane3.hpp"

class OBB3;

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

RaycastResult2D RaycastVsLineSegment2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist,
	Vec2 const& lineStart, Vec2 const& lineEnd);

RaycastResult2D RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist,AABB2 const& box);

struct RaycastResult3D
{
public:
	RaycastResult3D() {}
	RaycastResult3D(Vec3 fwdNormal, Vec3 startPos, float len);
public:
	bool m_didImpact = false;
	float m_impactDist = 100000.f;
	Vec3 m_impactPos = Vec3(0.f, 0.f,0.f);
	Vec3 m_impactNormal = Vec3(0.f, 0.f,0.f);

	Vec3 m_rayFwdNormal;
	Vec3 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

RaycastResult3D RaycastVsSphere3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist,
	Vec3 sphereCenter, float sphereRadius);
RaycastResult3D RaycastVsAABB3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, AABB3 const& box);
RaycastResult3D RaycastVsZCylinder3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, Vec3 const& cylinderCenter, float cylinderRadius, float halfHeight);
RaycastResult3D	RaycastVsPlane3(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, Plane3 const& plane);
RaycastResult3D	RaycastVsOBB3(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, OBB3 const& box);

float GetMaxOrMinFromThree(float a, float b, float c,bool isMax);