#include <Windows.h> 
#include "InputSystem.hpp"

InputSystem::InputSystem()
{
	for (int i = 0; i < NUM_KEYCODES; i++)
	{
		m_keyStates[i] = KeyButtonState();
	}
	
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_controllers[i] = XboxController(i);
	}
}

InputSystem::~InputSystem()
{
}

void InputSystem::Startup()
{
}

void InputSystem::Shutdown()
{
}

void InputSystem::BeginFrame()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_controllers[i].Update();
	}
}

void InputSystem::EndFrame()
{
	for (int i = 0; i < NUM_KEYCODES; i++)
	{
		m_keyStates[i].m_wasPressedLastFrame = m_keyStates[i].m_isPressed;
	}
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	if (m_keyStates[keyCode].m_isPressed == true && m_keyStates[keyCode].m_wasPressedLastFrame == false)
	{
		return true;
	}
	return false;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	if (m_keyStates[keyCode].m_isPressed == false && m_keyStates[keyCode].m_wasPressedLastFrame == true)
	{
		return true;
	}
	return false;
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isPressed;
}

bool InputSystem::AnythingDown()
{
	for (int i = 0; i < NUM_KEYCODES; i++)
	{
		if (m_keyStates[i].m_isPressed)
		{
			return true;
		}
	}
	return false;
}

void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = true;
}

void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = false;
}

XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}
