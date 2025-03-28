#pragma once
#include "Game/AnimState.hpp"
#include "Player.hpp"

class LegIdleState : public AnimState 
{
public:
	LegIdleState(SpriteAnimDefinition* anim);
	
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) override; 
};