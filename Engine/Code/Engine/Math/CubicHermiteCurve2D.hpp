#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"

class CubicHermiteCurve2D
{
public:
	CubicHermiteCurve2D(Vec2 startPos, Vec2 startVelocity, Vec2 endPos, Vec2 endVelocity);
	explicit CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier);

	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;
private:
	float h00(float t) const {
		return 2.0f * t * t * t - 3.0f * t * t + 1.0f;
	}

	float h10(float t) const {
		return t * t * t - 2.0f * t * t + t;
	}

	float h01(float t) const {
		return -2.0f * t * t * t + 3.0f * t * t;
	}

	float h11(float t) const {
		return t * t * t - t * t;
	}

public:
	Vec2 m_startPos;
	Vec2 m_startVelocity;
	Vec2 m_endPos;
	Vec2 m_endVelocity;
};