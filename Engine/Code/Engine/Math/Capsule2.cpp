#include "Capsule2.hpp"

void Capsule2::Translate(Vec2 translation)
{
	m_bone.Translate(translation);
}

void Capsule2::SetCenter(Vec2 newCenter)
{
	m_bone.SetCenter(newCenter);
}

void Capsule2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_bone.RotateAboutCenter(rotationDeltaDegrees);
}
