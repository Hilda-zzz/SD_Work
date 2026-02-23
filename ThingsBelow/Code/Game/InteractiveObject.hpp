#pragma once
#include "Engine/Math/Vec2.hpp"

// ============================================================================
// InteractiveObject - 可交互物体基类
// ============================================================================
// 所有能被触手交互的物体的基类
// 包括：Food（食物）、LightBall（光球）等
// ============================================================================

enum class InteractiveObjectType
{
	FOOD,
	LIGHT_BALL,
	// TODO: 更多类型
};

enum class ObjectState
{
	HELD,       // 被玩家拖动
	DROPPING,   // 掉落中
	STATIC,     // 静止
	CAPTURED    // 被触手抓住
};

class InteractiveObject
{
public:
	InteractiveObject(InteractiveObjectType type, Vec2 const& position, float radius);
	virtual ~InteractiveObject() = default;

	// ========================================================================
	// 核心更新和渲染
	// ========================================================================
	virtual void Update(float deltaSeconds);
	virtual void Render() const = 0;  // 纯虚函数，子类必须实现

	// ========================================================================
	// 状态管理
	// ========================================================================
	void SetState(ObjectState state) { m_state = state; }
	ObjectState GetState() const { return m_state; }
	
	bool IsHeld() const { return m_state == ObjectState::HELD; }
	bool IsDropping() const { return m_state == ObjectState::DROPPING; }
	bool IsStatic() const { return m_state == ObjectState::STATIC; }
	bool IsCaptured() const { return m_state == ObjectState::CAPTURED; }

	// ========================================================================
	// 位置和物理
	// ========================================================================
	void SetPosition(Vec2 const& pos) { m_position = pos; }
	Vec2 GetPosition() const { return m_position; }
	
	void SetVelocity(Vec2 const& vel) { m_velocity = vel; }
	Vec2 GetVelocity() const { return m_velocity; }
	
	float GetRadius() const { return m_radius; }

	// ========================================================================
	// 类型查询
	// ========================================================================
	InteractiveObjectType GetType() const { return m_type; }
	bool IsFood() const { return m_type == InteractiveObjectType::FOOD; }
	bool IsLightBall() const { return m_type == InteractiveObjectType::LIGHT_BALL; }

protected:
	// 应用重力和碰撞（子类可重写）
	virtual void ApplyPhysics(float deltaSeconds);
	virtual void CheckBoundaries();

protected:
	InteractiveObjectType m_type;
	ObjectState m_state;
	
	Vec2 m_position;
	Vec2 m_velocity;
	float m_radius;
	
	// 物理参数
	float m_gravity;      // 重力加速度
	float m_drag;         // 空气阻力
};
