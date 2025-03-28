#pragma once
#include "Game/WeaponState.hpp"
class WeaponIdleState : WeaponState
{
	WeaponIdleState(WeaponStrategy* strategy)
		: WeaponState("WeaponIdle", strategy, UpperBodyState::WEAPON_IDLE) {}
	void UpdateWeaponStrategy(WeaponStrategy* strategy) override;
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};