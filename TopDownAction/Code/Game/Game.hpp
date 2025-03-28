#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
class Clock;
class Player;
class Map;
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
	void RenderGameMode() const;
	void RenderUI() const;
	void RenderDebugMode() const;

public:
	Clock* m_gameClock = nullptr;
	bool m_isAttractMode = true;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;

private:
	Camera m_screenCamera;
	Camera m_gameCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;

	Player* m_player = nullptr;

	std::vector<Map*> m_maps;
	Map* m_curMap = nullptr;
};