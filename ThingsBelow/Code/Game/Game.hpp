#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
class Clock;
class Tentacle;

class Tentacle;
class TentacleEditPanel;  

class TentacleManager;
class Food;

enum class GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_GAMEPLAY,
	GAME_STATE_TENTACLE_TEST,
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
	void UpdateTentacleTest(float deltaTime);

	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

	void RenderAttractMode() const;
	void RenderGameplayMode() const;
	void RenderTentacleTest() const;

	void RenderUI() const;
	void RenderDebugMode() const;

	//------Process Control------
	void EnterState(GameState state);
	void EnterAttractMode();
	void EnterGameplayMode();
	void EnterTentacleTest();

	void ExitState(GameState state);
	void ExitAttractMode();
	void ExitGameplayMode();
	void ExitTentacleTest();

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

	// Tentacle Test
	//Tentacle* m_testTentacle = nullptr;
	TentacleEditPanel* m_tentacleDebugPanel = nullptr; 

	// Tentacle system
	TentacleManager* m_tentacleManager;

	// Interactive objects
	std::vector<Food*> m_foods;
	Food* m_heldFood;  // 当前被拖动的食物
};