#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"
class Player :public Entity
{
public:
	Player(Game* owner);
	void Update(float deltaSeconds) override;
	void Render() const override;
	 ~Player();

	 void UpdateKBInput(float deltaSeconds);
	 void UpdateControllerInput(float deltaSeconds);

public:
	Camera m_playerCam;
	float m_moveSpeed = 4.f;
	float m_sprintFactor = 20.f;
	float m_rollSpeed = 90.f;
	float m_yawSpeed = 50.f;
	float m_pitchSpeed = 50.f;
	float m_mouseSensitivity = 0.075f;
	float m_gamepadSensitivity = 180.f;
};