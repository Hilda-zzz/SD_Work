#pragma once
#include "InputController.hpp"

class Player;
class GameCamera;
class PlayerController;

class InputController_SpectatorMode : public InputController
{
public:
	InputController_SpectatorMode(PlayerController* playerController);

	void HandleKeyboardInput(float deltaTime) override;
	void HandleMouseInput(float deltaTime) override;
	void HandleControllerInput(float deltaTime) override;
	void Update(float deltaTime) override;

	void SetIsXY(bool isXY) { m_xyOnly = isXY; }

private:
	PlayerController* m_playerController = nullptr;
	Player* m_curPlayer = nullptr;
	GameCamera* m_curGameCam = nullptr;
	bool m_xyOnly = false;  //SPECTATOR_XY → m_xyOnly = true
};