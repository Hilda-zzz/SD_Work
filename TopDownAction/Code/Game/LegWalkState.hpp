#pragma once
#include "Game/AnimState.hpp"
#include "Player.hpp"

class LegWalkState : public AnimState
{
public:
	LegWalkState(SpriteAnimDefinition* anim);

	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};