#pragma once
#include "Game/TentacleState.hpp"
#include "Game/TentacleStateType.hpp"
#include <map>

class Tentacle;

// TODO: Phase 3 - 性格系统
// struct TentaclePersonality;

class TentacleStateMachine
{
public:
	TentacleStateMachine(Tentacle* owner);
	~TentacleStateMachine();

	// Initialize with all state instances
	void Initialize();

	// TODO: Phase 3 - 根据性格初始化
	// void Initialize(const TentaclePersonality& personality);

	// Update current state
	void Update(float deltaSeconds);

	// Change to a new state
	void ChangeState(TentacleStateType newStateType);

	// Get current state type
	TentacleStateType GetCurrentStateType() const { return m_currentStateType; }

	// Get current state object (for debugging)
	TentacleState* GetCurrentState() const { return m_currentState; }
	const char* GetCurrentStateName() const;

private:
	// Register a state in the state map
	void RegisterState(TentacleStateType type, TentacleState* state);

	// Clean up all registered states
	void CleanupStates();

private:
	Tentacle* m_owner=nullptr;
	TentacleState* m_currentState;
	TentacleStateType m_currentStateType;
	std::map<TentacleStateType, TentacleState*> m_states;  // 这个触手专属的状态实例
};