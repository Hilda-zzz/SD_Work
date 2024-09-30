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
