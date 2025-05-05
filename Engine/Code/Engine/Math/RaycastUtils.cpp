#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath> 
#include <cfloat>
#include <algorithm>
#include "Engine/Math/OBB3.hpp"
#include "../Core/DebugRenderSystem.hpp"

RaycastResult2D::RaycastResult2D(Vec2 fwdNormal, Vec2 startPos, float len) :m_rayFwdNormal(fwdNormal), m_rayStartPos(startPos), m_rayMaxLength(len)
{
}
RaycastResult2D::RaycastResult2D(Ray const& ray) :m_rayFwdNormal(ray.m_rayFwdNormal), m_rayStartPos(ray.m_rayStartPos), m_rayMaxLength(ray.m_rayMaxLength)
{

}

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
	if ((rayEnd-startPos).x==0.f) //horizontal
	{
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
		txMin = t1;
		txMax = t2;
	}
	//--------------------------------------------------------------------
	float tyMin = 0.0f;
	float tyMax = 1.f;
	if ((rayEnd - startPos).y == 0.f) //vertical
	{
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
 	if (tMin > 1)
	{
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
//-------------------------------------------------------------------------------------------------------
RaycastResult3D::RaycastResult3D(Vec3 fwdNormal, Vec3 startPos, float len) :m_rayFwdNormal(fwdNormal), m_rayStartPos(startPos), m_rayMaxLength(len)
{
}

RaycastResult3D RaycastVsSphere3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, Vec3 sphereCenter, float sphereRadius)
{
	RaycastResult3D result = RaycastResult3D(fwdNormal, startPos, maxDist);

	Vec3 sc = sphereCenter - startPos;
	Vec3 i = fwdNormal;
	Vec3 k = CrossProduct3D(i, sc);
	k.Normalized();
	Vec3 j = CrossProduct3D(k,i);
	float sc_j = DotProduct3D(sc, j);
	if (sc_j >= sphereRadius || sc_j <= -sphereRadius)
	{
		result.m_didImpact = false;
		return result;
	}
	float sc_i = DotProduct3D(sc, i);
	float adjust = std::sqrtf(sphereRadius * sphereRadius - sc_j * sc_j);
	if (sc_i >= maxDist + sphereRadius || sc_i <= -adjust)
	{
		result.m_didImpact = false;
		return result;
	}

	if (IsPointInsideSphere3D(startPos, sphereCenter, sphereRadius))
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
		result.m_impactNormal = (result.m_impactPos - sphereCenter).GetNormalized();
		result.m_impactDist = 0.f;
		return result;
	}
}

RaycastResult3D RaycastVsAABB3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, AABB3 const& box)
{
	RaycastResult3D result = RaycastResult3D(fwdNormal, startPos, maxDist);
	if (fwdNormal.GetLength() == 0.f)
	{
		result.m_didImpact = false;
		return result;
	}
	Vec3 rayEnd = startPos + fwdNormal * maxDist;
	AABB3 rayBox;
	float btX = std::min(startPos.x, rayEnd.x);
	float btY = std::min(startPos.y, rayEnd.y);
	float btZ = std::min(startPos.z, rayEnd.z);
	float trX = std::max(startPos.x, rayEnd.x);
	float trY = std::max(startPos.y, rayEnd.y);
	float trZ = std::max(startPos.z, rayEnd.z);
	rayBox = AABB3(Vec3(btX, btY, btZ), Vec3(trX, trY, trZ));

	if (!DoAABBsOverlap3D(rayBox, box))
	{
		result.m_didImpact = false;
		return result;
	}
	//--------------------------------------------------------------------
	float txMin = 0.0f;
	float txMax = 1.f;
	if ((rayEnd - startPos).x == 0.f) //horizontal
	{
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
		txMin = t1;
		txMax = t2;
	}
	//--------------------------------------------------------------------
	float tyMin = 0.0f;
	float tyMax = 1.f;
	if ((rayEnd - startPos).y == 0.f) //vertical
	{
		tyMin = -INFINITY;
		tyMax = INFINITY;
	}
	else
	{
		float t1 = ((box.m_mins.y - startPos.y) / (fwdNormal.y * maxDist));
		float t2 = ((box.m_maxs.y - startPos.y) / (fwdNormal.y * maxDist));

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}
		tyMin = t1;
		tyMax = t2;
	}
	//--------------------------------------------------------------------
	float tzMin = 0.0f;
	float tzMax = 1.f;
	if ((rayEnd - startPos).z == 0.f) //vertical
	{
		tzMin = -INFINITY;
		tzMax = INFINITY;
	}
	else
	{
		float t1 = ((box.m_mins.z - startPos.z) / (fwdNormal.z * maxDist));
		float t2 = ((box.m_maxs.z - startPos.z) / (fwdNormal.z * maxDist));

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}
		tzMin = t1;
		tzMax = t2;
	}
	//--------------------------------------------------------------------
	float tMin = GetMaxOrMinFromThree(txMin,tyMin,tzMin,true); //in
	float tMax = GetMaxOrMinFromThree(txMax, tyMax, tzMax, false);//out

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
	if (tMin > 1)
	{
		result.m_didImpact = false;
		return result;
	}
	float outHitTime = (tMin >= 0) ? tMin : 0.f;
	result.m_impactDist = outHitTime * maxDist;
	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
	result.m_didImpact = true;
	if (tMin < 0.f)
	{
		result.m_impactNormal = -fwdNormal;
	}
	else
	{
		int hitFace = 0; //1x,2y,3z
		if (tMin == txMin) hitFace = 1;
		else if (tMin == tyMin) hitFace = 2;
		else hitFace = 3;
		if (hitFace==1)
		{
			if (fwdNormal.x > 0)
			{
				result.m_impactNormal = Vec3(-1.f, 0.f,0.f);
			}
			else
			{
				result.m_impactNormal = Vec3(1.f, 0.f, 0.f);
			}
		}
		else if (hitFace == 2)
		{
			if (fwdNormal.y > 0)
			{
				result.m_impactNormal = Vec3(0.f, -1.f,0.f);
			}
			else
			{
				result.m_impactNormal = Vec3(0.f, 1.f,0.f);
			}
		}
		else
		{
			if (fwdNormal.z > 0)
			{
				result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
			}
			else
			{
				result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
			}
		}
	}
	return result;
}

RaycastResult3D RaycastVsZCylinder3D(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, Vec3 const& cylinderCenter, float cylinderRadius, float halfHeight)
{
	RaycastResult3D result = RaycastResult3D(fwdNormal, startPos, maxDist);

	Vec2 startPointInXY = Vec2(startPos.x, startPos.y);
	Vec2 cylinderCenterInXY = Vec2(cylinderCenter.x, cylinderCenter.y);
	Vec3 rayEnd = startPos + fwdNormal * maxDist;
	Vec3 rayEndInXY = Vec3(rayEnd.x, rayEnd.y, 0.f);
	Vec3 rayDirectionInXY = Vec3(fwdNormal.x, fwdNormal.y, 0.f).GetNormalized();
	float distInXY = DotProduct3D(fwdNormal * maxDist, rayDirectionInXY);

	Vec2 fwdInXY = Vec2(fwdNormal.x, fwdNormal.y).GetNormalized();
	RaycastResult2D resultWithDisc=RaycastVsDisc2D(startPointInXY, fwdInXY, distInXY, cylinderCenterInXY,cylinderRadius);
	if (!resultWithDisc.m_didImpact)
	{
		result.m_didImpact = false;
		return result;
	}
	else
	{
		Vec2 nearPoint=GetNearestPointOnInfiniteLine2D(cylinderCenterInXY, startPointInXY+fwdInXY, startPointInXY);
		float distSquare = GetDistanceSquared2D(nearPoint, cylinderCenterInXY);
		float aabb2HalfWidth = sqrt(cylinderRadius*cylinderRadius-distSquare);

		Vec2 rayDirect = Vec2(DotProduct3D(fwdNormal, rayDirectionInXY), DotProduct3D(fwdNormal, Vec3(0.f, 0.f, 1.f))).GetNormalized();
		Vec2 startPointInNewPlane = Vec2();
		Vec3 aabbCenter = Vec3(nearPoint.x, nearPoint.y, cylinderCenter.z);
		startPointInNewPlane.x = DotProduct3D(startPos - aabbCenter, rayDirectionInXY);
		startPointInNewPlane.y = DotProduct3D(startPos - aabbCenter, Vec3(0.f, 0.f, 1.f));
		RaycastResult2D resultWithAABB2 = RaycastVsAABB2D(startPointInNewPlane, rayDirect, maxDist, AABB2(Vec2(-aabb2HalfWidth, -halfHeight), Vec2(aabb2HalfWidth, halfHeight)));
		if (resultWithAABB2.m_didImpact)
		{
			result.m_didImpact = true;
			result.m_impactPos = aabbCenter +resultWithAABB2.m_impactPos.x* rayDirectionInXY +Vec3(0.f,0.f,resultWithAABB2.m_impactPos.y);
			result.m_impactDist = resultWithAABB2.m_impactDist;
			if (resultWithAABB2.m_impactNormal == Vec2(0.f, 1.f)|| resultWithAABB2.m_impactNormal == Vec2(0.f, -1.f))
			{
				result.m_impactNormal = Vec3(0.f, 0.f, resultWithAABB2.m_impactNormal.y);
			}
			else
			{
				if (IsPointInsideCylinder3D(result.m_impactPos,cylinderCenter,cylinderRadius-0.01f,halfHeight))
				{
					result.m_impactNormal = -result.m_rayFwdNormal;
				}
				else
				{
					result.m_impactNormal = (result.m_impactPos - Vec3(cylinderCenter.x, cylinderCenter.y, result.m_impactPos.z)).GetNormalized();
				}
				
			}
		}
	}
	return result;
}

RaycastResult3D RaycastVsPlane3(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, Plane3 const& plane)
{
	//if the length just reach the plane, it should not impact
	RaycastResult3D result = RaycastResult3D(fwdNormal, startPos, maxDist);
	float h = 0.f;
	if (IsPointFrontOfPlane3(startPos,plane))
	{
		h = -DotProduct3D(fwdNormal, plane.m_normal);
		result.m_impactNormal = plane.m_normal;
	}
	else
	{
		h = DotProduct3D(fwdNormal, plane.m_normal);
		result.m_impactNormal = -plane.m_normal;
	}

	if (h <= 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	float altitude = DotProduct3D(startPos, plane.m_normal)-plane.m_distance;
	if (altitude == 0.f)
	{
		//if start point on the plane
		result.m_didImpact = false;
		return result;
	}
	if (abs(altitude) >= maxDist)
	{
		result.m_didImpact = false;
		return result;
	}
	result.m_impactDist = abs(altitude) / h;
	if (result.m_impactDist >= maxDist)
	{
		result.m_didImpact = false;
		return result;
	}

	result.m_didImpact = true;
	result.m_impactPos = startPos + fwdNormal * result.m_impactDist;

	return result;
}

RaycastResult3D RaycastVsOBB3(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist, OBB3 const& box)
{
	RaycastResult3D result = RaycastResult3D(fwdNormal, startPos, maxDist);

	Mat44 mat = Mat44(box.m_iBasis, box.m_jBasis, box.m_kBasis, box.m_center);
	Mat44 invMat = mat.GetOrthonormalInverse();

	Vec3 relativePos = invMat.TransformPosition3D(startPos);
	Vec3 relativeFwd = invMat.TransformDirection3D(fwdNormal);

	relativeFwd.Normalized();

	RaycastResult3D aabbResult = RaycastVsAABB3D(relativePos, relativeFwd, maxDist,
		AABB3(-box.m_halfDimensions, box.m_halfDimensions));

	result.m_didImpact = aabbResult.m_didImpact;

	if (result.m_didImpact)
	{
		result.m_impactPos = mat.TransformPosition3D(aabbResult.m_impactPos);
		result.m_impactNormal = mat.TransformDirection3D(aabbResult.m_impactNormal);
		result.m_impactDist = aabbResult.m_impactDist;
	}

	//DebugAddWorldPoint(result.m_impactPos, 0.2f, 0.f,Rgba8::HILDA);
	return result;
}

float GetMaxOrMinFromThree(float a, float b, float c, bool isMax)
{
	float d;
	if (isMax)
	{
		d = a > b ? a : b;
		d = d > c ? d : c;
		return d;
	}
	else
	{
		d = a < b ? a : b;
		d = d < c ? d : c;
		return d;
	}
}



