#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath> 
RaycastResult2D RaycastVsDisc2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist,
	Vec2 discCenter, float discRadius)
{
	RaycastResult2D result= RaycastResult2D(fwdNormal, startPos, maxDist);

	Vec2 sc = discCenter - startPos;
	Vec2 i = fwdNormal;
	Vec2 j = i.GetRotated90Degrees();
	float sc_j = DotProduct2D(sc, j);
	if (sc_j >= discRadius || sc_j <= -discRadius)
	{
		result.m_didImpact = false;
		return result;
	}
	float sc_i = DotProduct2D(sc, i);
	if (sc_i >= maxDist + discRadius || sc_i <= -discRadius)
	{
		result.m_didImpact = false;
		return result;
	}

	if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactNormal = -i*50.f;
		result.m_impactDist = 0.f;
		return result;
	}
	float adjust = std::sqrtf(discRadius * discRadius - sc_j * sc_j);
	result.m_impactDist = sc_i - adjust;
	if (result.m_impactDist >= maxDist)
	{
		result.m_didImpact = false;
		return result;
	}
	else
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos + i * result.m_impactDist;
		result.m_impactNormal = (result.m_impactPos - discCenter).GetNormalized() * 50.f;
		return result;
	}
	
}


RaycastResult2D::RaycastResult2D(Vec2 fwdNormal, Vec2 startPos, float len):m_rayFwdNormal(fwdNormal),m_rayStartPos(startPos), m_rayMaxLength(len)
{
}

RaycastResult2D::RaycastResult2D(Ray const& ray) :m_rayFwdNormal(ray.m_rayFwdNormal), m_rayStartPos(ray.m_rayStartPos), m_rayMaxLength(ray.m_rayMaxLength)
{

}
