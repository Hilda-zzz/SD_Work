#pragma once
#include "Engine/Input/IInputConsumer.hpp"
#include <vector>
#include "../Input/InputSystem.hpp"

class Renderer;
class Panel;
class Window;

class GameUISystem: public IInputConsumer
{
public:
	GameUISystem(int inputPriority, Renderer* curRenderer, Window* curWindow,InputSystem* curInputSystem);
	~GameUISystem();

	void Update(float deltaTime);
	void UpdateContinuousInput(float deltaTime);
	void Render() const;

	bool ConsumeInput(unsigned char keyCode) override;
	bool DispatchDiscreteEvent(InputEvent const& event);
	//bool DispatchContinuousEvent(InputEvent const& event);

	void PushPanel(Panel* panel);
	void PopPanel(Panel* panel);
	void PopAllPanel();

	bool IsKeyDown(unsigned char keyCode);

private:
	Renderer* m_curRenderer = nullptr;
	Window* m_curWindow = nullptr;
	InputSystem* m_curInputSystem = nullptr;
	std::vector<Panel*> m_panelStack;

	Vec2 m_lastMousePos;
};