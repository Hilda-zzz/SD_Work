#include <Windows.h> 
#include <Xinput.h>
#include "XboxController.hpp"
#include <Engine/Math/MathUtils.hpp>
#pragma comment( lib, "xinput" ) 
// Note: for Windows 7 and earlier support, use “xinput 9_1_0” explicitly instead

XboxController::XboxController()
{
	for (int i = 0; i < static_cast<int>(XboxButtonID::NUM); i++)
	{
		m_buttons[i] = KeyButtonState();
	}
	m_leftStick = AnalogJoystick();
	m_rightStick = AnalogJoystick();
}

XboxController::XboxController(int id):m_id(id)
{
	for (int i = 0; i < static_cast<int>(XboxButtonID::NUM); i++)
	{
		m_buttons[i] = KeyButtonState();
	}
}

XboxController::~XboxController()
{
}

bool XboxController::IsConnected() const
{
	XINPUT_STATE xboxControllerState = {}; // Clear (zero-out) the controller state structure
	DWORD result = XInputGetState(m_id, &xboxControllerState); // Get fresh state info
	if (result == ERROR_SUCCESS) // Result if the controller is connected (error code is SUCCESS)
	{
		return true;
	}
	else
		return false;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[static_cast<int>(buttonID)];
}

bool XboxController::IsButtongDown(XboxButtonID buttonID) const
{
	return m_buttons[static_cast<int>(buttonID)].m_isPressed;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	if (m_buttons[static_cast<int>(buttonID)].m_isPressed == true && m_buttons[static_cast<int>(buttonID)].m_wasPressedLastFrame == false)
	{
		return true;
	}
	return false;
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	if (m_buttons[static_cast<int>(buttonID)].m_isPressed == false && m_buttons[static_cast<int>(buttonID)].m_wasPressedLastFrame == true)
	{
		return true;
	}
	return false;
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	XInputGetState(m_id, &xboxControllerState); // Get fresh state info
	if (!IsConnected())
	{
		m_isConnected = false;
		Reset();
	}
	else
	{
		m_isConnected = true;

		//change place
		for (int i = 0; i < static_cast<int>(XboxButtonID::NUM); i++)
		{
			m_buttons[i].m_wasPressedLastFrame = m_buttons[i].m_isPressed;
		}

		UpdateJoystick(m_leftStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateJoystick(m_rightStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateTrigger(m_leftTrigger, xboxControllerState.Gamepad.bLeftTrigger);
		UpdateTrigger(m_rightTrigger, xboxControllerState.Gamepad.bRightTrigger);
		for (int i = 0; i< static_cast<int>(XboxButtonID::NUM)-4; i++)
		{
			UpdateButton(static_cast<XboxButtonID>(i), xboxControllerState.Gamepad.wButtons, static_cast<unsigned short>(1 << i));
		}
		UpdateButton(static_cast<XboxButtonID>(10), xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A);
		UpdateButton(static_cast<XboxButtonID>(11), xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B);
		UpdateButton(static_cast<XboxButtonID>(12), xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X);
		UpdateButton(static_cast<XboxButtonID>(13), xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y);
	}

}

void XboxController::Reset()
{
	m_leftStick.Reset();
	m_rightStick.Reset();
	for (int i = 0; i < static_cast<int>(XboxButtonID::NUM); i++)
	{
		m_buttons[i].m_isPressed = false;
		m_buttons[i].m_wasPressedLastFrame = false;
	}
	m_leftTrigger = 0.0f;
	m_rightTrigger = 0.0f;
	m_isConnected = false;
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joysstick, short rawX, short rawY)
{
	float rawNormX = RangeMap(static_cast<float>(rawX), -32768.f, 32767.f, -1.f, 1.f);
	float rawNormY = RangeMap(static_cast<float>(rawY), -32768.f, 32767.f, -1.f, 1.f);
	out_joysstick.UpdatePosition(rawNormX, rawNormY);
}

void XboxController::UpdateTrigger(float& our_triggerValue, unsigned char rawValue)
{
	our_triggerValue= RangeMap(float(rawValue), 0.f, 255.f, 0.f, 1.f);
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	//bool isp= (buttonFlags & buttonFlag) == buttonFlag;
	m_buttons[(int)buttonID].m_isPressed = (buttonFlags & buttonFlag) == buttonFlag;
}
