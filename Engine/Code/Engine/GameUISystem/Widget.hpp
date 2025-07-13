#pragma once
#include <vector>
#include "../Math/Vec2.hpp"
#include "../Math/AABB2.hpp"
#include "../Input/InputSystem.hpp"
class Renderer;
class Window;
class GameUISystem;

class Widget
{
public: 
	//Widget();
	virtual ~Widget()=default;

	virtual void Update(float deltaTime);
	virtual void UpdateSelf(float deltaTime);
	virtual void Render(Renderer* renderer) const;
	virtual void RenderSelf(Renderer* renderer) const;
	virtual bool HandleInput(InputEvent const& event);

	bool AddChild(Widget* childWidget);
	bool RemoveChild(Widget* childWidget);

	void SetVisible(bool visible) { m_isVisible = visible; }
	bool IsVisible() const { return m_isVisible; }

	bool ProcessInputHierarchy(InputEvent const& event);
	bool ContainsPoint(Vec2 const& point) const;

protected:
	std::vector<Widget*> m_children;
	Widget* m_parent = nullptr;
	bool m_isVisible = true;
	bool m_canReceiveInput = true;
	Vec2 m_position;
	AABB2 m_bounds;

	GameUISystem* m_curUISystem = nullptr;
	Renderer* m_curRenderer = nullptr;
	Window* m_curWindow = nullptr;
};