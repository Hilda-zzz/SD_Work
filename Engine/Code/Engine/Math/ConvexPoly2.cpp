#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Core/EngineCommon.hpp>

//-----------------------------------------------------------------------------------------------
ConvexPoly2::ConvexPoly2()
{
}

//-----------------------------------------------------------------------------------------------
ConvexPoly2::ConvexPoly2(std::vector<Vec2> const& vertices)
	: m_vertices(vertices)
{
}

//-----------------------------------------------------------------------------------------------
ConvexPoly2::~ConvexPoly2()
{
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::GenerateRandomConvex(int numVertices, Vec2 const& center, float radius, RandomNumberGenerator& rng)
{
	m_vertices.clear();
	if (numVertices < 3)
		numVertices = 3;

	// 计算最小角度间隔（确保不会太窄）
	float minAngleGap = 360.f / (float)numVertices * 0.5f;  // 至少是均匀分布的一半

	// 生成随机角度（带间隔约束）
	std::vector<float> angles;
	angles.push_back(rng.RollRandomFloatInRange(0.f, 360.f));  // 第一个角度随机

	// 生成剩余角度
	for (int i = 1; i < numVertices; ++i)
	{
		bool validAngleFound = false;
		int attempts = 0;
		float newAngle = 0.f;

		while (!validAngleFound && attempts < 100)
		{
			newAngle = rng.RollRandomFloatInRange(0.f, 360.f);

			// 检查与已有角度的距离
			bool tooClose = false;
			for (int j = 0; j < (int)angles.size(); ++j)
			{
				float diff = fabsf(newAngle - angles[j]);
				// 处理环绕情况（例如 359度 和 1度）
				if (diff > 180.f)
					diff = 360.f - diff;

				if (diff < minAngleGap)
				{
					tooClose = true;
					break;
				}
			}

			if (!tooClose)
			{
				validAngleFound = true;
			}

			attempts++;
		}

		angles.push_back(newAngle);
	}

	// 排序角度
	for (int i = 0; i < (int)angles.size() - 1; ++i)
	{
		for (int j = i + 1; j < (int)angles.size(); ++j)
		{
			if (angles[j] < angles[i])
			{
				float temp = angles[i];
				angles[i] = angles[j];
				angles[j] = temp;
			}
		}
	}

	// 转换为顶点
	for (int i = 0; i < numVertices; ++i)
	{
		Vec2 vertex = Vec2::MakeFromPolarDegrees(angles[i], radius);
		vertex += center;
		m_vertices.push_back(vertex);
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::MakeRegularPolygon(int numSides, Vec2 const& center, float radius)
{
	m_vertices.clear();

	if (numSides < 3)
		numSides = 3;

	float angleStep = 360.f / (float)numSides;

	for (int i = 0; i < numSides; ++i)
	{
		float angle = angleStep * (float)i;
		Vec2 vertex = Vec2::MakeFromPolarDegrees(angle, radius);
		vertex += center;
		m_vertices.push_back(vertex);
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::MakeBox(Vec2 const& mins, Vec2 const& maxs)
{
	m_vertices.clear();

	// Create 4 vertices in CCW order (bottom-left, bottom-right, top-right, top-left)
	m_vertices.push_back(Vec2(mins.x, mins.y));  // BL
	m_vertices.push_back(Vec2(maxs.x, mins.y));  // BR
	m_vertices.push_back(Vec2(maxs.x, maxs.y));  // TR
	m_vertices.push_back(Vec2(mins.x, maxs.y));  // TL
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::MakeBoxCentered(Vec2 const& center, float width, float height)
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	Vec2 mins(center.x - halfWidth, center.y - halfHeight);
	Vec2 maxs(center.x + halfWidth, center.y + halfHeight);

	MakeBox(mins, maxs);
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::SetFromVertices(std::vector<Vec2> const& vertices)
{
	m_vertices = vertices;
	EnsureCCWWinding();
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::RotateAroundPoint(Vec2 const& pivot, float degrees)
{
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		// Translate to origin (relative to pivot)
		Vec2 relativePos = m_vertices[i] - pivot;

		// Rotate
		relativePos = relativePos.GetRotatedDegrees(degrees);

		// Translate back
		m_vertices[i] = relativePos + pivot;
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::ScaleAroundPoint(Vec2 const& pivot, float uniformScale)
{
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		// Translate to origin (relative to pivot)
		Vec2 relativePos = m_vertices[i] - pivot;

		// Scale
		relativePos *= uniformScale;

		// Translate back
		m_vertices[i] = relativePos + pivot;
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::Translate(Vec2 const& offset)
{
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		m_vertices[i] += offset;
	}
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::TransformAroundPoint(Vec2 const& pivot, float rotationDegrees, float uniformScale)
{
	// First scale, then rotate (order matters!)
	ScaleAroundPoint(pivot, uniformScale);
	RotateAroundPoint(pivot, rotationDegrees);
}

//-----------------------------------------------------------------------------------------------
ConvexPoly2 ConvexPoly2::GetTransformed(Vec2 const& pivot, float rotationDegrees, float uniformScale) const
{
	ConvexPoly2 result = *this;  // Copy
	result.TransformAroundPoint(pivot, rotationDegrees, uniformScale);
	return result;
}

//-----------------------------------------------------------------------------------------------
Vec2 ConvexPoly2::GetCenter() const
{
	if (m_vertices.empty())
		return Vec2::ZERO;

	Vec2 sum = Vec2::ZERO;
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		sum += m_vertices[i];
	}

	return sum / (float)m_vertices.size();
}

//-----------------------------------------------------------------------------------------------
float ConvexPoly2::GetBoundingRadius() const
{
	Vec2 center = GetCenter();
	float maxDistSq = 0.f;

	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		float distSq = (m_vertices[i] - center).GetLengthSquared();
		if (distSq > maxDistSq)
		{
			maxDistSq = distSq;
		}
	}

	return sqrtf(maxDistSq);
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::GetBounds(Vec2& out_mins, Vec2& out_maxs) const
{
	if (m_vertices.empty())
	{
		out_mins = Vec2::ZERO;
		out_maxs = Vec2::ZERO;
		return;
	}

	out_mins = m_vertices[0];
	out_maxs = m_vertices[0];

	for (int i = 1; i < (int)m_vertices.size(); ++i)
	{
		if (m_vertices[i].x < out_mins.x) out_mins.x = m_vertices[i].x;
		if (m_vertices[i].y < out_mins.y) out_mins.y = m_vertices[i].y;
		if (m_vertices[i].x > out_maxs.x) out_maxs.x = m_vertices[i].x;
		if (m_vertices[i].y > out_maxs.y) out_maxs.y = m_vertices[i].y;
	}
}

//-----------------------------------------------------------------------------------------------
bool ConvexPoly2::ContainsPoint(Vec2 const& point) const
{
	int vertexCount = (int)m_vertices.size();
	if (vertexCount < 3)
		return false;

	// For a convex polygon in CCW order, point is inside if it's on the left side of all edges
	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 const& v0 = m_vertices[i];
		Vec2 const& v1 = m_vertices[(i + 1) % vertexCount];

		// Edge vector
		Vec2 edge = v1 - v0;

		// Vector from v0 to point
		Vec2 toPoint = point - v0;

		// Cross product (2D): edge.x * toPoint.y - edge.y * toPoint.x
		float cross = edge.x * toPoint.y - edge.y * toPoint.x;

		// If cross product is negative, point is on the right side (outside for CCW polygon)
		if (cross < 0.f)
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
Vec2 ConvexPoly2::GetVertex(int index) const
{
	if (index < 0 || index >= (int)m_vertices.size())
		return Vec2::ZERO;

	return m_vertices[index];
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::GetEdge(int edgeIndex, Vec2& out_start, Vec2& out_end) const
{
	int vertexCount = (int)m_vertices.size();
	if (vertexCount < 2 || edgeIndex < 0)
	{
		out_start = Vec2::ZERO;
		out_end = Vec2::ZERO;
		return;
	}

	out_start = m_vertices[edgeIndex % vertexCount];
	out_end = m_vertices[(edgeIndex + 1) % vertexCount];
}

//-----------------------------------------------------------------------------------------------
bool ConvexPoly2::IsConvex() const
{
	int vertexCount = (int)m_vertices.size();
	if (vertexCount < 3)
		return false;

	bool hasPositive = false;
	bool hasNegative = false;

	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 const& v0 = m_vertices[i];
		Vec2 const& v1 = m_vertices[(i + 1) % vertexCount];
		Vec2 const& v2 = m_vertices[(i + 2) % vertexCount];

		Vec2 edge1 = v1 - v0;
		Vec2 edge2 = v2 - v1;

		// Cross product
		float cross = edge1.x * edge2.y - edge1.y * edge2.x;

		if (cross > 0.f) hasPositive = true;
		if (cross < 0.f) hasNegative = true;

		// If we have both positive and negative, it's not convex
		if (hasPositive && hasNegative)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
bool ConvexPoly2::IsValid() const
{
	// Check vertex count
	if (m_vertices.size() < 3)
		return false;

	// Check if convex
	return IsConvex();
}

//-----------------------------------------------------------------------------------------------
void ConvexPoly2::EnsureCCWWinding()
{
	float area = GetSignedArea();

	// If area is negative, vertices are in CW order, so reverse them
	if (area < 0.f)
	{
		// Reverse the vertex order
		int count = (int)m_vertices.size();
		for (int i = 0; i < count / 2; ++i)
		{
			Vec2 temp = m_vertices[i];
			m_vertices[i] = m_vertices[count - 1 - i];
			m_vertices[count - 1 - i] = temp;
		}
	}
}

//-----------------------------------------------------------------------------------------------
float ConvexPoly2::GetSignedArea() const
{
	int vertexCount = (int)m_vertices.size();
	if (vertexCount < 3)
		return 0.f;

	// Shoelace formula
	float area = 0.f;
	for (int i = 0; i < vertexCount; ++i)
	{
		Vec2 const& v0 = m_vertices[i];
		Vec2 const& v1 = m_vertices[(i + 1) % vertexCount];

		area += (v0.x * v1.y - v1.x * v0.y);
	}

	return area * 0.5f;
}