#include "MathUtils.hpp"
#include <cmath> 
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "IntVec2.hpp"
#define PI 3.1415926535897f

//Angle
float ConvertDegreesToRadians(float degrees)
{
	return (PI / 180.0f) * degrees;
}
float ConvertRadiansToDegrees(float radians)
{
	return (180.0f * radians) / PI;
}
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return std::cos(radians);
}
float SinDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return std::sin(radians);
}
float Atan2Degrees(float y, float x)
{
	//minus??
	float cur_radians = std::atan2(y, x);
	float cur_degrees = ConvertRadiansToDegrees(cur_radians);
	return cur_degrees;
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float disp = endDegrees - startDegrees;
	while (disp > 180.f)
	{
		disp -= 360.f;
	}

	while (disp < -180.f)
	{
		disp += 360.f;
	}

	return disp;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float angularDispDegrees = GetShortestAngularDispDegrees(currentDegrees, goalDegrees) ;
	if (fabsf(angularDispDegrees) > maxDeltaDegrees)
	{
		if(angularDispDegrees>=0)
			return currentDegrees + maxDeltaDegrees;
		else
			return currentDegrees - maxDeltaDegrees;
	}
	if (fabsf(angularDispDegrees) <= maxDeltaDegrees)
		return goalDegrees;
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	return ConvertRadiansToDegrees(acosf(DotProduct2D(a,b)/(a.GetLength()*b.GetLength())));
}

//basic 2d 3d utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return std::sqrt((positionA.x-positionB.x)* (positionA.x - positionB.x)+
		(positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x*b.x+a.y*b.y;
}

//?
float GetDistance2DInt(IntVec2 const& positionA, IntVec2 const& positionB)
{
	return std::sqrt((positionA.x - positionB.x) * (positionA.x - positionB.x) +
		(positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB) 
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) +
		(positionA.y - positionB.y) * (positionA.y - positionB.y);
}
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	float dz = positionB.z - positionA.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	float dz = positionB.z - positionA.z;
	return dx * dx + dy * dy + dz * dz;

}
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	return std::sqrt(dx * dx + dy * dy);
}
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	return dx * dx + dy * dy;
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject,vectorToProjectOnto.GetNormalized());
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normal = vectorToProjectOnto.GetNormalized();
	return DotProduct2D(vectorToProject, normal)* normal;
}

//Geomertric query utilities
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float cur_dis = GetDistance2D(centerA, centerB);
	if (cur_dis <= radiusA + radiusB)
	{
		return true;
	}
	else
		return false;
}
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float cur_dis = GetDistance3D(centerA, centerB);
	if (cur_dis <= radiusA + radiusB)
	{
		return true;
	}
	else
		return false;
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	if (GetDistance2D(point, discCenter) < discRadius)
	{
		return true;
	}
	return false;
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) < sectorRadius)
	{
	/*	if (GetAngleDegreesBetweenVectors2D(point - sectorTip, Vec2::MakeFromPolarDegrees(sectorApertureDegrees, sectorRadius).GetNormalized())
			< 0.5f * sectorApertureDegrees)*/
		Vec2 point_tip = point- sectorTip ;
		if(abs(GetShortestAngularDispDegrees(point_tip.GetOrientationDegrees(), sectorForwardDegrees))<0.5*sectorApertureDegrees)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) < sectorRadius)
	{
		if (GetAngleDegreesBetweenVectors2D(point-sectorTip,sectorForwardNormal)<0.5f*sectorApertureDegrees)
		{
			return true;
		}
		return false;
	}
	return false;
}

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	if (GetDistance2D(referencePosition, discCenter) <= discRadius)
	{
		return referencePosition;
	}
	else
	{
		Vec2 refer_center = referencePosition - discCenter;
		refer_center.SetLength(discRadius);
		return refer_center + discCenter;
	}
}

bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	Vec2 cp = fixedPoint - mobileDiscCenter;
	if (cp.GetLengthSquared() >= discRadius * discRadius)
	{
		return false;
	}
	else
	{
		Vec2 newCp = cp;
		newCp.SetLength(discRadius);
		mobileDiscCenter = mobileDiscCenter - (newCp - cp);
		return true;
	}
}

bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	Vec2 cp = mobileDiscCenter - fixedDiscCenter;
	if (cp.GetLengthSquared() >= (mobileDiscRadius + fixedDiscRadius)* (mobileDiscRadius + fixedDiscRadius))
	{
		return false;
	}
	else
	{
		cp.SetLength(mobileDiscRadius + fixedDiscRadius);
		mobileDiscCenter =fixedDiscCenter+cp;
		return true;
	}
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 ab = bCenter - aCenter;
	if (ab.GetLengthSquared() >= (aRadius + bRadius) * (aRadius + bRadius))
	{
		return false;
	}
	else
	{
		float overlap = aRadius + bRadius - ab.GetLength();
		ab.SetLength(overlap * 0.5f);
		aCenter += -ab;
		bCenter += ab;
		return true;
	}
}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 p=fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, p);
}

//transform
void TransformPosition2D(Vec2& posToTransform, float uniformScale,
	float rotationDegrees, Vec2 const& translation)
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;

	float cosTheta = CosDegrees(rotationDegrees);
	float sinTheta = SinDegrees(rotationDegrees);

	float newX = posToTransform.x * cosTheta - posToTransform.y * sinTheta;
	float newY = posToTransform.x * sinTheta + posToTransform.y * cosTheta;
	posToTransform.x = newX;
	posToTransform.y = newY;

	posToTransform.x += translation.x;
	posToTransform.y += translation.y;

}
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation)
{
	posToTransform=posToTransform.x * iBasis + posToTransform.y * jBasis + translation;
}

void TransformPositionXY3D(Vec3& posToTransform, float uniformScale,
	float rotationDegrees, Vec2 const& translation)
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;

	float cosTheta = CosDegrees(rotationDegrees);
	float sinTheta = SinDegrees(rotationDegrees);

	float newX = posToTransform.x * cosTheta - posToTransform.y * sinTheta;
	float newY = posToTransform.x * sinTheta + posToTransform.y * cosTheta;
	posToTransform.x = newX;
	posToTransform.y = newY;
	
	posToTransform.x += translation.x;
	posToTransform.y += translation.y;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const jBasis, Vec2 const& translation)
{
	Vec2 posToTransform2 = posToTransform.x * iBasis + posToTransform.y * jBasis + translation;
	posToTransform.x = posToTransform2.x;
	posToTransform.y = posToTransform2.y;

	// posToTransform.x will be changed while this func so the result will be wrong
	//posToTransform.x = posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translation.x;
	//posToTransform.y = posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translation.y;
}
float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	else if (value > maxValue)
	{
		return maxValue;
	}
	else
		return value;
}

float GetClampedZeroToOne(float value)
{
	if (value < 0)
	{
		return 0;
	}
	else if (value > 1)
	{
		return 1;
	}
	else
		return value;
}

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return start + (end - start) * fractionTowardEnd;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	if (rangeEnd != rangeStart) //cannot use rangeEnd>rangeStart cause the order of parameters can be reverted
	{
		return (value - rangeStart) / (rangeEnd - rangeStart);
	}
	return 0.0f;
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float t = GetFractionWithinRange(inValue, inStart, inEnd);
	return Interpolate(outStart, outEnd, t);
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if (inEnd > inStart)
	{
		if (inValue < inStart)
		{
			return outStart;
		}
		else if (inValue > inEnd)
		{
			return outEnd;
		}
		else
			return RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	}
	else
	{
		if (inValue < inEnd)
		{
			return outEnd;
		}
		else if (inValue > inStart)
		{
			return outStart;
		}
		else
			return RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	}
	return 0.0f;
}

int RoundDownToInt(float value)
{
	//?
	return floorf(value);
}
