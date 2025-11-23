#include "InputController_PlayerMode.hpp"
#include "PlayerController.hpp"
#include "Player.hpp"
#include "GameCamera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Game.hpp"
#include "World.hpp"

extern Window* g_theWindow;
InputController_PlayerMode::InputController_PlayerMode(PlayerController* playerController)
	: m_playerController(playerController)
{
	m_curPlayer = m_playerController->GetPlayer();
	m_curGameCam = m_playerController->GetCamera();
}

void InputController_PlayerMode::HandleKeyboardInput(float deltaTime)
{
	UNUSED(deltaTime);
	// 计算移动方向
	Vec3 fwdDirection, leftDirection, upDirection;
	if(m_curPlayer->GetPhysicsMode()==PhysicsMode::WALKING)
		m_curPlayer->GetOrientation().GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	else
		m_curPlayer->GetEyesightOrientation().GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);

	Vec3 moveInput(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W')) moveInput += fwdDirection;
	if (g_theInput->IsKeyDown('S')) moveInput -= fwdDirection;
	if (g_theInput->IsKeyDown('A')) moveInput += leftDirection;
	if (g_theInput->IsKeyDown('D')) moveInput -= leftDirection;

	if (moveInput.GetLengthSquared() > 0.0f)
	{
		moveInput.Normalized();
	}

	// 根据是否在地面选择加速度
	float acceleration = 0.f;
	if (m_curPlayer->GetPhysicsMode() == PhysicsMode::WALKING)
	{
		acceleration = m_curPlayer->IsOnGround() ?
			g_playerHorizontalGroundAcceleration :
			g_playerHorizontalAirAcceleration;
	}
	else
	{
		acceleration = g_playerHorizontalFlyAcceleration;
	}


	// 添加输入加速度（不是直接设置velocity）
	Vec3 newAcceleration = m_curPlayer->GetAcceleration() + moveInput * acceleration;
	m_curPlayer->SetAcceleration(newAcceleration); 

	// 跳跃
	Vec3 originVelocity = m_curPlayer->GetVelocity();
	//originVelocity.z = 0.f;
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		if (m_curPlayer->CanJump())
		{
			m_curPlayer->SetVelocity(originVelocity + Vec3(0.f, 0.f, g_playerJumpImpulse));
		}
	}
}

void InputController_PlayerMode::HandleMouseInput(float deltaTime)
{
	// 在FirstPerson和OverTheShoulder模式下，鼠标旋转Player的朝向
	// 相机会自动跟随Player（在GameCamera::Update中处理）
	UNUSED(deltaTime);

	Vec2 mouseDelta = g_theInput->GetCursorClientDelta();
	IntVec2 clientDims = g_theWindow->GetClientDimensions();

	// 转换为归一化坐标
	float normalizedDeltaX = mouseDelta.x * (float)clientDims.x;
	float normalizedDeltaY = mouseDelta.y * (float)clientDims.y;

	// 应用灵敏度 (0.075 per mouse delta，符合Assignment文档)
	float mouseSensitivity = 0.075f;
	float yawDelta = -normalizedDeltaX * mouseSensitivity;
	float pitchDelta = normalizedDeltaY * mouseSensitivity;

	// 获取Player当前朝向
	EulerAngles orientation = m_curPlayer->GetOrientation();

	// Player只旋转yaw（水平旋转）
	orientation.m_yawDegrees += yawDelta;

	// 相机pitch单独存储（不影响Player的移动方向）
	float cameraPitch = m_curGameCam->GetOrientation().m_pitchDegrees + pitchDelta;
	cameraPitch = GetClamped(cameraPitch, -85.0f, 85.0f);

	// 更新Player朝向（只有yaw）
	m_curPlayer->SetOrientation(orientation);

	// 更新相机朝向（yaw + pitch）
	EulerAngles cameraOrientation(orientation.m_yawDegrees, cameraPitch, 0.0f);
	m_curGameCam->SetOrientation(cameraOrientation);

	m_curPlayer->SetEyesightOrientation(cameraOrientation);

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_curPlayer->GetWorld()->DigAtRaycast();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		m_curPlayer->GetWorld()->PlaceBlockAtRaycast(m_curPlayer->m_curBlockBrushName);
	}
}

void InputController_PlayerMode::HandleControllerInput(float deltaTime)
{
	UNUSED(deltaTime);
}

void InputController_PlayerMode::Update(float deltaTime)
{
	if (!m_playerController || !m_curPlayer || !m_curGameCam)
	{
		return;
	}
	HandleKeyboardInput(deltaTime);
	HandleMouseInput(deltaTime);
	HandleControllerInput(deltaTime);
}
