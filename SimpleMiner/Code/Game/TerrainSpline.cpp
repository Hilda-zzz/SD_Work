#include "TerrainSpline.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <algorithm>

TerrainSpline::TerrainSpline()
	: m_inputMin(-1.0f)
	, m_inputMax(1.0f)
	, m_needsRebuild(false)
{
	m_controlPoints.push_back(Vec2(-1.0f, -1.0f));
	m_controlPoints.push_back(Vec2(1.0f, 1.0f));
	RebuildSpline();
}

TerrainSpline::TerrainSpline(std::vector<Vec2> const& points, float inputMin, float inputMax)
	: m_controlPoints(points)
	, m_inputMin(inputMin)
	, m_inputMax(inputMax)
	, m_needsRebuild(true)
{
	SortControlPoints();
	RebuildSpline();
}

float TerrainSpline::Evaluate(float input) const
{
	if (m_controlPoints.empty())
		return 0.0f;
	
	if (m_controlPoints.size() == 1)
		return m_controlPoints[0].y;
	
	float normalizedInput = RangeMapClamped(input, m_inputMin, m_inputMax, 0.0f, 1.0f);
	
	Vec2 result = m_hermiteSpline.EvaluateAtParametric(normalizedInput);
	
	return result.y;
}

void TerrainSpline::SetControlPoints(std::vector<Vec2> const& points)
{
	m_controlPoints = points;
	SortControlPoints();
	RebuildSpline();
}

void TerrainSpline::AddControlPoint(Vec2 const& point)
{
	m_controlPoints.push_back(point);
	SortControlPoints();
	RebuildSpline();
}

void TerrainSpline::RemoveControlPoint(size_t index)
{
	if (index < m_controlPoints.size())
	{
		m_controlPoints.erase(m_controlPoints.begin() + index);
		RebuildSpline();
	}
}

void TerrainSpline::SetControlPoint(size_t index, Vec2 const& newPoint)
{
	if (index < m_controlPoints.size())
	{
		m_controlPoints[index] = newPoint;
		SortControlPoints();
		RebuildSpline();
	}
}

void TerrainSpline::SetInputRange(float minVal, float maxVal)
{
	m_inputMin = minVal;
	m_inputMax = maxVal;
}

void TerrainSpline::GetOutputRange(float& outMin, float& outMax) const
{
	if (m_controlPoints.empty())
	{
		outMin = 0.0f;
		outMax = 0.0f;
		return;
	}
	
	outMin = m_controlPoints[0].y;
	outMax = m_controlPoints[0].y;
	
	for (size_t i = 1; i < m_controlPoints.size(); ++i)
	{
		float y = m_controlPoints[i].y;
		if (y < outMin) outMin = y;
		if (y > outMax) outMax = y;
	}
}

void TerrainSpline::RebuildSpline()
{
	if (m_controlPoints.size() < 2)
		return;
	
	std::vector<Vec2> normalizedPoints;
	normalizedPoints.reserve(m_controlPoints.size());
	
	float firstX = m_controlPoints.front().x;
	float lastX = m_controlPoints.back().x;
	float range = lastX - firstX;
	
	if (range < 0.0001f)
		range = 1.0f;
	
	for (const Vec2& point : m_controlPoints)
	{
		float normalizedX = (point.x - firstX) / range;
		normalizedPoints.push_back(Vec2(normalizedX, point.y));
	}
	
	m_hermiteSpline = CubicHermiteSpline2D(normalizedPoints);
	m_needsRebuild = false;
}

void TerrainSpline::SortControlPoints()
{
	std::sort(m_controlPoints.begin(), m_controlPoints.end(),
		[](const Vec2& a, const Vec2& b) {
			return a.x < b.x;
		});
}
