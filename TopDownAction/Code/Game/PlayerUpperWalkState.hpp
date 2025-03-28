#pragma once
#include "Game/AnimState.hpp"
#include "Player.hpp"

class PlayerUpperWalkState : public AnimState
{
public:
	PlayerUpperWalkState(SpriteAnimDefinition* anim);

	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};