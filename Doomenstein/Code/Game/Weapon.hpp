#pragma once
#include "Engine/Math/Vec3.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/Timer.hpp"
class Actor;
class PlayerController;
class Weapon
{
public:
	Weapon(int defIndex,Actor* owner);
	~Weapon();

	bool Fire(Vec3 const& dir, PlayerController* curPlayerController);
	Vec3 GetRandomDirectionInCone(Vec3 const& dir, float coneDegree);

	int m_weaponDefIndex = -1;
	WeaponDefinition const& m_def;

	Timer m_refireTimer;
	Actor* m_owner=nullptr;
};