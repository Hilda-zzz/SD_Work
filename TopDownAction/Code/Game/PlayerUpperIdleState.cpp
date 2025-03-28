#include "PlayerUpperIdleState.hpp"

PlayerUpperIdleState::PlayerUpperIdleState(SpriteAnimDefinition* anim) : AnimState("UpperWalk", anim)
{

}

int  PlayerUpperIdleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (conditions.at("isMoving")) {
		return static_cast<int>(LegState::WALK);
	}
	else if (conditions.at("isMoving") && conditions.at("isRunning")) {
		return static_cast<int>(LegState::RUN);
	}
	else if (conditions.at("isCrouching")) {
		return static_cast<int>(LegState::CROUCH);
	}

	return static_cast<int>(LegState::WALK);
}
