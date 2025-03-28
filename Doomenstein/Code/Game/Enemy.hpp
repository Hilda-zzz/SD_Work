#pragma once
#include "Game/Actor.hpp"
#include "Engine/Math/ZCylinder.hpp"

class Enemy: public Actor
{
public:
	Enemy(Vec3 const& position, EulerAngles const& orientation, Rgba8 const& color,
		bool isStatic = true,ZCylinder cylinder=ZCylinder(Vec3(0.f,0.f, 0.3525f),0.35f,0.3525f));
	~Enemy();
};