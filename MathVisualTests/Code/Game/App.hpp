#pragma once
#include "Engine/Math/Vec2.hpp"

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
	void ChangeGameMode(unsigned char modeType);
public:
	bool m_isQuitting = false;
	
private:
	void Update(float deltaSeconds);
	void BeginFrame();
	void Render() const;
	void EndFrame();

private:
	Game* m_theGame;
	float m_timeLastFrameStart=0;
};