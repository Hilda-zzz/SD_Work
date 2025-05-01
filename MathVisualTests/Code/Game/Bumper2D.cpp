#include "Bumper2D.hpp"

Bumper2D::Bumper2D(BumperType type, Vec2 const& discPos, float radius, float elasticity):m_type(type),m_elasticity(elasticity)
{
	switch (type)
	{
	case BumperType::DISC:
		m_discPos = discPos;
		m_discRadius = radius;
		break;
	case BumperType::OBB2:
		break;
	case BumperType::CAPSULE2:
		break;
	default:
		break;
	}
	
}

Bumper2D::Bumper2D(BumperType type, OBB2 const& obbBox, float elasticity) :m_type(type), m_elasticity(elasticity)
{
	switch (type)
	{
	case BumperType::DISC:
		break;
	case BumperType::OBB2:
		m_obbBox = obbBox;
		break;
	case BumperType::CAPSULE2:
		break;
	default:
		break;
	}
}

Bumper2D::Bumper2D(BumperType type, Capsule2 const& capsule, float elasticity) :m_type(type), m_elasticity(elasticity)
{
	switch (type)
	{
	case BumperType::DISC:
		break;
	case BumperType::OBB2:
		break;
	case BumperType::CAPSULE2:
		m_capsule = capsule;
		break;
	default:
		break;
	}
}
