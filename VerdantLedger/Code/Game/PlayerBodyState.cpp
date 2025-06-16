#include "PlayerBodyState.hpp"
#include "AnimStateEnum.hpp"

int PlayerBodyIdleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (conditions.at("isMoving")) {
		return static_cast<int>(PlayerBodyStates::WALK);
	}

	return static_cast<int>(PlayerBodyStates::IDLE);
}

int PlayerBodyWalkState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions)
{
	m_elapsedTime += deltaTime;

	if (!conditions.at("isMoving")) {
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::WALK);
}
