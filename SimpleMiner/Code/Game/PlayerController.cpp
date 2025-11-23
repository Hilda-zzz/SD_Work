#include "PlayerController.hpp"
#include "Game/Player.hpp"
#include "Game/GameCamera.hpp"
#include "InputController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "InputController_SpectatorMode.hpp"
#include "InputController_PlayerMode.hpp"

extern InputSystem* g_theInput;

PlayerController::PlayerController(Player* player, GameCamera* gameCamera)
	:m_player(player),m_camera(gameCamera)
{
	m_spectatorController= new InputController_SpectatorMode(this);
	m_playerModeController = new InputController_PlayerMode(this);
}

PlayerController::~PlayerController()
{
}

void PlayerController::SetInputController(InputController* inputController)
{
	m_currentInputController = inputController;
}

void PlayerController::SetCameraMode(CameraMode mode)
{
	switch (mode) 
	{
	case CameraMode::FIRST_PERSON:
	case CameraMode::OVER_THE_SHOULDER:
		m_currentInputController = m_playerModeController;
		break;
	case CameraMode::SPECTATOR_FULL:
		m_spectatorController->SetIsXY(false);
		m_currentInputController = m_spectatorController;
		break;
	case CameraMode::SPECTATOR_XY:
		m_spectatorController->SetIsXY(true);
		m_currentInputController = m_spectatorController;
		break;
	case CameraMode::INDEPENDENT:
		m_currentInputController = m_playerModeController;
		break;
	}
	m_camera->SetMode(mode);
}

void PlayerController::SetPlayerPhysicsMode(PhysicsMode mode)
{
	m_player->SetPhysicsMode(mode);
}

void PlayerController::UpdateInput(float deltaTime)
{
	if(g_theInput->WasKeyJustPressed('C'))
		CycleCameraMode();
	if (g_theInput->WasKeyJustPressed('V'))
		CyclePlayerPhysicsMode();
	// Block selection
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_player->m_curBlockBrushName = "Glowstone";
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_player->m_curBlockBrushName = "Cobblestone";
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_player->m_curBlockBrushName = "ChiseledBrick";
	}

	m_currentInputController->Update(deltaTime);
}

void PlayerController::CycleCameraMode()
{
	int currentMode = (int)m_camera->GetMode();
	currentMode = (currentMode + 1) % (int)(CameraMode::COUNT);
	SetCameraMode((CameraMode)currentMode);
}

void PlayerController::CyclePlayerPhysicsMode()
{
	int currentMode = (int)m_player->GetPhysicsMode();
	currentMode = (currentMode + 1) % 3;
	SetPlayerPhysicsMode((PhysicsMode)currentMode);
}
