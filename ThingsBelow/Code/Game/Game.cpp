#include "Game/Game.hpp"
//#include "Game/Tentacle.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "ThirdParty/ImGUI/implot.h"
#include <Engine/Math/MathUtils.hpp>
#include "Game/TentacleDebugPanel.hpp"
#include "TentacleManager.hpp"
#include "Food.hpp"
#include "SceneBounds.hpp"

extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

Game::Game()
{
	m_gameClock = new Clock();

	m_tentacleManager = nullptr;
	m_heldFood = nullptr;       
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
	case GameState::GAME_STATE_TENTACLE_TEST:
		UpdateTentacleTest(deltaSeconds);
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
	case GameState::GAME_STATE_TENTACLE_TEST:
		RenderTentacleTest();
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
	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_nextGameState = GameState::GAME_STATE_TENTACLE_TEST;
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

void Game::UpdateTentacleTest(float deltaTime)
{
	// 返回 Attract 的逻辑
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextGameState = GameState::GAME_STATE_ATTRACT;
	}

	// ====================================================================
	// 食物生成和拖动逻辑
	// ====================================================================

	bool isMousePressed = g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
	bool wasMouseJustPressed = g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE);
	bool wasMouseJustReleased = g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE);

	// 获取鼠标世界坐标
	Vec2 mouseClientPos = g_theInput->GetCursorClientPosition();
	IntVec2 windowDims = g_theWindow->GetClientDimensions();
	Vec2 mouseWorldPos = m_screenCamera.GetOrthoBottomLeft() +
		Vec2(mouseClientPos.x / windowDims.x,
			(windowDims.y - mouseClientPos.y) / windowDims.y) *
		(m_screenCamera.GetOrthoTopRight() - m_screenCamera.GetOrthoBottomLeft());

	// 鼠标刚按下：生成食物
	if (wasMouseJustPressed)
	{
		Food* newFood = new Food(mouseWorldPos, 15.0f);
		newFood->SetState(ObjectState::HELD);
		m_foods.push_back(newFood);
		m_heldFood = newFood;
	}

	// 鼠标持续按下：食物跟随鼠标
	if (isMousePressed && m_heldFood)
	{
		m_heldFood->SetPosition(mouseWorldPos);
	}

	// 鼠标松开：食物开始掉落
	if (wasMouseJustReleased && m_heldFood)
	{
		m_heldFood->SetState(ObjectState::DROPPING);
		m_heldFood = nullptr;
	}

	// ====================================================================
	// 更新所有食物
	// ====================================================================

	for (Food* food : m_foods)
	{
		food->Update(deltaTime);
	}

	// ====================================================================
	// 更新触手管理器
	// ====================================================================

	if (m_tentacleManager)
	{
		std::vector<Food*> droppingFoods;
		for (Food* food : m_foods)
		{
			if (food->IsDropping()||food->IsStatic())
			{
				droppingFoods.push_back(food);
			}
		}

		// 告诉 Manager 哪些食物在掉落
		m_tentacleManager->SetDroppingFoods(droppingFoods);

		m_tentacleManager->Update(deltaTime);
	}

	// ====================================================================
	// 清理被抓住的食物
	// ====================================================================

	for (auto it = m_foods.begin(); it != m_foods.end(); )
	{
		if ((*it)->ShouldDelete())  // 使用标记
		{
			delete* it;
			it = m_foods.erase(it);
		}
		else
		{
			++it;
		}
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
	
	// Draw instructions
	std::vector<Vertex_PCU> textVerts;
	g_theRenderer->BindTexture(&g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont")->GetTexture());
	g_theRenderer->GetBitmapFontFromFileName("Data/Fonts/SquirrelFixedFont")->AddVertsForText2D(textVerts, Vec2(SCREEN_SIZE_X * 0.5f - 200.f, SCREEN_SIZE_Y * 0.5f), 20.f, "Press T for Tentacle Test");
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
	
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderTentacleTest() const
{
	//g_theRenderer->BeginCamera(m_screenCamera);
	//g_theRenderer->BindTexture(nullptr);
	//
	//if (m_testTentacle)
	//{
	//	m_testTentacle->Render();
	//}
	//
	//// Draw instructions
	//std::vector<Vertex_PCU> textVerts;
	//g_theRenderer->BindTexture(&g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont")->GetTexture());
	//g_theRenderer->GetBitmapFontFromFileName("Data/Fonts/SquirrelFixedFont")->AddVertsForText2D(textVerts, Vec2(10.f, 780.f), 15.f, "Drag mouse to move target | U: Toggle Debug UI | ESC: Back");
	//g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
	//
	//g_theRenderer->EndCamera(m_screenCamera);
	//

	g_theRenderer->BeginCamera(m_screenCamera);

	// Render ImGui debug UI
	if (m_tentacleDebugPanel)
	{
		m_tentacleDebugPanel->Render();
	}

	g_theRenderer->BindTexture(nullptr);

	// 渲染所有食物
	for (const Food* food : m_foods)
	{
		food->Render();
	}

	// 渲染触手管理器（包含所有触手）
	if (m_tentacleManager)
	{
		m_tentacleManager->Render();
	}

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
	case GameState::GAME_STATE_TENTACLE_TEST:
		EnterTentacleTest();
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

void Game::EnterTentacleTest()
{
	//m_testTentacle = new Tentacle();
	//m_testTentacle->Initialize(23, 128.0f, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
	//m_testTentacle->SetRootFixed(true);
	//m_testTentacle->SetAngleConstraint(360.0f);
	//m_testTentacle->SetCamera(&m_screenCamera);
	// 

	Vec2 sceneBoundsMin = m_screenCamera.GetOrthoBottomLeft();
	Vec2 sceneBoundsMax = m_screenCamera.GetOrthoTopRight();
	SceneBounds::SetBounds(sceneBoundsMin, sceneBoundsMax);

	if (!m_tentacleManager)
	{
		m_tentacleManager = new TentacleManager();
		m_tentacleManager->Initialize(&m_screenCamera, Vec2(800.0f, 100.0f));

		// 让管理器创建触手
		m_tentacleManager->CreateLeaderTentacle(40, 128.0f, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));

		// TODO Phase 3: 创建常驻触手
		m_tentacleManager->AddNewResidentTentacle(40, 180.0f, Vec2(750.0f, 100.0f));
		m_tentacleManager->AddNewResidentTentacle(40, 180.0f, Vec2(850.0f, 100.0f));
	}

	// 新增：创建调试面板
	m_tentacleDebugPanel = new TentacleEditPanel();
	if (m_tentacleManager && m_tentacleDebugPanel)
	{
		m_tentacleDebugPanel->SetTentacle(m_tentacleManager->GetLeaderTentacle());
	}
	m_tentacleDebugPanel->Show();
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
	case GameState::GAME_STATE_TENTACLE_TEST:
		ExitTentacleTest();
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

void Game::ExitTentacleTest()
{
	delete m_tentacleDebugPanel;
	m_tentacleDebugPanel = nullptr;

	// 清理食物
	for (Food* food : m_foods)
	{
		delete food;
	}
	m_foods.clear();

	// 清理管理器
	if (m_tentacleManager)
	{
		m_tentacleManager->Shutdown();
		delete m_tentacleManager;
		m_tentacleManager = nullptr;
	}
}
