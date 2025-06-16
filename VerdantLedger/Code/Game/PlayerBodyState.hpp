#pragma once
#include "Game/AnimState.hpp"

class PlayerBodyIdleState: public AnimState
{
public:
	PlayerBodyIdleState(SpriteAnimDefinition* anim) :AnimState("11", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};

class PlayerBodyWalkState : public AnimState
{
public:
	PlayerBodyWalkState(SpriteAnimDefinition* anim):AnimState("22", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override;
};