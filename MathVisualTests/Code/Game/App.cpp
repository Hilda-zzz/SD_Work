#include "App.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "Game.hpp"
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Core/Time.hpp>
#include <Engine/Audio/AudioSystem.hpp>
#include <Engine/Window/Window.hpp>
#include "Game/GameCommon.hpp"
#include "Game//GameNearestPoint.hpp"
#include "Game/GameRaycastVsDiscs.hpp"

App*			g_theApp = nullptr;
Renderer*		g_theRenderer = nullptr;
Camera*			g_theCamera = nullptr;
InputSystem*	g_theInput = nullptr;
AudioSystem*	g_theAudio = nullptr;
Window*			g_theWindow = nullptr;
bool			g_isDebugDraw = false;
App::~App()
{
	delete m_theGame;
	m_theGame = nullptr;
}

App::App()
{
	
}

void App::Startup()
{
	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);
	
	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_aspectRatio = 2.f;
	windowConfig.m_windowTitle = "MathVisualTests";
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
    g_theRenderer = new Renderer(rendererConfig);
	
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();

	m_theGame = new GameRaycastVsDiscs();
}

void App::Shutdown()
{
	delete m_theGame;
	m_theGame = nullptr;

	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;
}

void App::RunFrame()
{
	float timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaTime = timeNow - m_timeLastFrameStart;
	m_timeLastFrameStart = timeNow;
	BeginFrame();
	if (deltaTime > 0.1f)
		deltaTime = 0.1f;
	Update(deltaTime);
	Render();
	EndFrame();
}

void App::RunMainLoop()
{
	while (!m_isQuitting)
	{
		g_theApp->RunFrame();
	}
}

void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

void App::ChangeGameMode(unsigned char modeType)
{
	delete m_theGame;
	m_theGame = nullptr;
	switch (modeType)
	{
	case GAME_MODE_NEAREST_POINT:
		m_theGame = new GameNearestPoint();
		return;
	case GAME_MODE_RAYCAST_VS_DISCS:
		m_theGame = new GameRaycastVsDiscs();
		return;
	}
}


void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		delete m_theGame;
		m_theGame = nullptr;
		m_theGame = new GameNearestPoint();
	}
	m_theGame->Update(deltaSeconds);
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	m_theGame->Renderer();
}

void App::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
}




