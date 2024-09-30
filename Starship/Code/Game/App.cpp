#include "App.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "Game.hpp"
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Core/Time.hpp>
App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
Camera* g_theCamera = nullptr;
InputSystem* g_theInput = nullptr;
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
	//i am not sure for the camera??
	g_theCamera = new Camera();

    g_theRenderer = new Renderer(); 
    g_theRenderer->Startup();

	g_theInput = new InputSystem();
	g_theInput->Startup();

	m_theGame = new Game();
}

void App::Shutdown()
{
	delete m_theGame;
	m_theGame = nullptr;

	g_theRenderer->Shutdown();

	delete g_theRenderer;
	g_theRenderer = nullptr;

	//i am not sure for the camera??
	delete g_theCamera;
	g_theCamera = nullptr;

	g_theInput->Shutdown();
	delete g_theInput;
	g_theInput = nullptr;
}

void App::AdjustForPauseAndTimeDitortion(float& deltaSeconds)
{
	//if ( g_theInput->IsKeyDown(27))
	//{
	//	//m_isQuitting = true;
	//}
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_isPause = !m_isPause;
	}

	m_isSlow = g_theInput->IsKeyDown('T');

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPause = false;
		m_pauseAfterUpdate = true;
	}

	//--------------------------------------------------------------------------------------

	if (m_isPause)
	{
		deltaSeconds = 0.f;
	}
	if (m_isSlow)
	{
		deltaSeconds *= 0.10f;
	}
	if (m_pauseAfterUpdate)
	{
		m_isPause = true;
		m_pauseAfterUpdate = false;
	}
	//--------------------------------------------------------------------------------------

}

void App::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame(); //call engine system
}

void App::Update(float deltaSeconds)
{
	AdjustForPauseAndTimeDitortion(deltaSeconds);
	m_theGame->Update(deltaSeconds);

	//if (g_theInput->WasKeyJustPressed(0x77))
	//{
	//	//reset
	//	g_theApp->Shutdown();
	//	delete g_theApp;
	//	g_theApp = nullptr;
	//	g_theApp = new App();
	//	g_theApp->Startup();
	//}
}

void App::Render()  const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(*g_theCamera);

	m_theGame->Renderer();
	//for的时候用asteroidIndex这种具体的名字

	g_theRenderer->EndCamera(*g_theCamera);
}

void App::EndFrame()
{	
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
}

void App::RunFrame()
{
	float timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaTime = timeNow - m_timeLastFrameStart;
	m_timeLastFrameStart = timeNow;
    BeginFrame();         
	//float deltaTime = (1.f / 60.f);
	if (deltaTime > 0.1f)
		deltaTime = 0.1f;
    Update(deltaTime);
    Render();
    EndFrame();
}

void App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInput->HandleKeyPressed(keyCode);
}

void App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInput->HandleKeyReleased(keyCode);
}

void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

