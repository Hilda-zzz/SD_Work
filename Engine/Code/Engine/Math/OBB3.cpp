#include "OBB3.hpp"
#include "MathUtils.hpp"
#include <cmath>

OBB3::OBB3(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDimension, Vec3 const& center)
{
	m_iBasis = iBasis;
	m_jBasis = jBasis;
	m_kBasis = kBasis;
	m_halfDimensions = halfDimension;
	m_center = center;
}

OBB3::OBB3(Vec3 const& iBasis, Vec3 const& halfDimension, Vec3 const& center)
{
	m_iBasis = iBasis.GetNormalized();
	m_halfDimensions = halfDimension;

	Vec3 upVector = Vec3(0.f, 0.f, 1.f); 
	m_jBasis = CrossProduct3D(upVector, m_iBasis).GetNormalized();
	if (m_jBasis.GetLengthSquared() < 0.001f) 
	{
		m_jBasis = CrossProduct3D(Vec3(0.f, 1.f, 0.f), m_iBasis).GetNormalized();

		if (m_jBasis.GetLengthSquared() < 0.001f) {
			m_jBasis = CrossProduct3D(Vec3(1.f, 0.f, 0.f), m_iBasis).GetNormalized();
		}
	}

	m_kBasis = CrossProduct3D(m_iBasis, m_jBasis).GetNormalized();
	m_center = center;
}
