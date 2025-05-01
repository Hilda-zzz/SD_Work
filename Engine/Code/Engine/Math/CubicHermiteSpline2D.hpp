#pragma once
#include "Vec2.hpp"
#include <vector>
#include "CubicHermiteCurve2D.hpp"

class CubicHermiteSpline2D
{
public:
	CubicHermiteSpline2D(){}
	CubicHermiteSpline2D(std::vector<Vec2> const& allPoints);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;
public:
	std::vector<Vec2> m_points;
	std::vector<Vec2> m_velocities;
	std::vector<CubicHermiteCurve2D> m_curves;
};