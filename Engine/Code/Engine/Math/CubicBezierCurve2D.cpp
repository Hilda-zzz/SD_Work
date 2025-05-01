#include "CubicBezierCurve2D.hpp"
#include <cmath>
#include "MathUtils.hpp"
#include "Engine/Math/CubicHermiteCurve2D.hpp"

CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	:m_startPos(startPos),m_guidePos1(guidePos1),m_guidePos2(guidePos2),m_endPos(endPos)
{
}

CubicBezierCurve2D::CubicBezierCurve2D(CubicHermiteCurve2D const& fromHermite)
	:m_startPos(fromHermite.m_startPos),m_guidePos1(m_startPos+(fromHermite.m_startVelocity/3.f)),
	m_guidePos2(fromHermite.m_endPos-(fromHermite.m_endVelocity/3.f)),m_endPos(fromHermite.m_endPos)
{
}

Vec2 CubicBezierCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	Vec2 points[4] = { m_startPos, m_guidePos1, m_guidePos2, m_endPos};
	int n = 4;

	for (int r = 1; r < n; ++r)
	{
		for (int i = 0; i < n - r; ++i)
		{
			points[i] = (1.0f - parametricZeroToOne) * points[i] + parametricZeroToOne * points[i + 1];
		}
	}

	return points[0];
}

float CubicBezierCurve2D::GetApproximateLength(int numSubdivisions) const
{
	float totalLength = 0.0f;
	Vec2 prevPoint = m_startPos;

	float step = 1.f/ numSubdivisions;
	for (int i = 1; i <= numSubdivisions; i++)
	{
		Vec2 currentPoint = EvaluateAtParametric(step*i);

		Vec2 dist = currentPoint - prevPoint;
		totalLength += dist.GetLength();

		prevPoint = currentPoint;
	}

	return totalLength;
}

Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	if (distanceAlongCurve <= 0.f)
	{
		return m_startPos;
	}

	float step = 1.f / numSubdivisions;
	float accumulLen = 0.f;
	Vec2 prevPoint = m_startPos;
	float prevLen = 0.f;
	float curRatio = 0.f;
	for (int i = 1; i <= numSubdivisions; i++)
	{
		curRatio += step;
		Vec2 currentPoint = EvaluateAtParametric(curRatio);
		prevLen = accumulLen;
		float dist = (currentPoint - prevPoint).GetLength();
		accumulLen += dist;

		if (prevLen < distanceAlongCurve&&accumulLen>distanceAlongCurve)
		{
			float interpolateRatio = (distanceAlongCurve - prevLen) / dist;
			float x = RangeMap(interpolateRatio, 0.f, 1.f, prevPoint.x, currentPoint.x);
			float y = RangeMap(interpolateRatio, 0.f, 1.f, prevPoint.y, currentPoint.y);
			return Vec2(x, y);
		}
		prevPoint = currentPoint;
	}
	return m_endPos;
}
