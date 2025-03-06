#include "Engine/Window/Window.hpp"
#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"

Window* Window::s_mainWindow = nullptr;

Window* g_theWindow = nullptr;

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	InputSystem* input = nullptr;
	if (Window::s_mainWindow)
	{
		WindowConfig const& config = Window::s_mainWindow->GetConfig();
		input = config.m_inputSystem;
	}

	switch (wmMessageCode)
	{

	case WM_CLOSE:
	{
		g_theEventSystem->FireEvent("CloseWindow");
		return 0; 
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
		g_theEventSystem->FireEvent("KeyPressed", args);
		return 0;
	}

	case WM_KEYUP:
	{
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
		g_theEventSystem->FireEvent("KeyReleased", args);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
	/*	if (input)
		{
			input->HandleKeyPressed(KEYCODE_LEFT_MOUSE);
		}*/
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", KEYCODE_LEFT_MOUSE));
		g_theEventSystem->FireEvent("KeyPressed", args);
		return 0;
	}
	case WM_LBUTTONUP:
	{
	/*	if (input)
		{
			input->HandleKeyReleased(KEYCODE_LEFT_MOUSE);
		}*/
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", KEYCODE_LEFT_MOUSE));
		g_theEventSystem->FireEvent("KeyReleased", args);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		/*if (input)
		{
			input->HandleKeyPressed(KEYCODE_RIGHT_MOUSE);
		}*/
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", KEYCODE_RIGHT_MOUSE));
		g_theEventSystem->FireEvent("KeyPressed", args);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		/*if (input)
		{
			input->HandleKeyReleased(KEYCODE_RIGHT_MOUSE);
		}*/
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", KEYCODE_RIGHT_MOUSE));
		g_theEventSystem->FireEvent("KeyReleased", args);
		return 0;
	}
	case WM_CHAR:
	{
		EventArgs args;
		args.SetValue("CharInput", Stringf("%d", (char)wParam));
		g_theEventSystem->FireEvent("CharInput", args);
		return 0;
	}
	}
	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		BOOL const wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}
		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

Window::Window(WindowConfig windowConfig):m_config(windowConfig),m_clientDimension(IntVec2(0,0))
{
	s_mainWindow = this;
}

Window::~Window()
{
}

void Window::Startup()
{
	CreateOSWindow();

}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{

}
void Window::Shutdown()
{

}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

void* Window::GetDisplayContext() const
{
	return m_displayContext;
}

Vec2 Window::GetNormalizedMouseUV() const
{
	HWND windowHandle = static_cast<HWND>(m_windowHandle);
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = static_cast<float>(cursorCoords.x) / static_cast<float>(clientRect.right);
	float cursorY = static_cast<float>(cursorCoords.y) / static_cast<float>(clientRect.bottom);
	return Vec2(cursorX, 1.f - cursorY);
}

Vec2 Window::GetMousePixelPos() const
{
	HWND windowHandle = static_cast<HWND>(m_windowHandle);
	POINT cursorCoords;
	//RECT clientRect;
	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords);
	return Vec2((float)cursorCoords.x,(float)cursorCoords.y);
}

void* Window::GetHwnd() const
{
	return m_windowHandle;
}

IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimension;
}

bool Window::IsFocus()
{
	HWND window = GetActiveWindow();
	if (window == m_windowHandle)
	{
		return true;
	}
	else 
		return false;
}

bool Window::IsCursorVisible()
{
	CURSORINFO cursorInfo;
	cursorInfo.cbSize = sizeof(CURSORINFO);

	if (GetCursorInfo(&cursorInfo))
	{
		return (cursorInfo.flags & CURSOR_SHOWING) != 0;
	}
	return true;
}

void Window::SetCursorVisible(bool aimState)
{
	bool curState = IsCursorVisible();
	if (aimState == curState)
	{
		return;
	}
	if (aimState)
	{
		while (true)
		{
			if (::ShowCursor(aimState) >=  0)
			{
				break;
			}
		}
	}
	else
	{
		while (true)
		{
			if (::ShowCursor(aimState) < 0)
			{
				break;
			}
		}
	}
}

bool Window::SetCursorPosisiotn(int x, int y)
{
	if (IsFocus())
	{
		// Convert to screen coordinates for SetCursorPos
		POINT aimPoint = { x, y};
		::ClientToScreen(static_cast<HWND>(m_windowHandle), &aimPoint);
		::SetCursorPos(aimPoint.x, aimPoint.y);
		return true;
	}
	return false;
}

IntVec2 Window::GetClientRectCenterPos()
{
	RECT clientRect;
	::GetClientRect(static_cast<HWND>(m_windowHandle), &clientRect);
	int centerX = (clientRect.right - clientRect.left) / 2;
	int centerY = (clientRect.bottom - clientRect.top) / 2;
	return IntVec2(centerX, centerY);
	
}

void Window::CreateOSWindow()
{
	HMODULE applicationInstanceHandle = ::GetModuleHandleA(NULL);
	float clientAspect = m_config.m_aspectRatio;

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	DWORD const windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	DWORD const windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if (clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	//Store Client Dimension
	m_clientDimension.x = (int)clientWidth;
	m_clientDimension.y = (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)applicationInstanceHandle,
		NULL);
	m_windowHandle = windowHandle;
	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	m_displayContext = GetDC(windowHandle);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}




