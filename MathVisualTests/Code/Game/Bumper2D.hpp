#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/OBB2.hpp"


enum class BumperType
{
	DISC,
	OBB2,
	CAPSULE2
};

class Bumper2D
{
public:
	Bumper2D(BumperType type,Vec2 const& discPos, float radius,float elasticity);
	Bumper2D(BumperType type,OBB2 const& obbBox, float elasticity);
	Bumper2D(BumperType type, Capsule2 const& capsule,float elasticity);
	~Bumper2D() {};
public:
	BumperType m_type = BumperType::DISC;

	Vec2 m_discPos;
	float m_discRadius;

	OBB2 m_obbBox;

	Capsule2 m_capsule;

	float m_elasticity =1.f;

};