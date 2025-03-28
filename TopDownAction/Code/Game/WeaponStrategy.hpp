#pragma once
#include "PlayerWeaponEnum.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class WeaponStrategy 
{
public:
	virtual ~WeaponStrategy() = default;

	virtual SpriteAnimDefinition* GetIdleAnim() const = 0;
	virtual SpriteAnimDefinition* GetFireAnim() const = 0;
	virtual SpriteAnimDefinition* GetReloadAnim() const = 0;

	virtual void UpdateWeaponState(float deltaTime,
		std::unordered_map<std::string, bool>& conditions) = 0;

	virtual void Fire() = 0;
	virtual void Reload() = 0;

	virtual WeaponType GetWeaponType() const = 0;
};