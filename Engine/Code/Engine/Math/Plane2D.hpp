#pragma once
#include "Engine/Math/Vec2.hpp"

struct Plane2D
{
public:
	Plane2D() = default;
	Plane2D(Vec2 const& normal, float distance);

	// Construct from two points (defines an edge)
	// Normal points to the LEFT of the vector from pointA to pointB (CCW winding)
	static Plane2D FromPoints(Vec2 const& pointA, Vec2 const& pointB);

	// Queries
	float GetSignedDistance(Vec2 const& point) const;		// Positive = in front, Negative = behind
	bool IsPointInFront(Vec2 const& point) const;			// Returns true if point is on the front side
	bool IsPointBehind(Vec2 const& point) const;			// Returns true if point is on the back side
	bool IsPointOnPlane(Vec2 const& point, float epsilon = 0.001f) const;

	// Ray intersection
	// Returns true if ray intersects plane, and sets 't' to the parametric intersection distance
	// Ray is defined as: point(t) = rayStart + t * (rayEnd - rayStart), where t=[0,1]
	bool RaycastPlane(Vec2 const& rayStart, Vec2 const& rayEnd, float& out_t) const;

	// Get closest point on plane to a given point
	Vec2 GetClosestPoint(Vec2 const& point) const;

public:
	Vec2 m_normal = Vec2(1.f, 0.f);		
	float m_distance = 0.f;				// Signed distance from origin to plane along normal direction
};