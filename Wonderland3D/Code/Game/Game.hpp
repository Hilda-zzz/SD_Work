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
class CubeSkyBox;
class Shader;

typedef std::vector<Entity*> EntityList;

constexpr int MATERIAL_MATRIX_ROWS = 6;
constexpr int MATERIAL_MATRIX_COLS = 6;
constexpr float SPHERE_SPACING = 2.5f;

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

	void InitializeMaterialMatrix();
public:
	Clock* m_gameClock = nullptr;
	bool m_isAttractMode = true;
	bool m_isDevConsole = false;
	bool isDebugMode = false;
	RandomNumberGenerator m_rng;
	Shader* m_pbrShader = nullptr;

private:
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;


	EntityList  m_allEntities;
	Player* m_player = nullptr;
	//Prop* m_cube = nullptr;
	//Prop* m_cube2 = nullptr;
	//Prop* m_sphere = nullptr;
	Prop* m_groundGrid = nullptr;

	Texture* m_gridTex = nullptr;

	float m_previousTimeScale = 1.f;

	CubeSkyBox* m_cubeSkybox = nullptr;

	// pbr material sphere
	std::vector<Prop*> m_materialSpheres;

	//Light
	Vec3 m_sunDirection = Vec3(1.f, 1.f, -1.f);
	float m_sunIntensity = 1.f;
	float m_ambientIntensity = 0.4f;
};