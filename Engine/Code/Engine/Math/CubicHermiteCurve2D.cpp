#include "Engine/Math/CubicHermiteCurve2D.hpp"
#include "Engine/Math/MathUtils.hpp"

CubicHermiteCurve2D::CubicHermiteCurve2D(Vec2 startPos, Vec2 startVelocity, Vec2 endPos, Vec2 endVelocity)
	:m_startPos(startPos),m_startVelocity(startVelocity),m_endPos(endPos),m_endVelocity(endVelocity)
{

}

CubicHermiteCurve2D::CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier)
	:m_startPos(fromBezier.m_startPos),m_startVelocity(3.f*(fromBezier.m_guidePos1-m_startPos)),
	m_endPos(fromBezier.m_endPos),m_endVelocity(fromBezier.m_endPos-fromBezier.m_guidePos2)
{
}

Vec2 CubicHermiteCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	float t = GetClamped(parametricZeroToOne, 0.f, 1.f);
	Vec2 result = m_startPos * h00(t) +
		m_startVelocity * h10(t) +
		m_endPos * h01(t) +
		m_endVelocity * h11(t);

	return result;
}

float CubicHermiteCurve2D::GetApproximateLength(int numSubdivisions) const
{
	float totalLength = 0.0f;
	Vec2 prevPoint = m_startPos;

	float step = 1.f / numSubdivisions;
	for (int i = 1; i <= numSubdivisions; i++)
	{
		Vec2 currentPoint = EvaluateAtParametric(step * i);

		Vec2 dist = currentPoint - prevPoint;
		totalLength += dist.GetLength();

		prevPoint = currentPoint;
	}

	return totalLength;
}

Vec2 CubicHermiteCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
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

		if (prevLen < distanceAlongCurve && accumulLen>distanceAlongCurve)
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
