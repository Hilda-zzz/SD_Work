#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "ThirdParty/imgui/imgui.h"

class Clock;
class Prop;
class Player;
class Texture;
class Entity;
class CubeSkyBox;
class World;
class PiecewiseCurve1D;
class PlayerController;
class InputController_SpectatorMode;
class GameCamera;

// Physics constants
constexpr float GRAVITY = -20.0f;              // Units per second squared
constexpr float GROUND_CHECK_DISTANCE = 0.1f;
constexpr float MAX_FALL_SPEED = -50.0f;
constexpr float CORNER_OFFSET = 0.001f;

constexpr float GROUND_RAYCAST_OFFSET = 0.01f;  // 1cm
constexpr float GROUND_RAY_LENGTH = 0.02f;

constexpr float g_playerHeight = 1.8f;
constexpr float g_playerWidth = 0.6f;
constexpr float g_playerHorizontalGroundAcceleration = 64.0f;
constexpr float g_playerHorizontalFlyAcceleration = 32.0f;
constexpr float g_playerHorizontalAirAcceleration = 4.0f;
constexpr float g_playerHorizontalGroundDragCoefficient = 8.0f;
constexpr float g_playerHorizontalAirDragCoefficient = 4.0f;
constexpr float g_playerMaxHorizontalSpeed = 400.0f;
constexpr float g_playerMaxVerticalSpeed = 600.0f;
constexpr float g_playerGravityAcceleration = 20.f;
constexpr float g_playerJumpImpulse = 7.0f;
constexpr float g_playerEyeHeight = 1.65f;
constexpr float g_cameraOverShoulderDistance = 4.0f;


typedef std::vector<Entity*> EntityList;

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

	Vec2 GetScreenSize() { return m_screenSize; }

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);

	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion();

	void RenderAttractMode() const;
	void RenderGameplayMode() const;

	void RenderUI() const;
	void RenderDebugMode() const;
	void RenderTerrainEditor();

	void RenderSplineEditor(const char* label, PiecewiseCurve1D* spline,
		float inputMin, float inputMax,
		float outputMin, float outputMax,
		ImVec2 graphSize = ImVec2(300, 150));

	//------Process Control------
	void EnterState(GameState state);
	void EnterAttractMode();
	void EnterGameplayMode();

	void ExitState(GameState state);
	void ExitAttractMode();
	void ExitGameplayMode();

	//---------------------------
	void AddVertsForGroundGrid();
	void AddEntityToList(Entity& thisEntity, EntityList& list);

	void AddDebugText();
	

public:
	static GameState m_curGameState;
	static GameState m_nextGameState;

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

	World* m_world = nullptr;
	Player* m_player = nullptr;
	Prop* m_groundGrid = nullptr;

	PlayerController* m_playerController = nullptr;
	InputController_SpectatorMode* m_spectatorInputController = nullptr;
	GameCamera* m_gameCamera = nullptr;

	float m_previousTimeScale = 1.f;

	CubeSkyBox* m_cubeSkybox = nullptr;

	Vec2 m_screenSize = Vec2(1600.f, 800.f);

	bool m_showTerrainEditor = true;
};