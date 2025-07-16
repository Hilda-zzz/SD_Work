#include "GameUISystem.hpp"
#include "Panel.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "../Core/ErrorWarningAssert.hpp"

GameUISystem::GameUISystem(int inputPriority, Renderer* curRenderer, Window* curWindow, InputSystem* curInputSystem)
	:IInputConsumer(inputPriority)
{
	m_curRenderer = curRenderer;
	m_curWindow = curWindow;
	m_curInputSystem = curInputSystem;
}

GameUISystem::~GameUISystem()
{
}

void GameUISystem::Update(float deltaTime)
{
	for (auto it = m_panelStack.rbegin(); it != m_panelStack.rend(); ++it)
	{
		Panel* child = *it;
		child->Update(deltaTime);
	}
}

// void GameUISystem::UpdateContinuousInput(float deltaTime)
// {
// 	Vec2 curMousePos= m_curWindow->GetMousePixelPos();
// 	if (curMousePos != m_lastMousePos)
// 	{
// 		InputEvent moveEvent;
// 		moveEvent.type = InputEventType::MOUSE_MOVE;
// 		moveEvent.mousePos = curMousePos;
// 		DispatchContinuousEvent(moveEvent);
// 		m_lastMousePos = curMousePos;
// 	}
// }

void GameUISystem::Render() const
{
	for (Panel* panel : m_panelStack)
	{
		panel->Render(m_curRenderer);
	}
}

bool GameUISystem::ConsumeInput(unsigned char keyCode)
{
	InputEvent event;
	event.type = InputEventType::KEY_PRESS;
	event.keyCode = keyCode;
	//event.mousePos = g_theWindow->GetMousePositionRaw();

	return DispatchDiscreteEvent(event);
}

bool GameUISystem::DispatchDiscreteEvent(InputEvent const& event)
{
	for (auto it = m_panelStack.rbegin(); it != m_panelStack.rend(); ++it)
	{
		Panel* panel = *it;
		if (panel && panel->IsVisible())
		{
			if (panel->ProcessInputHierarchy(event))
			{
				return true;
			}
		}
	}
	return false;
}

// bool GameUISystem::DispatchContinuousEvent(InputEvent const& event)
// {
// 	for (auto it = m_panelStack.rbegin(); it != m_panelStack.rend(); ++it)
// 	{
// 		Panel* panel = *it;
// 		if (panel && panel->IsVisible())
// 		{
// 			if (panel->ProcessInputHierarchy(event))
// 			{
// 				return true;
// 			}
// 		}
// 	}
// }

void GameUISystem::PushPanel(Panel* panel)
{
	if (!panel) return;

	m_panelStack.push_back(panel);
}

void GameUISystem::PopPanel(Panel* panel)
{
	if (!panel)
	{
		DebuggerPrintf("Warning: Null panel passed to PopPanel\n");
		return;
	}

	auto it = std::find(m_panelStack.begin(), m_panelStack.end(), panel);
	if (it != m_panelStack.end())
	{
		m_panelStack.erase(it);
		DebuggerPrintf("Panel found and removed from stack, remaining: %d\n",
			(int)m_panelStack.size());
	}
	else
	{
		DebuggerPrintf("Warning: Panel not found in stack\n");
	}
}

void GameUISystem::PopAllPanel()
{
	while (!m_panelStack.empty())
	{
		m_panelStack.pop_back();
	}
}

bool GameUISystem::IsKeyDown(unsigned char keyCode)
{
	return m_curInputSystem->IsKeyDownRaw(keyCode);
}

