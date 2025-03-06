#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
class Clock;
class Prop;
class Player;
class Texture;
class Entity;

typedef std::vector<Entity*> EntityList;

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
	void AdjustForPauseAndTimeDitortion();
	void RenderAttractMode() const;
	void RenderUI() const;
	void RenderDebugMode() const;

	void AddVertsForGroundGrid();
	void AddVertsForCubes();
	void AddEntityToList(Entity& thisEntity, EntityList& list);
public:
	Clock* m_gameClock = nullptr;
	bool m_isAttractMode = true;
	bool m_isDevConsole = false;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;


private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;


	EntityList  m_allEntities;
	Player* m_player = nullptr;
	Prop* m_cube = nullptr;
	Prop* m_cube2 = nullptr;
	Prop* m_sphere = nullptr;
	Prop* m_groundGrid = nullptr;

	Texture* m_gridTex = nullptr;

	float m_previousTimeScale = 1.f;
};