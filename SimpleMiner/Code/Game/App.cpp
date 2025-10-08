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
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Engine/Core/XmlUtils.hpp"
#include <chrono>
#include <thread>
#include "Engine/JobSystem/JobSystem.hpp"
//#include "Game/EngineBuildPreferences.hpp"

App*			g_theApp = nullptr;
Renderer*		g_theRenderer = nullptr;
InputSystem*	g_theInput = nullptr;
AudioSystem*	g_theAudio = nullptr;
Game*			g_theGame = nullptr;
bool			g_isDebugDraw = false;
Clock*			g_systemClock = nullptr;
JobSystem*		g_theJobSystem = nullptr;

constexpr float TARGET_FPS = 400.0f;
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
	LoadingGameConfig("Data/GameConfig.xml");

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	
	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);
	
	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_aspectRatio = m_windowAspect;
	windowConfig.m_windowTitle = m_windowTitle;
	windowConfig.m_isFullscreen = m_windowFullscreen;
	g_theWindow = new Window(windowConfig);
	
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);
	
	g_systemClock = new Clock();
	DevConsoleConfig devConsoleConfig("Data/Fonts/SquirrelFixedFont", 0.7f, 45.f);
	g_theDevConsole = new DevConsole(devConsoleConfig,0);

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_fontName = "Data/Fonts/SquirrelFixedFont";
	debugRenderConfig.m_renderer=g_theRenderer;

	g_theJobSystem = new JobSystem();

	g_theEventSystem->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theAudio->Startup();
	g_theJobSystem->Startup();
	DebugRenderSystemStartup(debugRenderConfig);

	g_theEventSystem->SubscribeEventCallbackFuction("CloseWindow", OnQuitEvent);

	g_theGame = new Game();
}

void App::Shutdown()
{
	delete g_theGame;
	g_theGame = nullptr;

	DebugRenderSystemShutdown();
	g_theJobSystem->Shutdown();
	g_theAudio->Shutdown();
	g_theDevConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();

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

	delete g_systemClock;
	g_systemClock = nullptr;
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
	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
	g_theJobSystem->BeginFrame();
	DebugRenderBeginFrame();
	Clock::TickSystemClock();
}

void App::Update()
{
	if (g_theWindow->IsFocus())
	{
		if (g_theGame->m_curGameState==GameState::GAME_STATE_GAMEPLAY&&!g_theGame->m_isDevConsole)
		{
			g_theInput->SetCursorMode(CursorMode::FPS);
			g_theWindow->SetCursorVisible(false);
		}
		else
		{
			g_theInput->SetCursorMode(CursorMode::POINTER);
			g_theWindow->SetCursorVisible(true);
		}
	}
	else
	{
		g_theInput->SetCursorMode(CursorMode::POINTER);
		g_theWindow->SetCursorVisible(true);
	}

	XboxController const& controller = g_theInput->GetController(0);
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8)||controller.WasButtonJustPressed(XboxButtonID::B))
	{
		delete g_theGame;
		g_theGame = nullptr;
		g_theGame = new Game();
	}

	g_theGame->Update();
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8(100,100,100,255));
	g_theGame->Renderer();
}

void App::EndFrame()
{
	DebugRenderEndFrame();
	g_theJobSystem->EndFrame();
	g_theAudio->EndFrame();
	g_theDevConsole->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
}

void App::LoadingGameConfig(std::string const& filePath)
{
	XmlDocument gameConfigXml;
	XmlResult result = gameConfigXml.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS,
		Stringf("Failed to open required GameConfig file \"%s\"", filePath.c_str()));

	XmlElement* rootElement = gameConfigXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Failed to find GameConfig root element");

	std::string rootElementName = rootElement->Name();
	GUARANTEE_OR_DIE(rootElementName == "GameConfig",
		Stringf("Root element in %s was <%s>, must be <GameConfig>!",
			filePath.c_str(), rootElementName.c_str()));

	float windowAspect = ParseXmlAttribute(rootElement, "windowAspect", m_windowAspect);
	m_windowAspect = windowAspect;

	bool windowFullscreen = ParseXmlAttribute(rootElement, "windowFullscreen", m_windowFullscreen);
	m_windowFullscreen = windowFullscreen;

	std::string windowTitle = ParseXmlAttribute(rootElement, "windowTitle", m_windowTitle);
	m_windowTitle = windowTitle;
}

bool OnQuitEvent(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}




