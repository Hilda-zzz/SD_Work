#pragma once
#include "Game/AnimState.hpp"
#include "Game/WeaponStrategy.hpp"
#include "PlayerAnimStateEnum.hpp"
class WeaponState : public AnimState 
{
public:
	WeaponState(const std::string& name, WeaponStrategy* strategy, UpperBodyState stateEnum)
		: AnimState(name, strategy->GetIdleAnim()), m_weaponStrategy(strategy) 
	{
		m_stateEnum = static_cast<int>(stateEnum);
	}

	virtual void UpdateWeaponStrategy(WeaponStrategy* strategy) 
	{
		m_weaponStrategy = strategy;
		// 子类应该覆盖这个方法以更新具体的动画
	}

protected:
	WeaponStrategy* m_weaponStrategy;
public:
	WeaponState() = default;
};