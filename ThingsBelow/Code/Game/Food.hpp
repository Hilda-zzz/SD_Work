#pragma once
#include "InteractiveObject.hpp"
#include "Engine/Core/Rgba8.hpp"

// ============================================================================
// Food - 食物类
// ============================================================================
// 可以被玩家拖动，松开后掉落
// 触手会响应掉落的食物并抓取
// ============================================================================

class Food : public InteractiveObject
{
public:
	Food(Vec2 const& position, float radius = 15.0f);
	virtual ~Food() = default;

	// 实现基类的纯虚函数
	//virtual void Update(float deltaSeconds);
	virtual void Render() const override;

	// 设置颜色（可选）
	void SetColor(Rgba8 const& color) { m_color = color; }
	Rgba8 GetColor() const { return m_color; }

	bool ShouldDelete() { return m_shouldDelete; }
	void MarkForDeletion() { m_shouldDelete = true; }

private:
	Rgba8 m_color;

	bool m_shouldDelete=false;
};
