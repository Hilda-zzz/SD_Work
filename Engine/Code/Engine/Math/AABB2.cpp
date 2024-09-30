#include "AABB2.hpp"
#include "MathUtils.hpp"
AABB2::AABB2(AABB2 const& copyFrom):m_mins(copyFrom.m_mins),m_maxs(copyFrom.m_maxs)
{

}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
{
	m_mins.x = minX;
	m_mins.y = minY;
	m_maxs.x = maxX;
	m_maxs.y = maxY;
}

AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

bool AABB2::IsPointInside(Vec2 const& point) const
{
	if (point.x > m_mins.x && point.x<m_maxs.x
		&& point.y>m_mins.y && point.y < m_maxs.y)
	{
		return true;
	}
	return false;
}

Vec2 const AABB2::GetCenter() const
{
	Vec2 centerPoint{ (m_mins.x + m_maxs.x) / 2.f,(m_mins.y + m_maxs.y) / 2.f };
	return centerPoint;
}

Vec2 const AABB2::GetDimensions() const
{
	return Vec2(m_maxs.x-m_mins.x, m_maxs.y - m_mins.y);
}

Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	//itself??
	float x = GetClamped(referencePosition.x, m_mins.x, m_maxs.x);
	float y = GetClamped(referencePosition.y, m_mins.y, m_maxs.y);
	return Vec2(x, y);
}

Vec2 const AABB2::GetPointAtUV(Vec2 const& point) const
{
	float x = Interpolate(m_mins.x, m_maxs.x, point.x);
	float y = Interpolate(m_mins.y, m_maxs.y, point.y);
	return Vec2(x, y);
}

Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	float u=GetFractionWithinRange(point.x, m_mins.x, m_maxs.x);
	float v = GetFractionWithinRange(point.y, m_mins.y, m_maxs.y);
	return Vec2(u, v);
}

void AABB2::Translate(Vec2 const& translationToApply)
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 centerPoint = GetCenter();
	m_mins += newCenter- centerPoint;
	m_maxs += newCenter - centerPoint;
}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 centerPoint = GetCenter();
	m_mins = centerPoint - 0.5 * newDimensions;
	m_maxs = centerPoint + 0.5 * newDimensions;
}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (point.x < m_mins.x)
	{
		m_mins.x = point.x;
	}
	else if (point.x > m_maxs.x)
	{
		m_maxs.x = point.x;
	}

	if (point.y< m_mins.y)
	{
		m_mins.y = point.y;
	}
	else if (point.y > m_maxs.y)
	{
		m_maxs.y = point.y;
	}
}
