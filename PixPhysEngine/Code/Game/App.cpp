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
#include <chrono>
#include <thread>
#include "Engine/JobSystem/JobSystem.hpp"
#include "Game/Nova2D/Nova2DSystem.hpp"
//#include "Game/EngineBuildPreferences.hpp"

App*			g_theApp = nullptr;
Renderer*		g_theRenderer = nullptr;
Camera*			g_theCamera = nullptr;
InputSystem*	g_theInput = nullptr;
AudioSystem*	g_theAudio = nullptr;
Game*			g_theGame = nullptr;
bool			g_isDebugDraw = false;
Clock*			g_systemClock = nullptr;
JobSystem*		g_theJobSystem = nullptr;

Nova2DSystem* g_nova2D = nullptr;

constexpr float TARGET_FPS = 60.0f;
constexpr float TARGET_FRAME_TIME = (1.0f / TARGET_FPS) * 1000.f;


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
	windowConfig.m_windowTitle = "Protogame2D";
	g_theWindow = new Window(windowConfig);
	
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	rendererConfig.m_imguiInitialized = true;
	g_theRenderer = new Renderer(rendererConfig);
	
	g_systemClock = new Clock();
	DevConsoleConfig devConsoleConfig("Data/Fonts/SquirrelFixedFont", 0.7f, 45.f);
	g_theDevConsole = new DevConsole(devConsoleConfig,0);

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	g_theJobSystem = new JobSystem();

	Nova2DConfig nova2DConfig = Nova2DConfig();
	g_nova2D = new Nova2DSystem(g_theRenderer, nova2DConfig);
	
	g_theEventSystem->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();
	g_theJobSystem->Startup();
	g_theEventSystem->SubscribeEventCallbackFuction("CloseWindow", OnQuitEvent);
	g_nova2D->Startup();

	g_theGame = new Game();
}

void App::Shutdown()
{
	delete g_theGame;
	g_theGame = nullptr;

	g_nova2D->Shutdown();
	g_theJobSystem->Shutdown();
	g_theAudio->Shutdown();
	g_theDevConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_nova2D;
	g_nova2D = nullptr;

	delete g_theJobSystem;
	g_theJobSystem = nullptr;

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
	auto lastFrameTime = std::chrono::high_resolution_clock::now();
	while (!m_isQuitting)
	{
		auto frameStartTime = std::chrono::high_resolution_clock::now();
		g_theApp->RunFrame();
		auto currentTime = std::chrono::high_resolution_clock::now();

		float actualFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - frameStartTime).count();

		while (actualFrameTime < TARGET_FRAME_TIME) {
			std::this_thread::yield();
			currentTime = std::chrono::high_resolution_clock::now();
			actualFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - frameStartTime).count();
		}
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

	g_theRenderer->BeginImguiFrame();

	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
	g_theJobSystem->BeginFrame();

	g_nova2D->BeginFrame();

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
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	g_theGame->Renderer();

	g_theRenderer->RenderImguiFrame();
}

void App::EndFrame()
{
	g_nova2D->EndFrame();
	g_theJobSystem->EndFrame();
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




