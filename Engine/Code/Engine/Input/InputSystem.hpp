#pragma once

#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <set>
#include "IInputConsumer.hpp"

class NamedStrings;
typedef NamedStrings EventArgs;
class IInputConsumer;

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_SPACE;

extern unsigned char const KEYCODE_LEFT_MOUSE;
extern unsigned char const KEYCODE_RIGHT_MOUSE;
extern unsigned char const KEYCODE_LEFTBRACKET;
extern unsigned char const KEYCODE_RIGHTBRACKET;

extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;
extern unsigned char const KEYCODE_LEFT_SHIFT;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

enum class CursorMode
{
	POINTER,
	FPS
};

struct CursorState
{
	IntVec2 m_cursorClientDelta;
	IntVec2 m_cursorClientPosition;
};

struct InputSystemConfig
{

};

struct InputConsumerComparator
{
	bool operator()(IInputConsumer* a, IInputConsumer* b) const {
		if (a->GetPriority() != b->GetPriority())
		{
			return a->GetPriority() < b->GetPriority();
		}
		return a < b;
	}
};

enum class InputEventType 
{
	NONE,
	KEY_PRESS,
	KEY_RELEASE,
	MOUSE_MOVE,
	MOUSE_PRESS,
	MOUSE_RELEASE,
	MOUSE_WHEEL
};

struct InputEvent 
{
	InputEventType type = InputEventType::NONE;
	unsigned char keyCode = 0;
	Vec2 mousePos = Vec2::ZERO;
	bool consumed = false;

	static InputEvent KeyPress(unsigned char key) 
	{
		InputEvent e;
		e.type = InputEventType::KEY_PRESS;
		e.keyCode = key;
		return e;
	}

	static InputEvent KeyRelease(unsigned char key)
	{
		InputEvent e;
		e.type = InputEventType::KEY_RELEASE;
		e.keyCode = key;
		return e;
	}
};

class InputSystem
{
public:
	InputSystem(InputSystemConfig inputConfig);
	~InputSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void RegisterInputConsumer(IInputConsumer* consumer);
	void UnregisterInputConsumer(IInputConsumer* consumer);

	bool WasKeyJustPressedRaw(unsigned char keyCode);
	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	bool IsKeyDownRaw(unsigned char keyCode);
	bool IsKeyDown(unsigned char keyCode);

	bool AnythingDown();
	void HandleKeyPressed(unsigned char keyCode);
	void HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);
	XboxController& GetControllerAndSet(int controllerID);

	void SetCursorMode(CursorMode cursorMode);

	//only for fps mode
	Vec2 GetCursorClientDelta() const;

	Vec2 GetCursorClientPosition() const;

	//Vec2 GetCursorNormalizedPosition() const;

public:
	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);

protected:
	
	KeyButtonState m_rawKeyStates[NUM_KEYCODES];
	KeyButtonState m_gameplayKeyStates[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	CursorMode m_cursorMode = CursorMode::POINTER;
	//CursorState m_cursorState;
	Vec2 m_lastCursorPos = Vec2::ZERO;
	Vec2 m_curCursorPos = Vec2::ZERO;
	Vec2 m_cursorClientDelta = Vec2::ZERO;

private:
	std::multiset<IInputConsumer*, InputConsumerComparator> m_inputConsumersList;

};