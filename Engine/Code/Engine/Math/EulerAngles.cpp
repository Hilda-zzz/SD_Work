#include "EulerAngles.hpp"
#include "MathUtils.hpp"
#include "../Core/StringUtils.hpp"

EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees):m_yawDegrees(yawDegrees),m_pitchDegrees(pitchDegrees),m_rollDegrees(rollDegrees)
{
	
}

void EulerAngles::GetAsVectors_IFwd_JLeft_KUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	Mat44 matResult = this->GetAsMatrix_IFwd_JLeft_KUp();
	out_forwardIBasis = matResult.GetIBasis3D();
	out_leftJBasis = matResult.GetJBasis3D();
	out_upKBasis = matResult.GetKBasis3D();
}

Vec3 const EulerAngles::GetForward_IFwd() const
{
	Mat44 matResult = this->GetAsMatrix_IFwd_JLeft_KUp();
	Vec3 out_forwardIBasis = matResult.GetIBasis3D();
	return out_forwardIBasis;
}

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	Mat44 matResult = Mat44();
	matResult.AppendZRotation(m_yawDegrees);
	matResult.AppendYRotation(m_pitchDegrees);
	matResult.AppendXRotation(m_rollDegrees);
	
	return matResult;
}

EulerAngles EulerAngles::operator*(float scale) const
{
	return EulerAngles(m_yawDegrees * scale, m_pitchDegrees * scale,m_rollDegrees*scale);
}

EulerAngles EulerAngles::operator+(EulerAngles const& eulerAnglesToAdd) const
{
	return EulerAngles(m_yawDegrees + eulerAnglesToAdd.m_yawDegrees, m_pitchDegrees+ eulerAnglesToAdd.m_pitchDegrees, m_rollDegrees + eulerAnglesToAdd.m_rollDegrees);
}

void EulerAngles::SetFromText(char const* text)
{
	Strings result = SplitStringOnDelimiterIgnoreSpace(text, ',');
	m_yawDegrees = (float)atof(result[0].c_str());
	m_pitchDegrees = (float)atof(result[1].c_str());
	m_rollDegrees = (float)atof(result[2].c_str());
}
