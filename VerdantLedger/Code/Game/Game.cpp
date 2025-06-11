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
#include "TileManager.hpp"
#include "Map.hpp"
#include "Player.hpp"

extern bool g_isDebugDraw;
extern Window* g_theWindow;

Game::Game()
{
	m_gameClock = new Clock();
	g_tileManager = &TileMapManager::GetInstance();
	g_tileManager->InitAllTilemapResources();

	m_player = new Player();
	m_curMap = new Map(this, g_tileManager->m_loadedMaps["Data/Tiled/HouseMap.tmx"], m_player);
}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;

	TileMapManager::DestroyInstance();
	g_tileManager = nullptr;

	delete m_curMap;
	m_curMap = nullptr;
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
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
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

	m_curMap->Update(deltaTime);

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
	m_curMap->Render();

	g_theRenderer->BeginCamera(m_screenCamera);
	RenderGameplayUI();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayUI() const
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
}

void Game::ExitState(GameState state)
{
}











