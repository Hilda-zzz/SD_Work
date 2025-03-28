#pragma once
#include "Game/AnimState.hpp"
#include "Player.hpp"

class  PlayerUpperIdleState : public AnimState
{
public:
	PlayerUpperIdleState(SpriteAnimDefinition* anim);

	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};