#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
class Clock;
class ConvexScene;
class SceneEditor;

enum class GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_GAMEPLAY,
};

class Game
{
public:
	Game();
	~Game();

	void Update();
	void Renderer() const;

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);

	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

	void RenderAttractMode() const;
	void RenderGameplayMode() const;

	void RenderUI() const;
	void RenderDebugMode() const;

	//------Process Control------
	void EnterState(GameState state);
	void EnterAttractMode();
	void EnterGameplayMode();

	void ExitState(GameState state);
	void ExitAttractMode();
	void ExitGameplayMode();

public:
	static GameState m_curGameState;
	static GameState m_nextGameState;

	Clock* m_gameClock = nullptr;
	RandomNumberGenerator m_rng;

private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
	bool m_isDevConsole = false;

	//------Convex Scene Editor------
	ConvexScene* m_scene = nullptr;
	SceneEditor* m_editor = nullptr;
};