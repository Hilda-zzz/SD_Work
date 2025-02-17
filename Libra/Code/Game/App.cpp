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

App*			g_theApp = nullptr;
Renderer*		g_theRenderer = nullptr;
Camera*			g_theCamera = nullptr;
InputSystem*	g_theInput = nullptr;
AudioSystem*	g_theAudio = nullptr;
//Window*			g_theWindow = nullptr;
Game*			g_theGame = nullptr;

BitmapFont* g_testFont = nullptr;


App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;

	//delete g_testFont;
}

App::App()
{
	
}

void App::Startup()
{
	LoadingGameConfig("Data/Definitions/GameConfig.xml");
	
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	g_theEventSystem->Startup();

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);
	g_theInput->Startup();
	
	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_aspectRatio = 2.f;
	windowConfig.m_windowTitle = "Libra";
	g_theWindow = new Window(windowConfig);
	g_theWindow->Startup();

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);
	g_theRenderer->Startup();

	g_testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont"); //#todo: move it to renderer start up 

	DevConsoleConfig devConsoleConfig(g_testFont, 1.f,29.5f);
	g_theDevConsole = new DevConsole(devConsoleConfig); //#todo maybe use name to get the font in devConsole startup
	//#then i can put all start() together below
	g_theDevConsole->Startup();

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);
	g_theAudio->Startup();

	g_theGame = new Game();
	EventArgs args;
	args.SetValue("name", "Squirrel");
	args.SetValue("health", "100");
	g_theEventSystem->FireEvent("Test",args);

	g_theEventSystem->SubscribeEventCallbackFuction("CloseWindow", OnQuitEvent);
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


void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		delete g_theGame;
		g_theGame = nullptr;
		g_theGame = new Game();
	}
	g_theGame->Update(deltaSeconds);
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8(1, 1, 1, 255));
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

void App::LoadingGameConfig(char const* gameConfigXmlFilePath)
{
	XmlDocument gameConfigXml;
	XmlResult result = gameConfigXml.LoadFile(gameConfigXmlFilePath);
	if (result == tinyxml2::XML_SUCCESS)
	{
		XmlElement* rootElement = gameConfigXml.RootElement();
		if (rootElement)
		{
			g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
		}
		else
		{
			DebuggerPrintf("Warning: game config was invalid(missing root element!\n");
		}
	}
	else
	{
		DebuggerPrintf("Warning: Fail to load game config from file!\n");
	}
}

bool OnQuitEvent(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}
