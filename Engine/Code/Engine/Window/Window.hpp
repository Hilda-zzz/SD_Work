#pragma once
#include <string>
#include "Engine/Math/Vec2.hpp"
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

public:
	static Window* s_mainWindow;
	

private:
	void CreateOSWindow();
	void RunMessagePump();
	
private:
	WindowConfig m_config;
	void* m_displayContext = nullptr;
	void* m_windowHandle = nullptr;
};