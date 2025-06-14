#include <Windows.h> 
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Window/Window.hpp"
unsigned char const KEYCODE_F1		= VK_F1;
unsigned char const KEYCODE_F2		= VK_F2;
unsigned char const KEYCODE_F3		= VK_F3;
unsigned char const KEYCODE_F4		= VK_F4;
unsigned char const KEYCODE_F5		= VK_F5;
unsigned char const KEYCODE_F6		= VK_F6;
unsigned char const KEYCODE_F7		= VK_F7;
unsigned char const KEYCODE_F8		= VK_F8;
unsigned char const KEYCODE_F9		= VK_F9;
unsigned char const KEYCODE_F10		= VK_F10;
unsigned char const KEYCODE_F11		= VK_F11;
unsigned char const KEYCODE_ESC		= VK_ESCAPE;
unsigned char const KEYCODE_SPACE	= VK_SPACE;

unsigned char const KEYCODE_UPARROW		= VK_UP;
unsigned char const KEYCODE_DOWNARROW	= VK_DOWN;
unsigned char const KEYCODE_LEFTARROW	= VK_LEFT;
unsigned char const KEYCODE_RIGHTARROW	= VK_RIGHT;

unsigned char const KEYCODE_LEFT_MOUSE		= VK_LBUTTON;
unsigned char const KEYCODE_RIGHT_MOUSE		= VK_RBUTTON;
unsigned char const KEYCODE_TILDE			= 0xC0;
unsigned char const KEYCODE_LEFTBRACKET		= 0xDB;
unsigned char const KEYCODE_RIGHTBRACKET	= 0xDD;
unsigned char const KEYCODE_ENTER			= VK_RETURN;
unsigned char const KEYCODE_BACKSPACE		= VK_BACK; 
unsigned char const KEYCODE_INSERT			= VK_INSERT;
unsigned char const KEYCODE_DELETE			= VK_DELETE;
unsigned char const KEYCODE_HOME			= VK_HOME;
unsigned char const KEYCODE_END				= VK_END;
unsigned char const KEYCODE_LEFT_SHIFT = VK_SHIFT;


extern InputSystem* g_theInput;
//extern Window* g_theWindow;

InputSystem::InputSystem(InputSystemConfig inputConfig)
{
	UNUSED(inputConfig);
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
	g_theEventSystem->SubscribeEventCallbackFuction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFuction("KeyReleased", InputSystem::Event_KeyReleased);
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
	//-------------------------------------------------------------
	m_lastCursorPos = m_curCursorPos;
	m_curCursorPos = Window::s_mainWindow->GetMousePixelPos();

	bool curVisible= Window::s_mainWindow->IsCursorVisible();
	if (m_cursorMode == CursorMode::FPS)
	{
		if (curVisible)
		{
			Window::s_mainWindow->SetCursorVisible(false);
		}
		IntVec2 clientDimensions = Window::s_mainWindow->GetClientDimensions();

		Vec2 rawDelta = m_curCursorPos - m_lastCursorPos;
		m_cursorClientDelta.x = rawDelta.x / (float)clientDimensions.x;
		m_cursorClientDelta.y = rawDelta.y / (float)clientDimensions.y;

		IntVec2 clientCenter = Window::s_mainWindow->GetClientRectCenterPos();
		Window::s_mainWindow->SetCursorPosisiotn(clientCenter.x, clientCenter.y);
	}
	else
	{
		if (!curVisible)
		{
			Window::s_mainWindow->SetCursorVisible(true);
		}
		m_cursorClientDelta = Vec2::ZERO;
	}
	m_curCursorPos = Window::s_mainWindow->GetMousePixelPos();
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

XboxController& InputSystem::GetControllerAndSet(int controllerID)
{
	return m_controllers[controllerID];
}

void InputSystem::SetCursorMode(CursorMode cursorMode)
{
	m_cursorMode = cursorMode;
}

Vec2 InputSystem::GetCursorClientDelta() const
{
	if (m_cursorMode == CursorMode::FPS)
	{
		return m_cursorClientDelta;
	}
	else
	{
		return Vec2::ZERO;
	}
}

Vec2 InputSystem::GetCursorClientPosition() const
{
	return g_theWindow->GetMousePixelPos();
}

bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	g_theInput->HandleKeyPressed(keyCode);
	return true;
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	g_theInput->HandleKeyReleased(keyCode);
	return true;
}

