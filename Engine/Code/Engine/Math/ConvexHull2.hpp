#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Plane2.hpp"
#include <vector>
#include "RaycastUtils.hpp"

class ConvexPoly2;

//struct RaycastVsHullResult2D
//{
//	RaycastResult2D m_raycastResult;
//	Vec2 m_realEnter;
//	Vec2 m_realExit;
//}


class ConvexHull2
{
public:
	ConvexHull2();
	~ConvexHull2();

	void BuildFromConvexPoly(ConvexPoly2 const& poly);

	void BuildFromVertices(std::vector<Vec2> const& vertices);

	void Clear();


	// Check if a point is inside the convex hull (behind all planes)
	bool ContainsPoint(Vec2 const& point) const;

	// Get signed distance to hull (negative = inside, positive = outside)
	float GetDistanceToPoint(Vec2 const& point) const;

	// Raycast against the convex hull
	// Returns closest intersection along the ray, if any
	//RaycastResult2D Raycast(Vec2 const& rayStart, Vec2 const& rayEnd, bool generateDebugData = false) const;
	RaycastResult2D Raycast(Vec2 const& rayStart, Vec2 const& rayEnd) const;
	RaycastResult2D RaycastVsPlanes(Vec2 const& rayStart, Vec2 const& fwdNormal, float maxDist) const;

	std::vector<Plane2> const& GetPlanes() const { return m_planes; }
	int GetPlaneCount() const { return (int)m_planes.size(); }
	Plane2 const& GetPlane(int index) const { return m_planes[index]; }


	bool IsValid() const;		// At least 3 planes, all properly oriented

private:

	// Test if a point is inside using all planes
	bool IsPointInsideAllPlanes(Vec2 const& point, int& out_rejectingPlaneIndex) const;

	// Find entry and exit fractions along ray
	void GetRayIntersectionRange(Vec2 const& rayStart, Vec2 const& rayEnd,
		float& out_tEnter, float& out_tExit,
		int& out_enterPlaneIndex, int& out_exitPlaneIndex) const;

private:

	std::vector<Plane2> m_planes;		// Planes with outward-facing normals (CCW winding)
};