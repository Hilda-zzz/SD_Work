#pragma once
#include "InputController.hpp"

class Player;
class GameCamera;
class PlayerController;

class InputController_PlayerMode : public InputController 
{
public:
	InputController_PlayerMode(PlayerController* playerController);

	void HandleKeyboardInput(float deltaTime) override;
	void HandleMouseInput(float deltaTime) override;
	void HandleControllerInput(float deltaTime) override;
	void Update(float deltaTime) override;

private:
	PlayerController* m_playerController = nullptr;
	Player* m_curPlayer = nullptr;
	GameCamera* m_curGameCam = nullptr;
};