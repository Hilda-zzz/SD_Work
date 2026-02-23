#include "Game/ConvexObject2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Core/Vertex_PCU.hpp>
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

// Static color constants definition
//const Rgba8 ConvexObject2::COLOR_FILL_NORMAL = Rgba8(150, 180, 220, 128);      // 浅蓝色半透明
//const Rgba8 ConvexObject2::COLOR_FILL_HIGHLIGHT = Rgba8(255, 255, 100, 180);   // 黄色高亮

const Rgba8 ConvexObject2::COLOR_FILL_NORMAL = Rgba8(102,178, 255, 255);     
const Rgba8 ConvexObject2::COLOR_FILL_HIGHLIGHT = Rgba8(51, 51, 255, 255);   
const Rgba8 ConvexObject2::COLOR_FILL_NORMAL_TRANS = Rgba8(102, 178, 255, 120);
const Rgba8 ConvexObject2::COLOR_FILL_HIGHLIGHT_TRANS = Rgba8(51, 51, 255, 120);


const Rgba8 ConvexObject2::COLOR_FILL_DRAGGING = Rgba8(50, 50, 50, 128);       

const Rgba8 ConvexObject2::COLOR_EDGE_NORMAL = Rgba8(0, 51, 102, 255);      
const Rgba8 ConvexObject2::COLOR_EDGE_HIGHLIGHT = Rgba8(0, 51, 102, 255);

const float ConvexObject2::EDGE_THICKNESS_NORMAL = 4.f;
const float ConvexObject2::EDGE_THICKNESS_HIGHLIGHT = 6.f;

//-----------------------------------------------------------------------------------------------
ConvexObject2::ConvexObject2()
{
}

//-----------------------------------------------------------------------------------------------
ConvexObject2::~ConvexObject2()
{
}

AABB2 ConvexObject2::GetBounds() const
{
	return m_cachedAABB;
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::GenerateRandom(int numVertices, Vec2 const& center, float minRadius, float maxRadius, RandomNumberGenerator& rng)
{
	m_generateCenter = center;
	m_generateRadius = rng.RollRandomFloatInRange(minRadius, maxRadius);
	// Generate random convex polygon
	m_poly.GenerateRandomConvex(numVertices, center, m_generateRadius, rng);

	// Rebuild hull from the polygon
	RebuildHullFromPoly();

	// Update cached bounds
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::MakeRegularPolygon(int numSides, Vec2 const& center, float radius)
{
	m_poly.MakeRegularPolygon(numSides, center, radius);
	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::MakeBox(Vec2 const& center, float width, float height)
{
	m_poly.MakeBoxCentered(center, width, height);
	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
bool ConvexObject2::ContainsPoint(Vec2 const& point) const
{
	// Use hull for efficient point containment test
	return m_hull.ContainsPoint(point);
	//return false;
}

//-----------------------------------------------------------------------------------------------
//RaycastResult2D ConvexObject2::Raycast(Vec2 const& rayStart, Vec2 const& rayEnd, bool generateDebugData) const
//{
//	return m_hull.Raycast(rayStart, rayEnd);
//}

RaycastResult2D ConvexObject2::Raycast(Vec2 const& rayStart, Vec2 const& fwdNormal, float maxDist) const
{
	return m_hull.RaycastVsPlanes(rayStart, fwdNormal, maxDist);
}

//-----------------------------------------------------------------------------------------------
bool ConvexObject2::DoesRayIntersectBoundingDisc(Vec2 const& rayStart, Vec2 const& rayEnd) const
{
	// Check if ray intersects the bounding disc (early rejection test)
	//return DoesDiscIntersectLineSegment2D(m_cachedCenter, m_cachedBoundingRadius, rayStart, rayEnd);
	return false;
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RotateAroundPoint(Vec2 const& pivot, float degrees)
{
	m_poly.RotateAroundPoint(pivot, degrees);

	Vec2 relativePos = m_generateCenter - pivot;
	m_generateCenter = relativePos.GetRotatedDegrees(degrees) + pivot;

	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::ScaleAroundPoint(Vec2 const& pivot, float uniformScale)
{
	m_poly.ScaleAroundPoint(pivot, uniformScale);

	Vec2 relativePos = m_generateCenter - pivot;
	m_generateCenter = relativePos * uniformScale + pivot;
	m_generateRadius *= uniformScale;

	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::Translate(Vec2 const& offset)
{
	m_poly.Translate(offset);

	m_generateCenter += offset;

	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::TransformAroundPoint(Vec2 const& pivot, float rotationDegrees, float uniformScale)
{
	m_poly.TransformAroundPoint(pivot, rotationDegrees, uniformScale);

	// 同步更新 generateCenter
	Vec2 relativePos = m_generateCenter - pivot;
	relativePos = relativePos.GetRotatedDegrees(rotationDegrees) * uniformScale;
	m_generateCenter = relativePos + pivot;
	// 同步更新 generateRadius
	m_generateRadius *= uniformScale;

	RebuildHullFromPoly();
	UpdateCachedBounds();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RenderFill(int drawMode) const
{
	g_theRenderer->SetModelConstants();

	std::vector<Vec2> const& vertices = m_poly.GetVertices();
	int vertexCount = (int)vertices.size();

	if (vertexCount < 3)
		return;

	// 根据状态选择颜色
	Rgba8 fillColor;
	if (drawMode == 1)
	{
		// Mode 1 配色：更鲜艳的填充
		switch (m_renderState)
		{
		case RenderState::HIGHLIGHTED:
			fillColor = COLOR_FILL_HIGHLIGHT_TRANS;
			break;
		case RenderState::DRAGGING:
			fillColor = COLOR_FILL_DRAGGING;
			break;
		default:
			fillColor = COLOR_FILL_NORMAL_TRANS;
			break;
		}
	}
	else
	{
		// Mode 0 配色（原来的）
		switch (m_renderState)
		{
		case RenderState::HIGHLIGHTED:
			fillColor = COLOR_FILL_HIGHLIGHT;
			break;
		case RenderState::DRAGGING:
			fillColor = COLOR_FILL_DRAGGING;
			break;
		default:
			fillColor = COLOR_FILL_NORMAL;
			break;
		}
	}

	// 绘制填充
	for (int i = 1; i < vertexCount - 1; ++i)
	{
		Vertex_PCU triangleVerts[3];
		triangleVerts[0] = Vertex_PCU(Vec3(vertices[0].x, vertices[0].y, 0.f), fillColor, Vec2::ZERO);
		triangleVerts[1] = Vertex_PCU(Vec3(vertices[i].x, vertices[i].y, 0.f), fillColor, Vec2::ZERO);
		triangleVerts[2] = Vertex_PCU(Vec3(vertices[i + 1].x, vertices[i + 1].y, 0.f), fillColor, Vec2::ZERO);
		g_theRenderer->DrawVertexArray(3, triangleVerts);
	}

	// RenderEdgeNorms();
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RenderEdges(int drawMode) const
{
	UNUSED(drawMode);

	g_theRenderer->SetModelConstants();

	std::vector<Vec2> const& vertices = m_poly.GetVertices();
	int vertexCount = (int)vertices.size();

	if (vertexCount < 2)
		return;

	// 根据状态选择颜色和粗细
	Rgba8 edgeColor;
	float edgeThickness;

	if (m_renderState == RenderState::HIGHLIGHTED || m_renderState == RenderState::DRAGGING)
	{
		edgeColor = COLOR_EDGE_HIGHLIGHT;
		edgeThickness = EDGE_THICKNESS_HIGHLIGHT;
	}
	else
	{
		edgeColor = COLOR_EDGE_NORMAL;
		edgeThickness = EDGE_THICKNESS_NORMAL;
	}

	// 绘制边缘
	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 start = vertices[i];
		Vec2 end = vertices[(i + 1) % vertexCount];
		DebugDrawLine(start, end, edgeThickness, edgeColor);
	}
}

//-----------------------------------------------------------------------------------------------
//void ConvexObject2::RenderFull() const
//{
//	RenderFill(int drawMode);
//	RenderEdges(int drawMode);
//	// RenderEdgeNorms();
//}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RenderBoundingDisc() const
{
	//// Draw bounding circle in debug color
	//DebugDrawHighCircle(m_generateRadius, m_generateCenter, Rgba8(0, 0, 0, 128));
	DebugDrawRingHighRes(2.f, m_generateRadius, Rgba8(0, 0, 0, 90), m_generateCenter);

}

void ConvexObject2::RenderEdgeNorms() const
{
	// Draw edge normals
	std::vector<Plane2> const& planes = m_hull.GetPlanes();
	std::vector<Vec2> const& vertices = m_poly.GetVertices();
	int vertexCount = (int)vertices.size();

	for (int i = 0; i < vertexCount; ++i)
	{
		// Get edge midpoint
		Vec2 const& v0 = vertices[i];
		Vec2 const& v1 = vertices[(i + 1) % vertexCount];
		Vec2 edgeMidpoint = (v0 + v1) * 0.5f;

		// Get normal from corresponding plane
		if (i < (int)planes.size())
		{
			Vec2 normal = planes[i].m_normal;
			Vec2 normalEnd = edgeMidpoint + normal * 15.f;  // 15 units long

			// Draw normal arrow (yellow)
			g_theRenderer->SetModelConstants();
			DebugDrawLine(edgeMidpoint, normalEnd, 1.5f, Rgba8(255, 255, 0, 255));
		}
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RenderRaycastDebug(RaycastResult2D const& result) const
{
	// TODO: Implement detailed raycast debug visualization
	// Show test points, rejecting planes, etc.
	UNUSED(result);
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::RebuildHullFromPoly()
{
	m_hull.BuildFromConvexPoly(m_poly);
}

//-----------------------------------------------------------------------------------------------
void ConvexObject2::UpdateCachedBounds()
{
	m_cachedAABB.m_mins = m_generateCenter - Vec2(m_generateRadius, m_generateRadius);
	m_cachedAABB.m_maxs = m_generateCenter + Vec2(m_generateRadius, m_generateRadius);
}

//-----------------------------------------------------------------------------------------------
//Rgba8 ConvexObject2::GetModulatedFillColor() const
//{
//	if (m_isHighlighted)
//	{
//		// Brighten fill color when highlighted
//		return Rgba8(
//			(unsigned char)GetClamped(m_fillColor.r + 50, 0, 255),
//			(unsigned char)GetClamped(m_fillColor.g + 50, 0, 255),
//			(unsigned char)GetClamped(m_fillColor.b + 50, 0, 255),
//			m_fillColor.a
//		);
//	}
//	return m_fillColor;
//}
//
////-----------------------------------------------------------------------------------------------
//Rgba8 ConvexObject2::GetModulatedEdgeColor() const
//{
//	if (m_isHighlighted)
//	{
//		// Make edge brighter and yellow-ish when highlighted
//		return Rgba8(255, 255, 100, 255);
//	}
//	return m_edgeColor;
//	//return Rgba8::WHITE;
//}