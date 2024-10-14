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

	bool IsQuitting() const { return m_isQuitting; }
	bool IsPause() const { return m_isPause; }
	bool IsSlow() const { return m_isSlow; }
	bool IsPauseAfterUpdate() const { return m_pauseAfterUpdate; }

	void HandleKeyPressed(unsigned char keyCode);
	void HandleKeyReleased(unsigned char keyCode);
	void HandleQuitRequested();
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

public:
	bool m_isQuitting = false;
	
private:
	void Update(float deltaSeconds);
	void BeginFrame();
	void Render() const;
	void EndFrame();

private:
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
	Vec2 m_shipPos{ 0.0f, 50.f };
	Game* m_theGame;
	float m_timeLastFrameStart=0;
};