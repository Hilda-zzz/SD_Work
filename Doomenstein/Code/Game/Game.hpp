#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
class Clock;
class Player;
class Texture;
class Entity;
class Map;
typedef std::vector<Entity*> EntityList;

enum class GameState
{
	NONE,
	ATTRACT,
	LOBBY,
	PLAYING,
	COUNT
};


class Game
{
public:
	Game();
	~Game();

	void Update();
	void Renderer() const;

	void EnterState(GameState state);
	void ExitState(GameState state);

	void EnterAttractMode();
	void EnterPlayingMode();

	void ExitPlayingMode();

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);
	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion();
	void RenderAttractMode() const;
	void RenderPlayingModeHUD() const;
	void RenderDebugMode() const;

	//void AddVertsForGroundGrid();
	//void AddVertsForCubes();
	//void AddEntityToList(Entity& thisEntity, EntityList& list);
public:
	Clock* m_gameClock = nullptr;
	bool m_isAttractMode = true;
	bool m_isDevConsole = false;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;

private:
	GameState m_curGameState = GameState::ATTRACT;
	GameState m_nextState = GameState::ATTRACT;

	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;

	Player* m_player = nullptr;
	Texture* m_gridTex = nullptr;

	float m_previousTimeScale = 1.f;

	//------------------------
	std::vector<Map*> m_maps;
	Map* m_curMap = nullptr;
	int m_curMapIndex = 0;
};