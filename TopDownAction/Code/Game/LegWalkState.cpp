#include "LegWalkState.hpp"

LegWalkState::LegWalkState(SpriteAnimDefinition* anim) : AnimState("LegWalk", anim)
{
	m_stateEnum = static_cast<int>(LegState::WALK);
}

int LegWalkState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (!conditions.at("isMoving")) {
		return static_cast<int>(LegState::IDLE);
	}
	else if (conditions.at("isMoving") && conditions.at("isRunning")) {
		return static_cast<int>(LegState::RUN);
	}
	else if (conditions.at("isCrouching")) {
		return static_cast<int>(LegState::CROUCH);
	}

	return static_cast<int>(LegState::WALK);
}
