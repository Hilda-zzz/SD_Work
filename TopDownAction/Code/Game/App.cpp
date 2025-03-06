#include "App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
//#include "Game/EngineBuildPreferences.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
Camera* g_theCamera = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Game* g_theGame = nullptr;
bool			g_isDebugDraw = false;
Clock* g_systemClock = nullptr;



App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;
}

App::App()
{

}

void App::Startup()
{
	//LoadingGameConfig("Data/Definitions/GameConfig.xml");

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_aspectRatio = 2.f;
	windowConfig.m_windowTitle = "TopDownActionHMlike";
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	g_systemClock = new Clock();
	DevConsoleConfig devConsoleConfig("Data/Fonts/SquirrelFixedFont", 0.7f, 45.f);
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	g_theEventSystem->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();

	g_theEventSystem->SubscribeEventCallbackFuction("CloseWindow", OnQuitEvent);

	g_theGame = new Game();
}

void App::Shutdown()
{
	delete g_theGame;
	g_theGame = nullptr;

	g_theAudio->Shutdown();
	g_theDevConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theAudio;
	g_theAudio = nullptr;

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


void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
	Clock::TickSystemClock();
}

void App::Update()
{
	if (g_theInput->WasKeyJustPressed(0x77))
	{
		delete g_theGame;
		g_theGame = nullptr;
		g_theGame = new Game();
	}
	g_theGame->Update();
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8::HILDA);
	g_theGame->Renderer();
}

void App::EndFrame()
{
	g_theAudio->EndFrame();
	g_theDevConsole->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
}

bool OnQuitEvent(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}