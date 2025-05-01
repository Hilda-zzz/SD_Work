#include "CubicHermiteSpline2D.hpp"
#include "MathUtils.hpp"

CubicHermiteSpline2D::CubicHermiteSpline2D(std::vector<Vec2> const& allPoints)
{
	m_points.reserve(allPoints.size());
	m_velocities.reserve(allPoints.size());
	m_points.push_back(allPoints[0]);
	m_points.push_back(allPoints[1]);
	m_velocities.push_back(Vec2(0.f, 0.f));
	for (size_t i = 2; i < allPoints.size(); i++)
	{
		m_points.push_back(allPoints[i]);
		Vec2 newVelocity = (m_points[i] - m_points[i - 2]) / 2.f;
		m_velocities.push_back(newVelocity);
	}
	m_velocities.push_back(Vec2(0.f, 0.f));

	for (int i = 0; i < (int)allPoints.size() - 1; i++)
	{
		CubicHermiteCurve2D curCurve = CubicHermiteCurve2D(m_points[i], m_velocities[i], m_points[i + 1], m_velocities[i + 1]);
		m_curves.push_back(curCurve);
	}
}

Vec2 CubicHermiteSpline2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	if (m_points.size() < 2)
		return m_points[0];

	float t = GetClamped(parametricZeroToOne, 0.f, 1.f);

	int segmentCount = (int)m_points.size() - 1;
	float scaleCurTime = segmentCount * t;

	int segIndex = (t < 1.0f) ? (int)floorf(scaleCurTime) : (segmentCount - 1);
	float segT = (t < 1.0f) ? (scaleCurTime - segIndex) : 1.0f;
	return m_curves[segIndex].EvaluateAtParametric(segT);
}

float CubicHermiteSpline2D::GetApproximateLength(int numSubdivisions) const
{
	float totalLen = 0.f;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		totalLen += m_curves[i].GetApproximateLength(numSubdivisions);
	}
	return totalLen;
}

Vec2 CubicHermiteSpline2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float prevDist=distanceAlongCurve;
	for (int i = 0; i < (int)m_curves.size(); i++)
	{
		prevDist = distanceAlongCurve;
		distanceAlongCurve -= m_curves[i].GetApproximateLength(numSubdivisions);
		if (distanceAlongCurve <= 0.f)
		{
			return m_curves[i].EvaluateAtApproximateDistance(prevDist);
		}
	}
	return Vec2(-1, -1);
}
