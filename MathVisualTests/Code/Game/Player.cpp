#include "Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Window/Window.hpp"

extern Window* g_theWindow;
Player::Player(Game* owner) :Entity(owner)
{
	g_theInput->SetCursorMode(CursorMode::FPS);
	g_theWindow->SetCursorVisible(false);

	m_position = Vec3(0.f, 0.f, 0.f);
	m_orientation = EulerAngles(0.f, 0.f, 0.f);
	m_playerCam = Camera();

	m_playerCam.SetCameraToRenderTransform(Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f)));
	m_playerCam.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f, 0.f, 0.f));
}

void Player::Update(float deltaSeconds)
{
	m_velocity = Vec3(0.f, 0.f, 0.f);
	m_angularVelocity = EulerAngles(0.f, 0.f, 0.f);
	Mat44 rollMat = Mat44();
	//--------------------------------------------------------
	UpdateKBInput(deltaSeconds);
	UpdateControllerInput(deltaSeconds);

	m_playerCam.SetPosition(m_position);
	m_playerCam.SetOrientation(m_orientation);
}

void Player::Render() const
{
}

Player::~Player()
{
}

void Player::UpdateKBInput(float deltaSeconds)
{
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);

	float curMoveSpeed;
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		curMoveSpeed = m_moveSpeed * 10.f;
	}
	else
	{
		curMoveSpeed = m_moveSpeed;
	}

	m_velocity = Vec3(0.f, 0.f, 0.f);
	Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		aimDirection += Vec3(fwdDirection.x,fwdDirection.y,0.f);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		aimDirection -= Vec3(fwdDirection.x, fwdDirection.y, 0.f);
	}
	if (g_theInput->IsKeyDown('A'))
	{
		aimDirection += leftDirection;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		aimDirection -= leftDirection;
	}
	if (g_theInput->IsKeyDown('Z'))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('C'))
	{
		m_position += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
	}

	m_angularVelocity.m_rollDegrees = 0.f;
	if (g_theInput->IsKeyDown('Q'))
	{
		m_angularVelocity.m_rollDegrees += -m_rollSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_angularVelocity.m_rollDegrees += m_rollSpeed * deltaSeconds;
	}

	m_angularVelocity.m_yawDegrees = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x;
	m_angularVelocity.m_pitchDegrees = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y;

	m_angularVelocity.m_yawDegrees *= 0.125f;
	m_angularVelocity.m_pitchDegrees *= 0.125;
	m_orientation = m_orientation + m_angularVelocity;
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

	aimDirection.Normalized();
	aimDirection.SetLength(curMoveSpeed);
	m_velocity = aimDirection;
	m_position += m_velocity * deltaSeconds;

	if (g_theInput->IsKeyDown('H'))
	{
		m_orientation.m_pitchDegrees = 0.f;
		m_orientation.m_rollDegrees = 0.f;
		m_orientation.m_yawDegrees = 0.f;
		m_position = Vec3(0.f, 0.f, 0.f);
	}
}

void Player::UpdateControllerInput(float deltaSeconds)
{
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);

	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		float curMoveSpeed;
		if (controller.IsButtongDown(XboxButtonID::A))
		{
			curMoveSpeed = m_moveSpeed * 10.f;
		}
		else
		{
			curMoveSpeed = m_moveSpeed;
		}

		Vec3 moveDirection = Vec3();
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			m_velocity = Vec3(0.f, 0.f, 0.f);
			moveDirection -= controller.GetLeftStick().GetPosition().x * leftDirection;
			moveDirection += controller.GetLeftStick().GetPosition().y * fwdDirection;

		}
		if (controller.IsButtongDown(XboxButtonID::LEFT_SHOULDER))
		{
			m_position += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
		}
		if (controller.IsButtongDown(XboxButtonID::RIGHT_SHOULDER))
		{
			m_position += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
		}
		moveDirection.Normalized();
		m_velocity = moveDirection * (leftStickMagnitude * curMoveSpeed);

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			m_angularVelocity.m_yawDegrees = -controller.GetRightStick().GetPosition().x * m_yawSpeed;
			m_angularVelocity.m_pitchDegrees = -controller.GetRightStick().GetPosition().y * m_pitchSpeed;
		}

		m_angularVelocity.m_rollDegrees = 0.f;
		if (controller.GetLeftTrigger() > 0.f)
		{
			m_angularVelocity.m_rollDegrees += -m_rollSpeed * controller.GetLeftTrigger();
		}
		if (controller.GetRightTrigger() > 0.f)
		{
			m_angularVelocity.m_rollDegrees += m_rollSpeed * controller.GetRightTrigger();
		}

		m_orientation = m_orientation + m_angularVelocity * deltaSeconds;
		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
		m_position += m_velocity * deltaSeconds;

		if (controller.IsButtongDown(XboxButtonID::START))
		{
			m_orientation.m_pitchDegrees = 0.f;
			m_orientation.m_rollDegrees = 0.f;
			m_orientation.m_yawDegrees = 0.f;
			m_position = Vec3(0.f, 0.f, 0.f);
		}
	}
}