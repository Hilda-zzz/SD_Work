#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
class Game;

class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void RunFrame();
	void RunMainLoop();
	void HandleQuitRequested();

	void SetCursorMode(CursorMode cursorMode);

public:
	bool m_isQuitting = false;
	
private:
	void Update();
	void BeginFrame();
	void Render() const;
	void EndFrame();

private:
	float m_timeLastFrameStart=0;
};
bool OnQuitEvent(EventArgs& args);