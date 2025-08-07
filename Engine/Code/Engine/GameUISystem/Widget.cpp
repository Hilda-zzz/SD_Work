#include "Widget.hpp"
#include "../Core/EngineCommon.hpp"

void Widget::Update(float deltaTime)
{
	UpdateSelf(deltaTime);
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
	{
		Widget* child = *it;
		child->Update(deltaTime);
	}
}

void Widget::UpdateSelf(float deltaTime)
{
	UNUSED(deltaTime);
}

void Widget::Render(Renderer* renderer) const
{
	RenderSelf(renderer);

	for (Widget* child : m_children) 
	{
		if (child && child->IsActive()) 
		{
			child->Render(renderer);
		}
	}
}

void Widget::RenderSelf(Renderer* renderer) const
{
	UNUSED(renderer);
}

bool Widget::HandleInput(InputEvent const& event)
{
	UNUSED(event);
	return false;
}

bool Widget::AddChild(Widget* childWidget)
{
	if (!childWidget) return false;

	if (childWidget->m_parent)
	{
		childWidget->m_parent->RemoveChild(childWidget);
	}

	m_children.push_back(childWidget);
	childWidget->m_parent = this;
	return true;
}

bool Widget::RemoveChild(Widget* childWidget)
{
	if (!childWidget) return false;

	auto it = std::find(m_children.begin(), m_children.end(), childWidget);

	if (it != m_children.end()) 
	{
		childWidget->m_parent = nullptr;
		m_children.erase(it);
		return true;
	}

	return false;
}

bool Widget::ProcessInputHierarchy(InputEvent const& event)
{
	if (!m_isActive || !m_canReceiveInput) 
	{
		return false;
	}
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) 
	{
		Widget* child = *it;
		if (child->ProcessInputHierarchy(event))
		{
			return true; 
		}
	}
	return HandleInput(event);
}

// bool Widget::ContainsPoint(Vec2 const& point) const
// {
// 	return false;
// }
