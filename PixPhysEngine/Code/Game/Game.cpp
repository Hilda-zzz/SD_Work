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
#include "SandboxMap.hpp"
#include "SandboxPlayer.hpp"

#include "ThirdParty/imgui/imgui.h"
#include "CellMatManager.hpp"
#include "WangTileMap.hpp"

extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

RandomNumberGenerator Game::s_rng = RandomNumberGenerator();

Game::Game()
{
	m_gameClock = new Clock();

	CellMatManager::InitializeMaterials();
	/*m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
	m_sandboxMap = new SandboxMap(m_sandboxPlayer,IntVec2(640,320));

	m_wangTileMap = new WangTileMap(m_sandboxPlayer);*/
}

Game::~Game()
{
	if (m_sandboxPlayer)
	{
		delete m_sandboxPlayer;
		m_sandboxPlayer = nullptr;
	}

	if (m_sandboxMap)
	{
		delete m_sandboxMap;
		m_sandboxMap = nullptr;
	}

	if (m_wangTileMap)
	{
		delete m_wangTileMap;
		m_wangTileMap = nullptr;
	}

	delete m_gameClock;
	m_gameClock = nullptr;
}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();
	UpdateDeveloperCheats(deltaSeconds);
	m_curDeltaTime=deltaSeconds;
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

	switch (m_selectedGameMode)
	{
	case GameMode::SANDBOX:
		m_sandboxMap->Update(deltaTime);
		break;
	case GameMode::WANG_TILE_MAP:
		m_wangTileMap->Update(deltaTime);
		break;
	default:
		break;
	}
	//m_sandboxMap->Update(deltaTime);
	//m_wangTileMap->Update(deltaTime);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextGameState = GameState::GAME_STATE_ATTRACT;
	}
}

void Game::UpdateDeveloperCheats(float& deltaTime)
{
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

	RenderGameModeSelectionUI();
}

void Game::RenderGameplayMode() const
{
	//m_sandboxMap->Render();

	//m_wangTileMap->Render();
	switch (m_selectedGameMode)
	{
	case GameMode::SANDBOX:
		m_sandboxMap->Render();
		break;
	case GameMode::WANG_TILE_MAP:
		m_wangTileMap->Render();
		break;
	default:
		break;
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	float framerate = 1.f / m_curDeltaTime;
	char buffer[256];
	sprintf_s(buffer,
		"DT = %.2f\n"
		"Framerate = %.2f\nPress C to change material",
		m_curDeltaTime * 1000.f,
		framerate);
	std::string statsMessage(buffer);
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, statsMessage,
		AABB2(Vec2(100.f, 450.f), Vec2(1000.f, 750.f)), 15.f, Rgba8::CYAN, 0.7f, Vec2(0.f, 1.f));
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameModeSelectionUI() const
{
	// ImGui Game Mode Selection Window
	ImGui::SetNextWindowPos(ImVec2(SCREEN_SIZE_X * 0.5f - 150.f, SCREEN_SIZE_Y * 0.5f - 100.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300.f, 200.f), ImGuiCond_Always);

	ImGui::SetNextWindowFocus();

	ImGui::Begin("Game Mode Selection", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Select Game Mode:");
	ImGui::Spacing();
	ImGui::Spacing();

	// Center the buttons
	float buttonWidth = 200.f;
	float buttonHeight = 40.f;
	float windowWidth = ImGui::GetWindowWidth();
	float buttonPosX = (windowWidth - buttonWidth) * 0.5f;

	ImGui::SetCursorPosX(buttonPosX);
	if (ImGui::Button("Sandbox Mode", ImVec2(buttonWidth, buttonHeight)))
	{
		const_cast<Game*>(this)->m_selectedGameMode = GameMode::SANDBOX;
		const_cast<Game*>(this)->m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}

	ImGui::Spacing();

	ImGui::SetCursorPosX(buttonPosX);
	if (ImGui::Button("Wang Tile Map Mode", ImVec2(buttonWidth, buttonHeight)))
	{
		const_cast<Game*>(this)->m_selectedGameMode = GameMode::WANG_TILE_MAP;
		const_cast<Game*>(this)->m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}

	ImGui::Spacing();

	ImGui::SetCursorPosX(buttonPosX);
	if (ImGui::Button("Exit", ImVec2(buttonWidth, buttonHeight)))
	{
		g_theApp->m_isQuitting = true;
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Display currently selected mode
	const char* modeName = (m_selectedGameMode == GameMode::SANDBOX) ? "Sandbox" : "Wang Tile Map";
	ImGui::Text("Current Selection: %s", modeName);

	ImGui::End();
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
	m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
	switch (m_selectedGameMode)
	{
	case GameMode::SANDBOX:
		m_sandboxMap = new SandboxMap(m_sandboxPlayer, IntVec2(640, 320));
		break;
	case GameMode::WANG_TILE_MAP:
		m_wangTileMap = new WangTileMap(m_sandboxPlayer);
		break;
	default:
		break;
	}
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











