#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/Game.hpp"


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
	void ChangeGameMode(unsigned char modeType);
public:
	bool m_isQuitting = false;
	
private:
	void Update();
	void BeginFrame();
	void Render() const;
	void EndFrame();

private:
	Game* m_theGame;
	float m_timeLastFrameStart=0;
	GameMode m_curGameMode = GAME_MODE_PACHINKO2D;
};
bool OnQuitEvent(EventArgs& args);