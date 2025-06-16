#pragma once
#include "Game/AnimState.hpp"

class PlayerBodyIdleState: public AnimState
{
public:
	PlayerBodyIdleState(std::map<Direction, SpriteAnimDefinition*> const& anim) :AnimState("11", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyWalkState : public AnimState
{
public:
	PlayerBodyWalkState(std::map<Direction, SpriteAnimDefinition*> const& anim):AnimState("22", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};