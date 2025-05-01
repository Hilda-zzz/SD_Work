#include "App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/GameCommon.hpp"
#include "Game//GameNearestPoint.hpp"
#include "Game/GameRaycastVsDiscs.hpp"
#include "Game/GameRaycastVsLineSeg.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/GameRaycastVsAABB2D.hpp"
#include "Game3DTestShapes.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Game/GameCurve.hpp"
#include "GamePachinko2D.hpp"
App*			g_theApp = nullptr;
Renderer*		g_theRenderer = nullptr;
Camera*			g_theCamera = nullptr;
InputSystem*	g_theInput = nullptr;
Game*			g_theGame = nullptr;
bool			g_isDebugDraw = false;
Clock*			g_systemClock = nullptr;

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
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

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

	g_systemClock = new Clock();
	DevConsoleConfig devConsoleConfig("Data/Fonts/SquirrelFixedFont", 0.7f, 45.f);
	g_theDevConsole = new DevConsole(devConsoleConfig);

	g_theEventSystem->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();

	g_theEventSystem->SubscribeEventCallbackFuction("CloseWindow", OnQuitEvent);

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_fontName = "Data/Fonts/SquirrelFixedFont";
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRenderConfig);

	m_theGame = new GamePachinko2D();
}

void App::Shutdown()
{
	DebugRenderSystemShutdown();
	g_theDevConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete  g_theEventSystem;
	g_theEventSystem = nullptr;
}

void App::RunFrame()
{
	BeginFrame();
	Update();
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
	case GAME_MODE_RAYCAST_VS_LINESEG:
		m_theGame = new GameRaycastVsLineSeg();
		return;
	case GAME_MODEE_RAYCAST_VS_AABB2D:
		m_theGame = new GameRaycastVsAABB2D();
		return;
	case GAME_MODE_3DSHAPES:
		m_theGame = new Game3DTestShapes();
		return;
	case GAME_MODE_CURVES:
		m_theGame = new GameCurve();
		return;
	case GAME_MODE_PACHINKO2D:
		m_theGame = new GamePachinko2D();
		return;
	}
}


void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	DebugRenderBeginFrame();
	Clock::TickSystemClock();
}

void App::Update()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		delete m_theGame;
		m_theGame = nullptr;
		m_theGame = new GameNearestPoint();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		int newMode = ((int)m_curGameMode + 1)%(COUNT);
		m_curGameMode = GameMode(newMode);
		ChangeGameMode((unsigned char)m_curGameMode);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		int newMode;
		if ((int)m_curGameMode == 0)
		{
			newMode = (int)(GameMode::COUNT)-1;
		}
		else
		{
			newMode =(int)m_curGameMode -1;
		}
		m_curGameMode = GameMode(newMode);
		ChangeGameMode((unsigned char)m_curGameMode);
	}
	m_theGame->Update();
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	m_theGame->Renderer();
}

void App::EndFrame()
{
	DebugRenderEndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
}

bool OnQuitEvent(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}
