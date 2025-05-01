//#pragma once
//#include "Game/Actor.hpp"
//#include "Engine/Renderer/Camera.hpp"
//class Map;
//
//enum class PlayerMode
//{
//	FIRST_PLAYER,
//	FREE_CAMERA,
//	COUNT
//};
//
//
//class Player :public Actor
//{
//public:
//	Player(int actorDefIndex, Map* curMap);
//	void Update(float deltaSeconds) override;
//	void Render() const override;
//	 ~Player();
//
//	 void UpdateKBInputCam(float deltaSeconds);
//	 void UpdateControllerInputCam(float deltaSeconds);
//	 void UpdateCamera();
//
//public:
//	PlayerMode m_playerMode = PlayerMode::FREE_CAMERA;
//	Camera m_playerCam;
//
//	EulerAngles m_angularVelocity;
//
//	Vec3 m_camPosition;
//	EulerAngles m_camOrientation;
//
//	bool m_isCamMode = false;
//
//	float m_moveSpeed = 1.f;
//	float m_rollSpeed = 90.f;
//	float m_yawSpeed = 50.f;
//	float m_pitchSpeed = 50.f;
//	float m_yawPitchRatio = 0.075f;
//	Map* m_curMap = nullptr;
//
//	float m_eyeHeight;
//	float m_cameraFOV;
//};