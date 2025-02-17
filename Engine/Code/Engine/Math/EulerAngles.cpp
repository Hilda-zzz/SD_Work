#include "EulerAngles.hpp"
#include "MathUtils.hpp"

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

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	Mat44 matResult = Mat44();
	//float rollDegrees = m_rollDegrees;
	//float pitchDegrees = m_pitchDegrees;
	//float yawDegrees = m_yawDegrees;
	//if (m_rollDegrees < 0.f) rollDegrees = 360.f + m_rollDegrees;
	//if (m_pitchDegrees < 0.f) pitchDegrees = 360.f + m_pitchDegrees;
	//if (m_yawDegrees < 0.f) yawDegrees = 360.f + m_yawDegrees;
	matResult.AppendZRotation(m_yawDegrees);
	matResult.AppendYRotation(m_pitchDegrees);
	matResult.AppendXRotation(m_rollDegrees);
	
	return matResult;
}
