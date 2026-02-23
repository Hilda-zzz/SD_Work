#include "Plane2.hpp"
#include "MathUtils.hpp"

Plane2::Plane2(Vec2 const& normal, float distance)
	:m_normal(normal),m_distance(distance)
{
}

Plane2 Plane2::FromPoints(Vec2 const& pointA, Vec2 const& pointB)
{
	Vec2 dir = pointB - pointA;

	Vec2 normal = dir.GetRotatedMinus90Degrees();
	normal.Normalize();

	float distance = DotProduct2D(normal, pointA);

	return Plane2(normal, distance);
}

float Plane2::GetSignedDistance(Vec2 const& point) const
{
	return  DotProduct2D(m_normal, point)-m_distance;
}

bool Plane2::IsPointInFront(Vec2 const& point) const
{
	return GetSignedDistance(point) > 0.f;
}

bool Plane2::IsPointBehind(Vec2 const& point) const
{
	return GetSignedDistance(point) < 0.f;
}

bool Plane2::IsPointOnPlane(Vec2 const& point, float epsilon) const
{
	float distance = GetSignedDistance(point);
	return (distance >= -epsilon) && (distance <= epsilon);
}

Vec2 Plane2::GetClosestPoint(Vec2 const& point) const
{
	float signedDistance = GetSignedDistance(point);
	return point - (signedDistance * m_normal);
}
