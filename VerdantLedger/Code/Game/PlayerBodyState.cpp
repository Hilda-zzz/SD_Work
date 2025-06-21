#include "PlayerBodyState.hpp"
#include "AnimStateEnum.hpp"
#include "Engine/Core/EngineCommon.hpp"

int PlayerBodyIdleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;
	// moving first
	if (conditions.at("isMoving")) {
		return static_cast<int>(PlayerBodyStates::WALK);
	}
	// cannot use tools when moving
	if (conditions.at("holdTool"))
	{
		if (conditions.at("usingAxe"))
		{
			return static_cast<int>(PlayerBodyStates::AXE);
		}
		else if (conditions.at("usingHoe"))
		{
			return static_cast<int>(PlayerBodyStates::HOE);
		}
		else if (conditions.at("usingPickaxe"))
		{
			return static_cast<int>(PlayerBodyStates::PICKAXE);
		}
		else if (conditions.at("usingShovel"))
		{
			return static_cast<int>(PlayerBodyStates::SHOVEL);
		}
		else if (conditions.at("usingSickle"))
		{
			return static_cast<int>(PlayerBodyStates::SICKLE);
		}
		else if (conditions.at("usingWater"))
		{
			return static_cast<int>(PlayerBodyStates::WATER);
		}
	}
	return static_cast<int>(PlayerBodyStates::IDLE);
}

int PlayerBodyWalkState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (!conditions.at("isMoving")) {
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::WALK);
}

int PlayerBodyRunState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(deltaTime);
	UNUSED(conditions);
	UNUSED(curDirection);
	return 0;
}

int PlayerBodyAxeState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::AXE);
}

int PlayerBodyHoeState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::HOE);
}

int PlayerBodyPickaxeState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::PICKAXE);
}

int PlayerBodyShovelState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::SHOVEL);
}

int PlayerBodySickleState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::SICKLE);
}

int PlayerBodyWaterState::Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions, Direction curDirection)
{
	UNUSED(conditions);
	m_elapsedTime += deltaTime;

	m_curDirection = curDirection;

	if (m_directionalAnims->at(m_curDirection)->IsPlayOnceFinished(m_elapsedTime))
	{
		return static_cast<int>(PlayerBodyStates::IDLE);
	}
	return static_cast<int>(PlayerBodyStates::WATER);
}
