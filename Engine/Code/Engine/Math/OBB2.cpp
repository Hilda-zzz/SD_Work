#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2(Vec2 center, Vec2 ibasis, Vec2 halfDimensions):m_center(center),m_iBasisNormal(ibasis),m_halfDimensions(halfDimensions)
{
}

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	//bl
	out_fourCornerWorldPositions[0]= m_center - m_iBasisNormal * m_halfDimensions.x -
		m_iBasisNormal.GetRotated90Degrees() * m_halfDimensions.y;
	//br
	out_fourCornerWorldPositions[1] = m_center + m_iBasisNormal * m_halfDimensions.x -
		m_iBasisNormal.GetRotated90Degrees() * m_halfDimensions.y;
	//tr
	out_fourCornerWorldPositions[2] = m_center + m_iBasisNormal * m_halfDimensions.x +
		m_iBasisNormal.GetRotated90Degrees() * m_halfDimensions.y;
	//tl
	out_fourCornerWorldPositions[3] = m_center - m_iBasisNormal * m_halfDimensions.x +
		m_iBasisNormal.GetRotated90Degrees() * m_halfDimensions.y;
}

Vec2 const OBB2::GetLocalPosForWorldPos(Vec2 const& worldPos) const
{
	Vec2 cp = worldPos - m_center;
	Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();
	Vec2 localPosition = Vec2(DotProduct2D(m_iBasisNormal,cp), DotProduct2D(jBasisNormal,cp));
	return localPosition;

}
Vec2 const OBB2::GetWorldPosForLocalPos(Vec2 const& localPos) const
{
	return m_center + m_iBasisNormal * localPos.x +
		m_iBasisNormal.GetRotated90Degrees() *localPos.y;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}
