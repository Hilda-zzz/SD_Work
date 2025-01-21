#pragma once
#include "Game/GameCommon.hpp"
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
	void Update(float deltaSeconds);
	void BeginFrame();
	void Render() const;
	void EndFrame();

	void LoadingGameConfig(char const* gameConfigXmlFilePath);
private:
	//Game* m_theGame;
	float m_timeLastFrameStart=0;
};
bool OnQuitEvent(EventArgs& args);