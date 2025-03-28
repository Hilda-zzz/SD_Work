#include "WeaponIdleState.hpp"

void WeaponIdleState::UpdateWeaponStrategy(WeaponStrategy* strategy)
{
	m_weaponStrategy = strategy;
	m_anim = strategy->GetIdleAnim();
}

int WeaponIdleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (conditions.at("isFiring") && conditions.at("canFire")) {
		return static_cast<int>(UpperBodyState::WEAPON_FIRE);
	}
	else if (conditions.at("isReloading") || conditions.at("needsReload")) {
		return static_cast<int>(UpperBodyState::WEAPON_RELOAD);
	}
	else if (conditions.at("isMelee")) {
		return static_cast<int>(UpperBodyState::MELEE_ATTACK);
	}
	else if (conditions.at("isRolling")) {
		return static_cast<int>(UpperBodyState::ROLL);
	}

	return static_cast<int>(UpperBodyState::WEAPON_IDLE);
}
