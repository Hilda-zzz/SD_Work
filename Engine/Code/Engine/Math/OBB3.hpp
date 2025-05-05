#pragma once
#include "Vec3.hpp"

class OBB3
{
public:
	OBB3() {}
	OBB3(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDimension,Vec3 const& center);
	OBB3(Vec3 const& iBasis, Vec3 const& halfDimension, Vec3 const& center);
	~OBB3() {};

	Vec3 m_iBasis;
	Vec3 m_jBasis;
	Vec3 m_kBasis;
	Vec3 m_halfDimensions;
	Vec3 m_center;
};