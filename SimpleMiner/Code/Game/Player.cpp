#include "Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Window/Window.hpp"
#include "World.hpp"

extern Window* g_theWindow;
Player::Player(Game* owner) :Entity(owner)
{
	m_position = Vec3(0.f, 0.f, 120.f);
	//m_position = Vec3(0.f, 0.f, 0.f);
	m_orientation = EulerAngles(-45.f, 30.f, 0.f);
	m_playerCam = Camera();
	
 	m_playerCam.SetCameraToRenderTransform(Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f)));
 	m_playerCam.SetPositionAndOrientation(m_position, m_orientation);
}

void Player::Update(float deltaSeconds)
{
	m_velocity = Vec3(0.f, 0.f, 0.f);
	m_angularVelocity = EulerAngles(0.f, 0.f, 0.f);
	Mat44 rollMat = Mat44();
	//--------------------------------------------------------
	UpdateKBInput(deltaSeconds);
	UpdateControllerInput(deltaSeconds);

	HandleGameplayKBInput();

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
	// change mode
	if (g_theInput->WasKeyJustPressed('C'))
	{
		m_isSpectatorFull = !m_isSpectatorFull;
	}

	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	if (m_isSpectatorFull)
	{
		m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	}
	else
	{
		EulerAngles xyOrientation(m_orientation.m_yawDegrees, 0.f, 0.f);  // Only use yaw
		xyOrientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	}


	float curMoveSpeed;
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		curMoveSpeed=m_moveSpeed* m_sprintFactor;
	}
	else
	{
		curMoveSpeed = m_moveSpeed;
	}

	m_velocity = Vec3(0.f, 0.f, 0.f);
	Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		aimDirection += fwdDirection;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		aimDirection -= fwdDirection;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		aimDirection += leftDirection;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		aimDirection -= leftDirection;
	}

	m_angularVelocity.m_rollDegrees = 0.f;
	if (g_theInput->IsKeyDown('Q'))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_position += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
	}

	m_angularVelocity.m_yawDegrees = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x;
	m_angularVelocity.m_pitchDegrees = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y;

	m_angularVelocity.m_yawDegrees *= m_mouseSensitivity;
	m_angularVelocity.m_pitchDegrees *= m_mouseSensitivity;
	m_orientation = m_orientation + m_angularVelocity;
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);

	aimDirection.Normalized();
	aimDirection.SetLength(curMoveSpeed);
	m_velocity = aimDirection;
	m_position += m_velocity * deltaSeconds;

	// digging

}

void Player::HandleGameplayKBInput()
{
	// Block digging and placing
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_curWorld->DigBlock(m_position);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		m_curWorld->PlaceBlock(m_curBlockBrushName, m_position);
	}

	// Block selection
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_curBlockBrushName = "Glowstone";
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_curBlockBrushName = "Cobblestone";
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_curBlockBrushName = "ChiseledBrick";
	}
}

void Player::UpdateControllerInput(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		if (controller.WasButtonJustPressed(XboxButtonID::DPAD_UP))
		{
			m_isSpectatorFull = !m_isSpectatorFull;
		}

		Vec3 fwdDirection;
		Vec3 leftDirection;
		Vec3 upDirection;
		if (m_isSpectatorFull)
		{
			m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
		}
		else
		{
			EulerAngles xyOrientation(m_orientation.m_yawDegrees, 0.f, 0.f);  // Only use yaw
			xyOrientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
		}

		float curMoveSpeed;
		if (controller.GetLeftTrigger() > 0.f|| controller.GetRightTrigger() > 0.f)
		{
			curMoveSpeed = m_moveSpeed * m_sprintFactor;
		}
		else
		{
			curMoveSpeed = m_moveSpeed;
		}

		Vec3 moveDirection = Vec3();
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			m_velocity = Vec3(0.f, 0.f,0.f);
			moveDirection -= controller.GetLeftStick().GetPosition().x* leftDirection;
			moveDirection += controller.GetLeftStick().GetPosition().y* fwdDirection;

		}
 		if (controller.IsButtonDown(XboxButtonID::LEFT_SHOULDER))
 		{
 			m_position += Vec3(0.f, 0.f, 1.f) * curMoveSpeed*deltaSeconds;
 		}
 		if (controller.IsButtonDown(XboxButtonID::RIGHT_SHOULDER))
 		{
 			m_position += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
 		}
		moveDirection.Normalized();
		m_velocity = moveDirection * (leftStickMagnitude * curMoveSpeed);

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			m_angularVelocity.m_yawDegrees = -controller.GetRightStick().GetPosition().x * m_yawSpeed;
			m_angularVelocity.m_pitchDegrees = -controller.GetRightStick().GetPosition().y* m_pitchSpeed;
		}

		m_orientation = m_orientation + m_angularVelocity * deltaSeconds;
		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
		m_position += m_velocity * deltaSeconds;

	}
}

void Player::HandleGameplayControllerInput()
{
	XboxController const& controller = g_theInput->GetController(0);

	// Block digging and placing
 	if (controller.WasButtonJustPressed(XboxButtonID::X))
 	{
		m_curWorld->DigBlock(m_position);
 	}
 	if (controller.WasButtonJustPressed(XboxButtonID::Y))
 	{
		m_curWorld->PlaceBlock(m_curBlockBrushName, m_position);
 	}
 
 	// Block selection
 	if (controller.WasButtonJustPressed(XboxButtonID::DPAD_LEFT))
 	{
		m_curBlockBrushName = "Glowstone";
 	}
 	if (controller.WasButtonJustPressed(XboxButtonID::DPAD_DOWN))
 	{
		m_curBlockBrushName = "Cobblestone";
 	}
 	if (controller.WasButtonJustPressed(XboxButtonID::DPAD_RIGHT))
 	{
		m_curBlockBrushName = "ChiseledBrick";
 	}
}
