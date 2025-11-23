#include "InputController_SpectatorMode.hpp"
#include "PlayerController.hpp"
#include "Player.hpp"
#include "GameCamera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"

extern Window* g_theWindow;

InputController_SpectatorMode::InputController_SpectatorMode(PlayerController* playerController)
	: m_playerController(playerController)
{
	m_curPlayer = m_playerController->GetPlayer();
	m_curGameCam = m_playerController->GetCamera();
}

void InputController_SpectatorMode::HandleKeyboardInput(float deltaTime)
{
	UNUSED(deltaTime);
	// Get current camera orientation to determine movement directions
	Vec3 fwdDirection, leftDirection, upDirection;
	EulerAngles camOrientation = m_curGameCam->GetOrientation();
	if (m_xyOnly)
	{
		// SPECTATOR_XY: Movement restricted to XY plane
		// Use only yaw for horizontal directions, ignore pitch
		EulerAngles xyOrientation(camOrientation.m_yawDegrees, 0.f, 0.f);
		xyOrientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	}
	else
	{
		// SPECTATOR_FULL: Full 3D movement based on camera orientation
		camOrientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	}

	// Calculate movement input vector
	Vec3 moveInput(0.f, 0.f, 0.f);

	// WASD movement
	if (g_theInput->IsKeyDown('W'))
	{
		moveInput += fwdDirection;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveInput -= fwdDirection;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveInput += leftDirection;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveInput -= leftDirection;
	}

	// Q/E for up/down movement (only in SPECTATOR_FULL mode)
	if (!m_xyOnly)
	{
		if (g_theInput->IsKeyDown('Q'))
		{
			moveInput += Vec3(0.f, 0.f, 1.f); // World up
		}
		if (g_theInput->IsKeyDown('E'))
		{
			moveInput += Vec3(0.f, 0.f, -1.f); // World down
		}
	}

	// Normalize movement direction if there's input
	if (moveInput.GetLengthSquared() > 0.0f)
	{
		moveInput.Normalized();
	}

	// Calculate movement speed
	// Get Player to access movement speed settings
	float moveSpeed = 4.0f; // Default spectator speed
	moveSpeed = m_curPlayer->m_moveSpeed;

	// Sprint modifier (Shift key)
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		moveSpeed *= m_curPlayer->m_sprintMultiplier;
	}

	// Apply velocity to camera
	// In Spectator mode, we directly set velocity (no acceleration, instant response)
	m_curGameCam->SetVelocity(moveInput * moveSpeed);
}

void InputController_SpectatorMode::HandleMouseInput(float deltaTime)
{
	EulerAngles newOrientation = m_curGameCam->GetOrientation();
	EulerAngles newAngularVelocity = m_curGameCam->GetAngularVelocity();

	float mouseSensitivity = 0.075f; // Default sensitivity
	mouseSensitivity = m_curPlayer->m_mouseSensitivity;

	newAngularVelocity.m_yawDegrees = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x;
	newAngularVelocity.m_pitchDegrees = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y;
	newAngularVelocity.m_yawDegrees *= mouseSensitivity;
	newAngularVelocity.m_pitchDegrees *= mouseSensitivity;

	float rollSpeed = 90.0f; // degrees per second
	if (g_theInput->IsKeyDown('Z'))
	{
		newOrientation.m_rollDegrees += rollSpeed * deltaTime;
	}
	if (g_theInput->IsKeyDown('X'))
	{
		newOrientation.m_rollDegrees -= rollSpeed * deltaTime;
	}

	newOrientation = newOrientation + newAngularVelocity;
	newOrientation.m_rollDegrees = GetClamped(newOrientation.m_rollDegrees, -45.f, 45.f);
	newOrientation.m_pitchDegrees = GetClamped(newOrientation.m_pitchDegrees, -85.f, 85.f);
	// ---------------------------------------------------------
	m_curGameCam->SetAngularVelocity(newAngularVelocity);
	m_curGameCam->SetOrientation(newOrientation);
}

void InputController_SpectatorMode::HandleControllerInput(float deltaTime)
{
	UNUSED(deltaTime);
	// TODO: Implement gamepad input handling
	// This will be implemented later when gamepad support is needed
}

void InputController_SpectatorMode::Update(float deltaTime)
{
	if (!m_playerController || !m_curPlayer || !m_curGameCam)
	{
		return;
	}
	HandleKeyboardInput(deltaTime);
	HandleMouseInput(deltaTime);
	HandleControllerInput(deltaTime);
}
