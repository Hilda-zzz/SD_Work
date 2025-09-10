#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"
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

public:
	bool m_isQuitting = false;
	
private:
	void Update();
	void BeginFrame();
	void Render() const;
	void EndFrame();
	void LoadingGameConfig(std::string const& filePath);

private:
	float m_timeLastFrameStart=0;

	float m_windowAspect = 2.f;
	bool m_windowFullscreen = false;
	std::string m_windowTitle = "Simple Miner A01";
};
bool OnQuitEvent(EventArgs& args);