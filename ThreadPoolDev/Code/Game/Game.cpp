#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "ThreadPool.hpp"
#include <iostream>
extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

void sleepFor(int seconds) {
	std::cout << "Start Sleep " << seconds << " Secs..." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	std::cout << "Slept " << seconds << " Secs Finished" << std::endl;
}

// test task funcs
int fibonacci(int n) {
	if (n <= 1) return n;
	return fibonacci(n - 1) + fibonacci(n - 2);
}

std::string getMessage(const std::string& name, int waitSeconds) {
	std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
	return "Hello, " + name + "! (waited " + std::to_string(waitSeconds) + "s)";
}

void printMessage(const std::string& message) {
	std::cout << "Message: " << message << std::endl;
}

Game::Game()
{
	m_gameClock = new Clock();

	// Initialize thread pool
	size_t threadCount = std::thread::hardware_concurrency();
	std::cout << "The System has " << threadCount << " CPU cores" << std::endl;
	threadCount = threadCount == 0 ? 1 : threadCount;

	size_t poolThreads = std::min(threadCount, (size_t)4);

	try {
		std::cout << "\n--- Test Start ---" << std::endl;
		ThreadPool pool(poolThreads);

		std::vector<std::future<int>> fibs;
		std::vector<std::future<std::string>> msgs;
		std::vector<std::future<void>> prints;

		std::cout << "Start Submitting tasks!" << std::endl;
		std::cout << "Fibonacci tasks..." << std::endl;
		for (int i = 20; i < 25; ++i) {
			fibs.push_back(
				pool.Enqueue(fibonacci, i)
			);
		}

		std::cout << "GetMessages tasks..." << std::endl;
		for (int i = 1; i <= 3; ++i) {
			msgs.push_back(
				pool.Enqueue(getMessage, "User" + std::to_string(i), 1)
			);
		}

		std::cout << "PrintMessages tasks..." << std::endl;
		for (int i = 0; i < 3; ++i) {
			prints.push_back(
				pool.Enqueue(printMessage, "This is message " + std::to_string(i) + "\n")
			);
		}

		std::cout << "\n---Get tasks' results!---" << std::endl;
		for (size_t i = 0; i < fibs.size(); ++i) {
			std::cout << "fibonacci(" << (i + 20) << ") = " << fibs[i].get() << std::endl;
		}

		for (auto& future : msgs) {
			std::cout << future.get() << std::endl;
		}

		for (auto& future : prints) {
			future.wait();
		}

		std::cout << "\n--- Tests completed! ---" << std::endl;

		std::cout << "Is the pool stopped: " << (pool.IsStopped() ? "Yes" : "No") << std::endl;

	}
	catch(std::exception const& e){
		std::cerr << "exception when constructing thread pool in Game.cpp";
		return;
	}

	std::cout << "\n--- Test Complete ---" << std::endl;
}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;
}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

	UpdateCamera(deltaSeconds);

	// Update Game State
	if (m_curGameState != m_nextGameState)
	{
		ExitState(m_curGameState);
		EnterState(m_nextGameState);
		m_curGameState = m_nextGameState;
	}

	// Update DevConsole
	if (g_theInput->WasKeyJustPressedRaw(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->SetMode(OPEN_FULL);
			m_isDevConsole = true;
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
			m_isDevConsole = false;
		}
	}

	// Call Specific Update()
	switch (m_curGameState)
	{
	case GameState::GAME_STATE_ATTRACT:
		UpdateAttractMode(deltaSeconds);
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		UpdateGameplayMode(deltaSeconds);
		break;
	default:
		break;
	}

	UpdateDeveloperCheats(deltaSeconds);
}

void Game::Renderer() const
{
	switch (m_curGameState)
	{
	case GameState::GAME_STATE_ATTRACT:
		RenderAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		RenderGameplayMode();
		break;
	default:
		break;
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextGameState = GameState::GAME_STATE_ATTRACT;
	}
}

void Game::UpdateDeveloperCheats(float deltaTime)
{
	UNUSED(deltaTime);
	AdjustForPauseAndTimeDitortion(deltaTime);
	if (g_theInput->WasKeyJustPressed('L'))
	{
		g_isDebugDraw = !g_isDebugDraw;
	}
}

void Game::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_screenCamera.SetOrthographicView(Vec2{ 0.f,0.f }, Vec2{ 1600.f,800.f });
}

void Game::AdjustForPauseAndTimeDitortion(float& deltaSeconds)
{
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
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderUI() const
{
	g_theRenderer->BindTexture(nullptr);
	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
}

void Game::RenderDebugMode()const
{

}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::GAME_STATE_ATTRACT:
		EnterAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		EnterGameplayMode();
		break;
	default:
		break;
	}
}

void Game::EnterAttractMode()
{
}

void Game::EnterGameplayMode()
{
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::GAME_STATE_ATTRACT:
		ExitAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		ExitGameplayMode();
		break;
	default:
		break;
	}
}

void Game::ExitAttractMode()
{
}

void Game::ExitGameplayMode()
{
}











