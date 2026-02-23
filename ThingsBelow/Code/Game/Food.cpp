#include "Food.hpp"
#include "GameCommon.hpp"
#include <Engine/Math/MathUtils.hpp>

Food::Food(Vec2 const& position, float radius)
	: InteractiveObject(InteractiveObjectType::FOOD, position, radius)
	, m_color(Rgba8(255, 200, 100))  // 默认橙黄色
{
	// 食物特定的物理参数
	m_gravity = 400.0f;   // 稍轻一些
	m_drag = 0.99f;       // 较小的阻力
}

//void Food::Update(float deltaSeconds)
//{
//}

void Food::Render() const
{
	// 根据状态绘制不同的外观
	Rgba8 renderColor = m_color;
	
	switch (m_state)
	{
	case ObjectState::HELD:
		// 被拖动时稍微亮一些
		renderColor = Rgba8(
			GetClamped(m_color.r + 30, 0, 255),
			GetClamped(m_color.g + 30, 0, 255),
			GetClamped(m_color.b + 30, 0, 255)
		);
		break;

	case ObjectState::DROPPING:
		// 掉落时正常颜色
		break;

	case ObjectState::CAPTURED:
		// 被抓住时变暗
		renderColor = Rgba8(
			m_color.r / 2,
			m_color.g / 2,
			m_color.b / 2
		);
		break;

	case ObjectState::STATIC:
		// 静止时正常颜色
		break;
	}

	// 绘制圆形食物
	DebugDrawCircle(m_radius, m_position, renderColor);
	
	// 绘制边缘（增强视觉效果）
	DebugDrawRing(2.0f, m_radius - 2.0f, Rgba8(renderColor.r / 2, renderColor.g / 2, renderColor.b / 2), m_position);
}
