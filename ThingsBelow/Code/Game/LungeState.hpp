#pragma once
#include "TentacleState.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

class Food;

// ============================================================================
// LungePhase - Lunge 状态的内部阶段
// ============================================================================
enum class LungePhase
{
	EXTEND,    // 伸长阶段 - 动态增加长度，伸向目标
	CONTACT,   // 接触阶段 - 尖端贴在食物上保持一段时间
	RETRACT    // 收回阶段 - 动态缩短长度，带着食物回到根部
};

// ============================================================================
// LungeState - 突刺抓取状态
// ============================================================================
// 用于触手突刺抓取掉落的食物
// 
// 三个阶段：
// 1. EXTEND: 动态伸长，尖端快速接近食物
// 2. CONTACT: 尖端贴在食物上保持 0.2-0.3 秒
// 3. RETRACT: 动态缩短，食物跟随尖端回到根部附近
// 
// 特点：
// - 长度是逐帧动态改变的，不是瞬间改变
// - 食物在 CONTACT 后被标记为 CAPTURED
// - 食物在 RETRACT 阶段跟随触手尖端移动
// - 进入收回半径范围后计时一段时间才算完成
// ============================================================================

class LungeState : public TentacleState
{
public:
	LungeState(Tentacle* owner);

	virtual void OnEnter() override;
	virtual void Update(float deltaSeconds) override;
	virtual void OnExit() override;
	virtual const char* GetName() const override { return "Lunge"; }

	// ========================================================================
	// 参数配置（可在 ImGui 中调试）
	// ========================================================================
	
	void SetExtendSpeed(float speed) { m_extendSpeed = speed; }
	float GetExtendSpeed() const { return m_extendSpeed; }
	
	void SetRetractSpeed(float speed) { m_retractSpeed = speed; }
	float GetRetractSpeed() const { return m_retractSpeed; }
	
	void SetContactDuration(float duration) { m_contactDuration = duration; }
	float GetContactDuration() const { return m_contactDuration; }
	
	void SetReachThreshold(float threshold) { m_reachThreshold = threshold; }
	float GetReachThreshold() const { return m_reachThreshold; }
	
	void SetRetractRadius(float radius) { m_retractRadius = radius; }
	float GetRetractRadius() const { return m_retractRadius; }
	
	void SetRetractCompleteDuration(float duration) { m_retractCompleteDuration = duration; }
	float GetRetractCompleteDuration() const { return m_retractCompleteDuration; }

	// 获取当前阶段
	LungePhase GetCurrentPhase() const { return m_currentPhase; }

private:
	// ========================================================================
	// 更新各阶段
	// ========================================================================
	
	void UpdateExtend(float deltaSeconds);
	void UpdateContact(float deltaSeconds);
	void UpdateRetract(float deltaSeconds);

private:
	// 当前阶段
	LungePhase m_currentPhase;

	// 目标数据
	Vec2 m_targetPos;           // 食物位置
	Food* m_targetFood;         // 食物指针

	// Extend phase progress
	float m_extendProgress;      // 伸长进度（0.0 ~ 1.0）
	float m_targetLength;        // 目标长度（需要伸到的长度）

	// 长度控制
	float m_originalLength;     // 原始长度
	float m_currentLength;      // 当前长度

	// 计时器
	float m_contactTimer;       // 接触阶段计时
	float m_retractTimer;       // 收回区域停留计时
	bool m_insideRetractZone;   // 是否在收回区域内

	// 参数（可调）
	float m_extendSpeed;                  // 伸长速度 (units/s)
	float m_retractSpeed;                 // 收回速度 (units/s)
	float m_contactDuration;              // 接触持续时间 (s)
	float m_reachThreshold;               // 到达判定距离 (units)
	float m_retractRadius;                // 收回半径范围 (units)
	float m_retractCompleteDuration;      // 在收回区域停留多久算完成 (s)

	float m_retractStartLength;

	Vec2 m_retractTarget;       // 收回目标位置（终点）
	Vec2 m_retractStartPos;     // 收回起始位置（进入 Retract 时的尖端位置）
	float m_retractProgress;    // 收回进度（0.0 ~ 1.0）

	RandomNumberGenerator m_rng;
};
