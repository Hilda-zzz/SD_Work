#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/HerringboneMapGenerator.hpp"
class Clock;
class SandboxMap;
class SandboxPlayer;
class WangTileMap;
class Image;
class HerringboneTileset;
class Nova2DTestMap;

enum class GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_GAMEPLAY,
};

enum class GameMode
{
	SANDBOX,
	WANG_TILE_MAP,
	NOVA2D_TEST
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

	void UpdateDeveloperCheats(float& deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

	void RenderAttractMode() const;
	void RenderGameplayMode() const;

	void RenderGameModeSelectionUI() const;
	void RenderUI() const;
	void RenderDebugMode() const;

	//------Process Control------
	void EnterState(GameState state);
	void EnterAttractMode();
	void EnterGameplayMode();

	void ExitState(GameState state);
	void ExitAttractMode();
	void ExitGameplayMode();

	// --------------------------
	void DebugDrawCurrentTile() const;
	void RenderTileDebugUI();

public:
	static GameState m_curGameState;
	static GameState m_nextGameState;

	Clock* m_gameClock = nullptr;
	static RandomNumberGenerator s_rng;

private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
	bool m_isDevConsole = false;

	SandboxMap* m_sandboxMap = nullptr;
	SandboxPlayer* m_sandboxPlayer = nullptr;

	WangTileMap* m_wangTileMap = nullptr;

	float m_curDeltaTime = 0.f;

	GameMode m_selectedGameMode = GameMode::SANDBOX;

	Image* m_testUtilImg = nullptr;

	HerringboneTileset* m_testTileset = nullptr;
	int m_testTileIndex = 0;
	HerringboneMapGenerator m_generator;

	Nova2DTestMap* m_nova2DMap = nullptr;
};