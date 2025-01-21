#pragma once
#include "Vec2.hpp"

class OBB2
{
public:
	OBB2() {};
	OBB2(Vec2 center,Vec2 ibasis,Vec2 halfDimensions);
	~OBB2() {}
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;
public:
	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const; // for drawing! 
	Vec2 const GetLocalPosForWorldPos(Vec2 const& worldPos) const;
	Vec2 const GetWorldPosForLocalPos(Vec2 const& localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
};