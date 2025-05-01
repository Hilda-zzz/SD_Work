#pragma once
#include "Game/Controller.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Timer.hpp"

class SpriteSheet;
class SpriteAnimDefinition;
class BitmapFont;

enum class PlayerMode
{
	FIRST_PLAYER,
	FREE_CAMERA,
	COUNT
};

class PlayerController : public Controller
{
public:
	PlayerController(Map* map);
	~PlayerController();

	void Update(float deltaSeconds) override;
	void UpdatePlayerInput(float deltaSeconds);
	void UpdateKBInputCam(float deltaSeconds);
	void UpdateControllerInputCam(float deltaSeconds,int controllerID);
	void UpdateKBInputPlayer(float deltaSeconds);

	void SetWeaponAnimDef();


	void UpdateControllerInputPlayer(float deltaSeconds, int controllerID);
	void UpdateCamera(float deltaSeconds);
	void UpdateHUD();

	void RenderPlayerHUD() const;

	void Possess(ActorHandle actorHandle, int viewPortType);
	Actor* GetActor() const;

	bool PlayWeaponAnimationHud(std::string name);

	void SetPlayerIndexAndControllerID(int playerIndex, int controllerIndex);
	int GetPlayerControllerID();

public:
	Camera m_playerCam;
	Camera m_hudCam;


private:
	int m_playerIndex = -1;
	int m_controllerID = -1;


	PlayerMode m_playerMode = PlayerMode::FIRST_PLAYER;

	Vec3 m_camPosition;
	EulerAngles m_camOrientation;
	Vec3 m_camVelocity;
	EulerAngles m_camAngularVelocity;

	float m_rollSpeed = 90.f;
	float m_yawPitchRatio = 0.075f;

	BitmapFont* m_font = nullptr;

	std::vector<Vertex_PCU> m_hudBaseVerts;
	std::vector<Vertex_PCU> m_hudWeaponVerts;
	std::vector<Vertex_PCU> m_rectileVerts;
	std::vector<Vertex_PCU> m_textVerts; 
	std::vector<Vertex_PCU> m_blackVerts;

	Timer m_weaponAnimTimer;
	SpriteSheet* m_curWeaponSheet=nullptr;
	SpriteAnimDefinition* m_weaponAnimDef = nullptr;

	bool m_isWalking = false;
};