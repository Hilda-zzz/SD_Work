#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
ConvexHull2::ConvexHull2()
{
}

//-----------------------------------------------------------------------------------------------
ConvexHull2::~ConvexHull2()
{
}

//-----------------------------------------------------------------------------------------------
void ConvexHull2::BuildFromConvexPoly(ConvexPoly2 const& poly)
{
	m_planes.clear();

	int vertexCount = poly.GetVertexCount();
	if (vertexCount < 3)
		return;

	// Create one plane per edge
	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 v0 = poly.GetVertex(i);
		Vec2 v1 = poly.GetVertex((i + 1) % vertexCount);

		Plane2 plane = Plane2::FromPoints(v0, v1);
		m_planes.push_back(plane);
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexHull2::BuildFromVertices(std::vector<Vec2> const& vertices)
{
	m_planes.clear();

	int vertexCount = (int)vertices.size();
	if (vertexCount < 3)
		return;

	// Create one plane per edge
	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 const& v0 = vertices[i];
		Vec2 const& v1 = vertices[(i + 1) % vertexCount];

		Plane2 plane = Plane2::FromPoints(v0, v1);
		m_planes.push_back(plane);
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexHull2::Clear()
{
	m_planes.clear();
}

//-----------------------------------------------------------------------------------------------
bool ConvexHull2::ContainsPoint(Vec2 const& point) const
{
	// Point is inside if it's behind (or on) all planes
	for (int i = 0; i < (int)m_planes.size(); ++i)
	{
		if (m_planes[i].IsPointInFront(point))
		{
			return false;  // Point is in front of at least one plane (outside)
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
float ConvexHull2::GetDistanceToPoint(Vec2 const& point) const
{
	if (m_planes.empty())
		return 0.f;

	// Find the smallest signed distance (closest plane)
	float minDistance = m_planes[0].GetSignedDistance(point);

	for (int i = 1; i < (int)m_planes.size(); ++i)
	{
		float dist = m_planes[i].GetSignedDistance(point);
		if (fabsf(dist) < fabsf(minDistance))
		{
			minDistance = dist;
		}
	}

	return minDistance;
}

//-----------------------------------------------------------------------------------------------
//RaycastResult2D ConvexHull2::Raycast(Vec2 const& rayStart, Vec2 const& rayEnd, bool generateDebugData) const
//{
//	RaycastResult2D result;
//
//	if (m_planes.empty())
//		return result;
//
//	// Get entry and exit fractions
//	float tEnter = 0.f;
//	float tExit = 1.f;
//	int enterPlaneIndex = -1;
//	int exitPlaneIndex = -1;
//
//	GetRayIntersectionRange(rayStart, rayEnd, tEnter, tExit, enterPlaneIndex, exitPlaneIndex);
//
//	// Check if ray intersects hull
//	if (tEnter > tExit || tExit < 0.f || tEnter > 1.f)
//	{
//		// No intersection
//		return result;
//	}
//
//	// Clamp to [0, 1]
//	if (tEnter < 0.f)
//		tEnter = 0.f;
//
//	// Ray intersects hull
//	result.m_didImpact = true;
//	//result.m_impactFraction = tEnter;
//
//	Vec2 rayDisp = rayEnd - rayStart;
//	result.m_impactDist = tEnter * rayDisp.GetLength();
//	result.m_impactPos = rayStart + rayDisp * tEnter;
//
//	// Get normal from entering plane
//	if (enterPlaneIndex >= 0 && enterPlaneIndex < (int)m_planes.size())
//	{
//		result.m_impactNormal = m_planes[enterPlaneIndex].m_normal;
//	}
//
//	// Generate debug data if requested
//	//if (generateDebugData)
//	//{
//	//	// TODO: Implement debug data generation
//	//	// For now, just clear the arrays
//	//	result.m_debugTestPoints.clear();
//	//	result.m_debugTestResults.clear();
//	//	result.m_debugRejectingPlaneIndices.clear();
//	//}
//
//	return result;
//}

RaycastResult2D ConvexHull2::Raycast(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	RaycastResult2D result;

	if (m_planes.size() < 3)
		return result;

	Vec2 rayDisp = rayEnd - rayStart;
	float rayLength = rayDisp.GetLength();

	float realEnterT = 0.f;      // 最远的 front->behind（进入）
	float realExitT = 1.f;       // 最近的 behind->front（退出）
	int enterPlaneIndex = -1;

	for (int i = 0; i < (int)m_planes.size(); ++i)
	{
		Plane2 const& plane = m_planes[i];

		float distStart = plane.GetSignedDistance(rayStart);
		float distEnd = plane.GetSignedDistance(rayEnd);

		float denom = distEnd - distStart;

		if (fabsf(denom) < 0.0001f)
		{
			// Ray 平行于该平面
			if (distStart > 0.f)
			{
				// 整条射线都在这个平面的外侧，不可能相交
				return result;
			}
			continue;
		}

		// t = 求交点参数
		float t = -distStart / denom;

		if (distStart > 0.f && distEnd < 0.f)
		{
			// front -> behind（进入）
			if (t > realEnterT)
			{
				realEnterT = t;
				enterPlaneIndex = i;
			}
		}
		else if (distStart < 0.f && distEnd > 0.f)
		{
			// behind -> front（退出）
			if (t < realExitT)
			{
				realExitT = t;
			}
		}
	}

	// Exit 比 Enter 早，射线未穿过凸多边形
	if (realExitT < realEnterT)
		return result;

	// 用中点验证是否在凸包内部
	float midT = (realEnterT + realExitT) * 0.5f;
	Vec2 midPoint = rayStart + rayDisp * midT;

	if (!ContainsPoint(midPoint))
		return result;

	// 确认相交，填充结果
	result.m_didImpact = true;
	//result.m_impactFraction = realEnterT;
	result.m_impactDist = realEnterT * rayLength;
	result.m_impactPos = rayStart + rayDisp * realEnterT;

	if (enterPlaneIndex >= 0)
	{
		result.m_impactNormal = m_planes[enterPlaneIndex].m_normal;
	}

	return result;
}

RaycastResult2D ConvexHull2::RaycastVsPlanes(Vec2 const& rayStart, Vec2 const& fwdNormal, float maxDist) const
{
	RaycastResult2D result;

	if (m_planes.size() < 3)
		return result;

	float realEnterDist = 0.f;       // 最远入口
	float realExitDist = maxDist;    // 最近出口
	int enterPlaneIndex = -1;
	RaycastResult2D enterResult;

	for (int i = 0; i < (int)m_planes.size(); ++i)
	{
		RaycastResult2D planeResult = RaycastVsPlane2D(rayStart, fwdNormal, maxDist, m_planes[i]);

		if (!planeResult.m_didImpact)
		{
			// 未相交，检查是否平行且在外侧
			if (m_planes[i].IsPointInFront(rayStart))
			{
				// 起点在外侧且射线不与该平面相交，不可能穿过凸包
				return result;
			}
			continue;
		}

		float dot = DotProduct2D(fwdNormal, m_planes[i].m_normal);

		if (dot < 0.f)
		{
			// 异向 - 入口
			if (planeResult.m_impactDist > realEnterDist)
			{
				realEnterDist = planeResult.m_impactDist;
				enterPlaneIndex = i;
				enterResult = planeResult;
			}
		}
		else
		{
			// 同向 - 出口
			if (planeResult.m_impactDist < realExitDist)
			{
				realExitDist = planeResult.m_impactDist;
			}
		}
	}

	// Exit 比 Enter 早，不相交
	if (realExitDist < realEnterDist)
		return result;

	// 中点验证
	float midDist = (realEnterDist + realExitDist) * 0.5f;
	Vec2 midPoint = rayStart + fwdNormal * midDist;

	if (!ContainsPoint(midPoint))
		return result;

	// 相交，返回入口结果
	if (enterPlaneIndex >= 0)
	{
		result = enterResult;
		result.m_impactNormal = m_planes[enterPlaneIndex].m_normal;
	}
	else
	{
		// 起点在凸包内部，入口距离为 0
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = rayStart;
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
//bool ConvexHull2::IsValid() const
//{
//	// #TODO 似乎不全面
//	// At least 3 planes needed for a convex hull
//	return m_planes.size() >= 3;
//}

//-----------------------------------------------------------------------------------------------
//bool ConvexHull2::IsPointInsideAllPlanes(Vec2 const& point, int& out_rejectingPlaneIndex) const
//{
//	out_rejectingPlaneIndex = -1;
//
//	for (int i = 0; i < (int)m_planes.size(); ++i)
//	{
//		if (m_planes[i].IsPointBehind(point))
//		{
//			out_rejectingPlaneIndex = i;
//			return false;
//		}
//	}
//
//	return true;
//}

//-----------------------------------------------------------------------------------------------
//void ConvexHull2::GetRayIntersectionRange(Vec2 const& rayStart, Vec2 const& rayEnd,
//	float& out_tEnter, float& out_tExit,
//	int& out_enterPlaneIndex, int& out_exitPlaneIndex) const
//{
//	// Initialize to full ray range
//	out_tEnter = 0.f;
//	out_tExit = 1.f;
//	out_enterPlaneIndex = -1;
//	out_exitPlaneIndex = -1;
//
//	Vec2 rayDisp = rayEnd - rayStart;
//
//	// Test ray against each plane
//	for (int i = 0; i < (int)m_planes.size(); ++i)
//	{
//		Plane2 const& plane = m_planes[i];
//
//		float distStart = plane.GetSignedDistance(rayStart);
//		float distEnd = plane.GetSignedDistance(rayEnd);
//
//		// Check if ray is parallel to plane
//		float dotProduct = DotProduct2D(rayDisp, plane.m_normal);
//
//		if (fabsf(dotProduct) < 0.0001f)
//		{
//			// Ray is parallel to plane
//			if (distStart > 0.f)
//			{
//				// Ray is completely outside this plane
//				out_tEnter = 1.f;
//				out_tExit = 0.f;
//				return;
//			}
//			// Ray is parallel and inside, continue to next plane
//			continue;
//		}
//
//		// Calculate intersection fraction
//		float t = distStart / dotProduct;
//
//		// Determine if this is entry or exit
//		if (dotProduct < 0.f)
//		{
//			// Ray is entering the half-space (going from front to back)
//			if (t > out_tEnter)
//			{
//				out_tEnter = t;
//				out_enterPlaneIndex = i;
//			}
//		}
//		else
//		{
//			// Ray is exiting the half-space (going from back to front)
//			if (t < out_tExit)
//			{
//				out_tExit = t;
//				out_exitPlaneIndex = i;
//			}
//		}
//	}
//}