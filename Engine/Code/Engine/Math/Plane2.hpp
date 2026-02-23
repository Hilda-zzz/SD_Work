#pragma once
#include "Engine/Math/Vec2.hpp"

struct Plane2
{
public:
	Plane2() = default;
	Plane2(Vec2 const& normal, float distance);

	// Construct from two points (defines an edge)
	// Normal points to the RIGHT of the vector from pointA to pointB
	static Plane2 FromPoints(Vec2 const& pointA, Vec2 const& pointB);

	// Queries
	float GetSignedDistance(Vec2 const& point) const;		// Positive = in front, Negative = behind
	bool IsPointInFront(Vec2 const& point) const;			
	bool IsPointBehind(Vec2 const& point) const;			
	bool IsPointOnPlane(Vec2 const& point, float epsilon = 0.001f) const;

	// Get closest point on plane to a given point
	Vec2 GetClosestPoint(Vec2 const& point) const;

public:
	Vec2 m_normal = Vec2(1.f, 0.f);		
	float m_distance = 0.f;				// Signed distance from origin to plane along normal direction
};