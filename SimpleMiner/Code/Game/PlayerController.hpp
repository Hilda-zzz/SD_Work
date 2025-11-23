#pragma once
#include "GameCamera.hpp"

class Player;
class InputController;
class InputController_SpectatorMode;
class InputController_PlayerMode;

class PlayerController 
{
public:
	PlayerController(Player* player, GameCamera* gameCamera);
	~PlayerController();

	void UpdateInput(float deltaTime);
	void CycleCameraMode();
	void CyclePlayerPhysicsMode();

	// ****************************************************

	void SetInputController(InputController* inputController);

	void SetCameraMode(CameraMode mode); 
	void SetPlayerPhysicsMode(PhysicsMode mode);

	Player* GetPlayer() { return m_player; }
	GameCamera* GetCamera() { return m_camera; }
private:
	Player* m_player = nullptr;
	GameCamera* m_camera = nullptr;
	InputController* m_currentInputController = nullptr;

	InputController_SpectatorMode* m_spectatorController = nullptr;
	InputController_PlayerMode* m_playerModeController = nullptr;
};