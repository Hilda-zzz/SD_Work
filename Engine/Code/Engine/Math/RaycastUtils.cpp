#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath> 
#include <cfloat>
#include <algorithm>
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
	float adjust = std::sqrtf(discRadius * discRadius - sc_j * sc_j);
	if (sc_i >= maxDist + discRadius || sc_i <= -adjust)
	{
		result.m_didImpact = false;
		return result;
	}

	if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactNormal = -i;
		result.m_impactDist = 0.f;
		return result;
	}
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
		result.m_impactNormal = (result.m_impactPos - discCenter).GetNormalized();
		return result;
	}
	
}

RaycastResult2D RaycastVsLineSegment2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, Vec2 const& lineStart, Vec2 const& lineEnd)
{
	RaycastResult2D result = RaycastResult2D(fwdNormal, startPos, maxDist);
	Vec2 SE = lineEnd - lineStart;
	Vec2 RS = lineStart - startPos;
	Vec2 RE = lineEnd - startPos;
	Vec2 i = fwdNormal;
	Vec2 j = fwdNormal.GetRotated90Degrees();
	float RSj = DotProduct2D(RS, j);
	float REj = DotProduct2D(RE, j);
	float RSi = DotProduct2D(RS, i);
	float REi = DotProduct2D(RE, i);
	if (RSj * REj >= 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	if (RSi >= maxDist && REi >= maxDist)
	{
		result.m_didImpact = false;
		return result;
	}

	if (RSi <= 0.f && REi <= 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	float tImpact = RSj / (-REj + RSj);
	result.m_impactDist = RSi + tImpact * (REi - RSi);
	result.m_impactPos = startPos + i * result.m_impactDist;

	result.m_impactNormal = SE.GetRotated90Degrees().GetNormalized();
	if (DotProduct2D(i, result.m_impactNormal) > 0.f)
	{
		result.m_impactNormal= SE.GetRotatedMinus90Degrees().GetNormalized();
	}
	if (result.m_impactDist <= 0.f || result.m_impactDist >= maxDist)
	{
		result.m_didImpact = false;
		return result;
	}
	result.m_didImpact = true;
	return result;
}

RaycastResult2D RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist,AABB2 const& box)
{
	RaycastResult2D result = RaycastResult2D(fwdNormal, startPos, maxDist);
	if (fwdNormal.GetLength() == 0.f)
	{
		result.m_didImpact = false;
		return result;
	}
	Vec2 rayEnd = startPos + fwdNormal * maxDist;
	AABB2 rayBox;
	float btX = std::min(startPos.x, rayEnd.x);
	float btY= std::min(startPos.y, rayEnd.y);
	float trX = std::max(startPos.x, rayEnd.x);
	float trY = std::max(startPos.y, rayEnd.y);
	rayBox = AABB2(Vec2(btX, btY), Vec2(trX, trY));

	if (!DoAABB2Overlap(rayBox, box))
	{
		result.m_didImpact = false;
		return result;
	}
	//--------------------------------------------------------------------
	float txMin = 0.0f;
	float txMax = 1.f;
	if ((rayEnd-startPos).x==0.f) 
	{
		if (startPos.x< box.m_mins.x || startPos.x > box.m_maxs.x)
		{
			result.m_didImpact = false;
			return result;
		}
		txMin = -INFINITY;
		txMax = INFINITY;
	}
	else 
	{
		float t1 = (box.m_mins.x - startPos.x) / (fwdNormal.x * maxDist);
		float t2 = (box.m_maxs.x - startPos.x) / (fwdNormal.x * maxDist);

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}

		//txMin = std::max(txMin, t1);
		//txMax = std::min(txMax, t2);

		txMin = t1;
		txMax = t2;
	}
	//--------------------------------------------------------------------
	float tyMin = 0.0f;
	float tyMax = 1.f;
	if ((rayEnd - startPos).y == 0.f)
	{
		if (startPos.y< box.m_mins.y || startPos.y > box.m_maxs.y)
		{
			result.m_didImpact = false;
			return result;
		}
		tyMin = -INFINITY;
		tyMax = INFINITY;
	}
	else
	{
		float t1 =( (box.m_mins.y - startPos.y) /(fwdNormal.y * maxDist));
		float t2 = ((box.m_maxs.y - startPos.y) /(fwdNormal.y * maxDist));

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}

		//tyMin = std::max(tyMin, t1);
		//tyMax = std::min(tyMax, t2);
		tyMin = t1;
		tyMax = t2;
	}
	//--------------------------------------------------------------------
	float tMin = std::max(txMin, tyMin); //in
	float tMax = std::min(txMax, tyMax); //out

 	
 	if (tMax < tMin) 
 	{
 		result.m_didImpact = false;
 		return result;
 	}
 	if (tMax < 0) 
 	{
 		result.m_didImpact = false;
 		return result;
 	}
 	if (tMin > 1) {
 		result.m_didImpact = false;
 		return result;
 	}
	float outHitTime = (tMin >= 0) ? tMin : 0.f;
	result.m_impactDist = outHitTime *maxDist;
	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
	result.m_didImpact = true;
	if (tMin < 0.f)
	{
		result.m_impactNormal = -fwdNormal;
	}
	else
	{
		bool hitXFace = (txMin > tyMin);
		if (hitXFace)
		{
			if (fwdNormal.x > 0)
			{
				result.m_impactNormal = Vec2(-1.f, 0.f);
			}
			else
			{
				result.m_impactNormal = Vec2(1.f, 0.f);
			}
		}
		else
		{
			if (fwdNormal.y > 0)
			{
				result.m_impactNormal = Vec2(0.f, -1.f);
			}
			else
			{
				result.m_impactNormal = Vec2(0.f, 1.f);
			}
		}
	}
	return result;
}


RaycastResult2D::RaycastResult2D(Vec2 fwdNormal, Vec2 startPos, float len):m_rayFwdNormal(fwdNormal),m_rayStartPos(startPos), m_rayMaxLength(len)
{
}

RaycastResult2D::RaycastResult2D(Ray const& ray) :m_rayFwdNormal(ray.m_rayFwdNormal), m_rayStartPos(ray.m_rayStartPos), m_rayMaxLength(ray.m_rayMaxLength)
{

}
