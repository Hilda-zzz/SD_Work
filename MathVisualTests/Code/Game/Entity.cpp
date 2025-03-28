#include "Entity.hpp"

Entity::Entity(Game* owner) :m_game(owner), m_position(Vec3(0.f, 0.f, 0.f)), m_orientation(EulerAngles(0.f, 0.f, 0.f)), m_angularVelocity(EulerAngles(0.f, 0.f, 0.f))
{
}

Entity::~Entity()
{
	m_game = nullptr;
}

Mat44 Entity::GetModelToWorldTransform() const
{
	Mat44 modelToWorld = Mat44::MakeTranslation3D(m_position);
	modelToWorld.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
	return modelToWorld;
}