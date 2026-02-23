#include "InteractiveObject.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "SceneBounds.hpp"

InteractiveObject::InteractiveObject(InteractiveObjectType type, Vec2 const& position, float radius)
	: m_type(type)
	, m_state(ObjectState::STATIC)
	, m_position(position)
	, m_velocity(Vec2::ZERO)
	, m_radius(radius)
	, m_gravity(500.0f)    // 默认重力
	, m_drag(0.98f)        // 默认阻力
{
}

void InteractiveObject::Update(float deltaSeconds)
{
	// 根据状态更新
	switch (m_state)
	{
	case ObjectState::HELD:
		// 被拖动时，速度为 0
		m_velocity = Vec2::ZERO;
		break;

	case ObjectState::DROPPING:
		// 掉落时应用物理
		ApplyPhysics(deltaSeconds);
		break;

	case ObjectState::STATIC:
	case ObjectState::CAPTURED:
		// 静止或被抓，不移动
		m_velocity = Vec2::ZERO;
		break;
	}

	CheckBoundaries();
}

void InteractiveObject::ApplyPhysics(float deltaSeconds)
{
	// 应用重力
	m_velocity.y -= m_gravity * deltaSeconds;
	
	// 应用阻力
	m_velocity *= m_drag;
	
	// 更新位置
	m_position += m_velocity * deltaSeconds;
}

void InteractiveObject::CheckBoundaries()
{
	// 如果边界未初始化，不检查
	if (!SceneBounds::IsInitialized())
		return;

	Vec2 bottomLeft = SceneBounds::GetBottomLeft();
	Vec2 topRight = SceneBounds::GetTopRight();

	// 检查底部边界
	if (m_position.y - m_radius < bottomLeft.y)
	{
		m_position.y = bottomLeft.y + m_radius;
		m_velocity.y = 0.0f;

		// 如果在掉落，碰到地面后变为静止
		if (m_state == ObjectState::DROPPING)
		{
			m_state = ObjectState::STATIC;
		}
	}

	// 检查左右边界
	if (m_position.x - m_radius < bottomLeft.x)
	{
		m_position.x = bottomLeft.x + m_radius;
		m_velocity.x = 0.0f;
	}
	else if (m_position.x + m_radius > topRight.x)
	{
		m_position.x = topRight.x - m_radius;
		m_velocity.x = 0.0f;
	}

	// 检查顶部边界（可选）
	if (m_position.y + m_radius > topRight.y)
	{
		m_position.y = topRight.y - m_radius;
		m_velocity.y = 0.0f;
	}
}