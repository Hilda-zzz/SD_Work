#pragma once

class Tentacle;  // 前置声明

class TentacleState
{
protected:
	Tentacle* m_owner;

public:
	TentacleState(Tentacle* owner);
	virtual ~TentacleState() {}

	// 核心接口
	virtual void OnEnter() = 0;
	virtual void Update(float deltaSeconds) = 0;
	virtual void OnExit() = 0;

	// 过渡建议
	virtual float GetExitSpeed() const { return 150.0f; }
	virtual bool UseSplineExit() const { return false; }

	// 调试
	virtual const char* GetName() const = 0;
};