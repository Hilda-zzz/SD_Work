#pragma once
#include "Game/AnimState.hpp"
template<typename StateEnum>
class StateMachine 
{
public:
	StateMachine() : m_currentState(nullptr), m_currentStateEnum(-1) {} 

	~StateMachine() 
	{
		for (auto& pair : m_states) {
			delete pair.second;
		}
	}

	void RegisterState(StateEnum stateEnum , AnimState* state) 
	{
		int enumIndex = static_cast<int>(stateEnum);
		auto it = m_states.find(enumIndex);
		if (it != m_states.end()) 
		{
			delete it->second;
		}

		m_states[enumIndex] = state;

		if (m_currentState == nullptr) 
		{
			SetCurrentStateInternal(enumIndex);
		}
	}

	void SetCurrentState(StateEnum stateEnum)
	{
		int enumIndex = static_cast<int>(stateEnum);
		auto it = m_states.find(enumIndex);
		if (it == m_states.end())
		{
			return;
		}

		if (m_currentState) {
			m_currentState->Exit();
		}

		m_currentState = it->second;
		m_currentStateEnum = enumIndex;
		m_currentState->Enter();
	}

	void Update(float deltaTime, const std::unordered_map<std::string, bool>& conditions) 
	{
		if (!m_currentState) return;

		int nextStateEnum = m_currentState->Update(deltaTime, conditions);
		if (nextStateEnum != m_currentStateEnum) {
			SetCurrentStateInternal(nextStateEnum);
		}
	}

	SpriteDefinition const& GetCurrentSprite() const 
	{
		if (m_currentState) 
		{
			return m_currentState->GetCurSprite();
		}
		//static SpriteDefinition defaultSpriteDef;
		//return defaultSpriteDef;
	}

	std::string GetCurrentStateName() const {
		if (m_currentState) {
			return m_currentState->GetName();
		}
		return "None";
	}

	StateEnum GetCurrentStateEnum() const {
		return static_cast<StateEnum>(m_currentStateEnum);
	}

	AnimState* GetCurrentState() const {
		return m_currentState;
	}
private:
	void SetCurrentStateInternal(int stateEnum) 
	{
		auto it = m_states.find(stateEnum);
		if (it == m_states.end())
		{
			return;
		}

		if (m_currentState) {
			m_currentState->Exit();
		}

		m_currentState = it->second;
		m_currentStateEnum = stateEnum;
		m_currentState->Enter();
	}

private:
	std::unordered_map<int, AnimState*> m_states;
	AnimState* m_currentState;
	int m_currentStateEnum;
};
