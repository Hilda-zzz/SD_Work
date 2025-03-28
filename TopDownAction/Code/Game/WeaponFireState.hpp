#pragma once
#include "Game/WeaponState.hpp"
class WeaponFireState : WeaponState
{
	WeaponFireState(WeaponStrategy* strategy)
		: WeaponState("WeaponFire", strategy, UpperBodyState::WEAPON_FIRE) {}
	void	Enter() override;
	void	UpdateWeaponStrategy(WeaponStrategy* strategy) override;
	int		Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};