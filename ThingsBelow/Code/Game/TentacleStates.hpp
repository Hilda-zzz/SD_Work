#pragma once
#include "Game/TentacleState.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//-----------------------------------------------------------------------------------------------
// IdleSwayState - 默认摆动状态
//-----------------------------------------------------------------------------------------------
class IdleSwayState : public TentacleState
{
public:
	IdleSwayState(Tentacle* owner, float swayRadius, float swaySpeed, unsigned int seedX, unsigned int seedY);

	virtual void OnEnter() override;
	virtual void Update(float deltaSeconds) override;
	virtual void OnExit() override;
	virtual const char* GetName() const override { return "IdleSway"; }

private:
	void GenerateNewTarget();  // 生成新的随机目标点

private:
	float m_swayTime;
	float m_swayRadius;
	float m_swaySpeed;
	unsigned int m_noiseSeedX;
	unsigned int m_noiseSeedY;

	Vec2 m_currentTarget;      // 当前正在移动的目标
	Vec2 m_nextTarget;         // 下一个目标
	float m_targetChangeTimer; // 切换计时器
	float m_targetChangeDuration; // 多久换一次目标

	RandomNumberGenerator m_rng;
};
//-----------------------------------------------------------------------------------------------
// TrackTargetState - 跟随目标点
//-----------------------------------------------------------------------------------------------
class TrackTargetState : public TentacleState
{
public:
	TrackTargetState(Tentacle* owner);

	virtual void OnEnter() override;
	virtual void Update(float deltaSeconds) override;
	virtual void OnExit() override;
	virtual const char* GetName() const override { return "TrackTarget"; }

	// Configuration
	void SetLerpSpeed(float speed) { m_lerpSpeed = speed; }
	float GetLerpSpeed() const { return m_lerpSpeed; }

private:
	Vec2 m_smoothTarget;    // Internal smooth target (lerped)
	float m_lerpSpeed;      // How fast to lerp (default: 5.0)
};