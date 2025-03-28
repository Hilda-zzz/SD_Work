#include "LegIdleState.hpp"

LegIdleState::LegIdleState(SpriteAnimDefinition* anim) : AnimState("LegIdle", anim)
{
	m_stateEnum = static_cast<int>(LegState::IDLE);
}

int LegIdleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (conditions.at("isMoving") && conditions.at("isWalking")) {
		return static_cast<int>(LegState::WALK);
	}
	else if (conditions.at("isMoving") && conditions.at("isRunning")) {
		return static_cast<int>(LegState::RUN);
	}
	else if (conditions.at("isCrouching")) {
		return static_cast<int>(LegState::CROUCH);
	}

	return static_cast<int>(LegState::IDLE);
}
