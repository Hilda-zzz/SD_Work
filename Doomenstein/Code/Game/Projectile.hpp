#pragma once
#include "Game/Actor.hpp"
#include "Engine/Math/ZCylinder.hpp"

class Projectile : public Actor
{
public:
	Projectile(Vec3 const& position, EulerAngles const& orientation, Rgba8 const& color,
		bool isStatic = false, ZCylinder cylinder = ZCylinder(Vec3(0.f, 0.f, 0.0625f), 0.0625f, 0.0625f));
	~Projectile();
};