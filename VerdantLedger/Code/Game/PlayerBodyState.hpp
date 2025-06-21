#pragma once
#include "Game/AnimState.hpp"

class PlayerBodyIdleState: public AnimState
{
public:
	PlayerBodyIdleState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerIdle", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyWalkState : public AnimState
{
public:
	PlayerBodyWalkState(std::map<Direction, SpriteAnimDefinition*>* anim):AnimState("playerWalk", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyRunState : public AnimState
{
public:
	PlayerBodyRunState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerRun", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyAxeState : public AnimState
{
public:
	PlayerBodyAxeState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerAxe", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyHoeState : public AnimState
{
public:
	PlayerBodyHoeState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerHoe", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyPickaxeState : public AnimState
{
public:
	PlayerBodyPickaxeState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerPickaxe", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyShovelState : public AnimState
{
public:
	PlayerBodyShovelState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerShovel", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodySickleState : public AnimState
{
public:
	PlayerBodySickleState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerSickle", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};

class PlayerBodyWaterState : public AnimState
{
public:
	PlayerBodyWaterState(std::map<Direction, SpriteAnimDefinition*>* anim) :AnimState("playerWater", anim) {}
	int Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection) override;
};