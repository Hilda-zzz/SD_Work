#pragma once
#include "Game/Entity.hpp"
#include <string>

class World;
class GameCamera;

class Player : public Entity
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds) override;
	//void Render() const override;

	// Input handling (Player-specific)
	//void UpdateInput(float deltaSeconds);
	//void UpdateKBInput(float deltaSeconds);
	//void UpdateControllerInput(float deltaSeconds);
	//void HandleGameplayInput();

	// Movement (Player-specific actions)
	//void Jump();
	bool CanJump() const;

	// Camera control
	void SetGameCamera(GameCamera* camera) { m_gameCamera = camera; }
	GameCamera* GetGameCamera() { return m_gameCamera; }
	//void ProcessCameraInput(float deltaSeconds);

	void SetEyesightOrientation(EulerAngles eye);
	EulerAngles const& GetEyesightOrientation() const { return m_eyeSightOrientation; };
private:
	//bool IsPlayerPossessed() const;
	//void HandleDispossessedMode(float deltaSeconds);
	//void UpdateIndependentModeInput(float deltaSeconds);
public:
	// Camera reference (Player controls the camera but doesn't own it)
	GameCamera* m_gameCamera = nullptr;

	// Movement parameters
	float m_moveSpeed = 4.0f;              // Normal walking speed (m/s)
	float m_sprintMultiplier = 2.5f;       // Sprint speed multiplier
	float m_jumpSpeed = 8.0f;              // Initial jump velocity (m/s)

	// Input sensitivity
	float m_mouseSensitivity = 0.075f;
	float m_gamepadSensitivity = 180.0f;

	// Block interaction
	std::string m_curBlockBrushName = "Glowstone";

	// Legacy spectator mode (for backward compatibility with existing code)
	bool m_isSpectatorMode = false;
	bool m_isSpectatorFull = true;         // Full freedom vs XY-only
	float m_spectatorSpeed = 20.0f;
	float m_rollSpeed = 90.0f;
	float m_yawSpeed = 50.0f;
	float m_pitchSpeed = 50.0f;

	EulerAngles m_eyeSightOrientation;
};