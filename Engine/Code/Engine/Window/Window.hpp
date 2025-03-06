#pragma once
#include <string>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
class InputSystem;

struct WindowConfig
{
	InputSystem*		m_inputSystem=nullptr;
	float				m_aspectRatio=(16.f/9.f);
	std::string			m_windowTitle = "Unnamed Application";
};
class Window
{
public:
	Window(WindowConfig windowConfig);
	~Window();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();
	WindowConfig const& GetConfig() const;
	void* GetDisplayContext() const;
	Vec2 GetNormalizedMouseUV() const;
	Vec2 GetMousePixelPos() const;
	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;
	
	bool IsFocus();
	bool IsCursorVisible();
	void SetCursorVisible(bool aimState);
	bool SetCursorPosisiotn(int x, int y);
	IntVec2 GetClientRectCenterPos();
	//-----------------------------------------

	//bool SetCursorPos(int X, int Y);
	//bool GetCursorPos(LPPOINT lpPoint);
	//bool GetCursorInfo(PCURSORINFO pci);
	//bool ClientToScreen(HWND hWnd, LPPOINT lpPoint);
	//bool ScreenToClient(HWND hWnd, LPPOINT lpPoint);

public:
	static Window* s_mainWindow;
	

private:
	void CreateOSWindow();
	void RunMessagePump();
	
private:
	WindowConfig m_config;
	void* m_displayContext = nullptr;
	void* m_windowHandle = nullptr;

	IntVec2 m_clientDimension;
};