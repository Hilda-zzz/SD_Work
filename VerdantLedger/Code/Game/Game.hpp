#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"

class Clock;
class Map;
class TileMapManager;
class Player;

enum class GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_MENU,
	GAME_STATE_SAVELOAD,
	GAME_STATE_SETTINGS,
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
	//------Update--------------
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);

	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

	//------Render--------------
	void RenderAttractMode() const;
	void RenderGameplayMode() const;

	void RenderGameplayUI() const;
	void RenderDebugMode() const;

	//------Process Control------
	void EnterState(GameState state);
	void ExitState(GameState state);

public:
	Clock* m_gameClock = nullptr;
	RandomNumberGenerator m_rng;

	GameState m_curGameState = GameState::GAME_STATE_ATTRACT;
	GameState m_nextGameState = GameState::GAME_STATE_ATTRACT;
	
	Map* m_curMap = nullptr;
	Player* m_player = nullptr;
	TileMapManager* g_tileManager;
private:
	
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
	bool m_isDevConsole = false;
};