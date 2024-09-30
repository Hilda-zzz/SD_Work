
#pragma once
#include "AnalogJoystick.hpp"
#include "KeyButtonState.hpp"
#include <Engine/Input/XboxButtonID.hpp>
class XboxController
{
	friend class InputSystem;
public:
	XboxController();
	XboxController(int id);
	~XboxController();
	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton(XboxButtonID buttonID) const;
	bool IsButtongDown(XboxButtonID buttonID) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;
private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joysstick, short rawX, short rawY);
	void UpdateTrigger(float& our_triggerValue, unsigned char rawValue);
	void UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);
private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::NUM];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};