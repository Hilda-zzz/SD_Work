#include "Game/TentacleStateMachine.hpp"
#include "Game/Tentacle.hpp"
#include "Game/TentacleStates.hpp"
#include "LungeState.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

TentacleStateMachine::TentacleStateMachine(Tentacle* owner)
	: m_owner(owner)
	, m_currentState(nullptr)
	, m_currentStateType(TentacleStateType::IDLE)
{
}

TentacleStateMachine::~TentacleStateMachine()
{
	CleanupStates();
}

void TentacleStateMachine::Initialize()
{
	RandomNumberGenerator rng;

	float swayRadius = 30.0f;  // 绝对值（单位）
	float swaySpeed = 0.3f;
	unsigned int seedX = rng.RollRandomIntInRange(1,10000);
	unsigned int seedY = rng.RollRandomIntInRange(1, 10000);

	RegisterState(TentacleStateType::IDLE, new IdleSwayState(m_owner, swayRadius, swaySpeed, seedX, seedY));
	RegisterState(TentacleStateType::TRACK_TARGET, new TrackTargetState(m_owner));
	RegisterState(TentacleStateType::LUNGE, new LungeState(m_owner));

	ChangeState(TentacleStateType::IDLE);
}

void TentacleStateMachine::Update(float deltaSeconds)
{
	if (m_currentState != nullptr)
	{
		m_currentState->Update(deltaSeconds);
	}
}

void TentacleStateMachine::ChangeState(TentacleStateType newStateType)
{
	// Exit current state
	if (m_currentState != nullptr)
	{
		m_currentState->OnExit();
	}

	// Change to new state
	m_currentStateType = newStateType;
	auto it = m_states.find(newStateType);

	if (it != m_states.end())
	{
		m_currentState = it->second;
		m_currentState->OnEnter();
	}
	else
	{
		m_currentState = nullptr;
		// TODO: Error handling - State not found
	}
}

const char* TentacleStateMachine::GetCurrentStateName() const
{
	if (m_currentState != nullptr)
	{
		return m_currentState->GetName();
	}
	return "None";
}

void TentacleStateMachine::RegisterState(TentacleStateType type, TentacleState* state)
{
	m_states[type] = state;
}

void TentacleStateMachine::CleanupStates()
{
	// Exit current state before cleanup
	if (m_currentState != nullptr)
	{
		m_currentState->OnExit();
		m_currentState = nullptr;
	}

	// Delete all state instances
	for (auto& pair : m_states)
	{
		delete pair.second;
	}
	m_states.clear();
}