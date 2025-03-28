#include "WeaponFireState.hpp"

void WeaponFireState::Enter()
{
	WeaponState::Enter();
	m_weaponStrategy->Fire();
}

void WeaponFireState::UpdateWeaponStrategy(WeaponStrategy* strategy)
{
	m_weaponStrategy = strategy;
	m_anim = strategy->GetFireAnim();
}

int WeaponFireState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if ((m_anim->IsPlayOnceFinished(m_elapsedTime)) || !conditions.at("isFiring")) {
		return static_cast<int>(UpperBodyState::WEAPON_IDLE);
	}

	return static_cast<int>(UpperBodyState::WEAPON_FIRE);
}
