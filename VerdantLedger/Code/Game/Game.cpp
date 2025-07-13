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
#include "TileMapManager.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include <iostream>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ObstacleDefinitions.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/GameUISystem/GameUISystem.hpp"
#include "Engine/GameUISystem/Panel.hpp"

extern GameUISystem* g_gameUISystem;
extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

Vec2 const g_halfGameCamDimensions{ 30.f, 15.f };

Game::Game()
{
	m_gameClock = new Clock();
	ObstacleDefinition::InitializeObstacleDefinitionFromFile();

	g_tileManager = &TileMapManager::GetInstance();
	g_tileManager->InitAllTilemapResources();

	m_player = new Player(this);
	m_curMap = new Map(this, g_tileManager->m_loadedMaps["Data/Tiled/MyFarmMap.tmx"], m_player);

	g_theDevConsole->AddLine(DevConsole::HELPLIST, "WASD: Move around\n\
Tools: 0-None, 1-Axe, 2-Hoe, 3-Pickaxe, 4-Shovel, 5-Sickle, 6-Water\n\
Left Mouse Button: Use selected tool");

	InitializeMenuButtons();
}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;

	TileMapManager::DestroyInstance();
	g_tileManager = nullptr;

	delete m_curMap;
	m_curMap = nullptr;


	ObstacleDefinition::ShutdownObstacleDefinition();
}

void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

	m_curDeltaTime = deltaSeconds;

	float framerate = 1.f / m_curDeltaTime;
	if (framerate < 200)
		m_dropTimes += 1;

	UpdateCamera(deltaSeconds);

	g_gameUISystem->Update(deltaSeconds);

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
	g_gameUISystem->Render();
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::InitializeMenuButtons()
{
	Texture* menuBkg = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/dialogue box.png");
	m_menuPanel =Panel(Vec2(800.f, 400.f), menuBkg, AABB2(Vec2(-30.f, -30.f), Vec2(30.f, 30.f)));
	g_gameUISystem->PushPanel(&m_menuPanel);

	Texture* menuBtnTexture1 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn1.png");
	Texture* menuBtnTexture2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn2.png");
	Texture* menuBtnTexture3 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn3.png");

	float btnWidth = 240.0f;
	float btnHeight = 80.0f;

	float leftMargin = 200.0f;
	float startY = 350.0f;
	float buttonSpacing = 100.0f;

	AABB2 bkgExtent = AABB2(-btnWidth / 2.0f, -btnHeight / 2.0f, btnWidth / 2.0f, btnHeight / 2.0f);

	AABB2 textExtent = AABB2(-btnWidth / 2.0f + 10.0f, -btnHeight / 2.0f + 10.0f,
		btnWidth / 2.0f - 10.0f, btnHeight / 2.0f - 10.0f);

	BitmapFont* gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	float textHeight = 30.0f;

	Vec2 btnNormalPos = Vec2(leftMargin + btnWidth / 2.0f, startY);
	m_btnMenuStartNew = Button(g_gameUISystem,btnNormalPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "New", textHeight, gameFont, "StartNew");

	Vec2 btnGoldPos = Vec2(leftMargin + btnWidth / 2.0f, startY - buttonSpacing);
	m_btnMenuLoad = Button(g_gameUISystem, btnGoldPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Load", textHeight, gameFont, "Load");

	Vec2 btnTutorialPos = Vec2(leftMargin + btnWidth / 2.0f, startY - 2 * buttonSpacing);
	m_btnMenuExit = Button(g_gameUISystem, btnTutorialPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Exit", textHeight, gameFont, "Exit");

	//call back
	g_theEventSystem->SubscribeEventCallbackFuction("StartNew", BtnEvent_StartNew, true);
	g_theEventSystem->SubscribeEventCallbackFuction("Load", BtnEvent_Load, true);
	g_theEventSystem->SubscribeEventCallbackFuction("Exit", BtnEvent_Exit, true);

	m_menuPanel.AddChild(&m_btnMenuStartNew);
	m_menuPanel.AddChild(&m_btnMenuLoad);
	m_menuPanel.AddChild(&m_btnMenuExit);
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
// 	{
// 		m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
// 	}

// 	m_btnMenuStartNew.Update(deltaTime);
// 	m_btnMenuLoad.Update(deltaTime);
// 	m_btnMenuExit.Update(deltaTime);
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

	std::vector<Vertex_PCU> verts;
	verts.reserve(20);
	AddVertsForAABB2D(verts, AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)), Rgba8::WHITE);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBkg.png"));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);

// 	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
// 	m_btnMenuStartNew.Render(g_theRenderer);
// 	m_btnMenuLoad.Render(g_theRenderer);
// 	m_btnMenuExit.Render(g_theRenderer);

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	m_curMap->Render();

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	DebugDrawBoxLine(m_player->m_position - Vec2(10.f, 5.f), m_player->m_position + Vec2(10.f, 5.f), 0.5f, Rgba8::GREEN);

	g_theRenderer->BeginCamera(m_screenCamera);
	RenderGameplayUI();
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::string curTool;
	curTool = "Current tool: ";
	switch (m_player->m_curTool) {
	case PlayerTools::NONE:
		curTool += "None";
		break;
	case PlayerTools::AXE:
		curTool += "Axe";
		break;
	case PlayerTools::HOE:
		curTool += "Hoe";
		break;
	case PlayerTools::PICKAXE:
		curTool += "Pickaxe";
		break;
	case PlayerTools::SHOVEL:
		curTool += "Shovel";
		break;
	case PlayerTools::SICKLE:
		curTool += "Sickle";
		break;
	case PlayerTools::WATER:
		curTool += "Water";
		break;
	default:
		curTool += "Unknown";
		break;
	}
	curTool += " || Use 0-6 to switch tools || Use mouse left btn to use the tool (Pickaxe--Rock || Sickle--Weed)";
	font->AddVertsForTextInBox2D(title, curTool,
		AABB2(Vec2(10.f, 745.f), Vec2(1000.f, 765.f)), 15.f, Rgba8::BLACK, 0.7f, Vec2(0.f, 0.f));

	float framerate = 1.f / m_curDeltaTime;
	DebuggerPrintf("Framerate: %.2f\n", framerate);
	char buffer[256];
	sprintf_s(buffer, 
		"DT = %.2f\n"
		"Framerate = %.2f\n",
		m_curDeltaTime * 1000.f,
		framerate);
	std::string statsMessage(buffer);

	font->AddVertsForTextInBox2D(title, statsMessage,
		AABB2(Vec2(10.f, 500.f), Vec2(600.f, 700.f)), 15.f, Rgba8::WHITE, 0.7f, Vec2(0.f, 1.f));

// 	g_theRenderer->BindTexture(&font->GetTexture());
// 	g_theRenderer->DrawVertexArray(panelVerts);

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayUI() const
{
// 	g_theRenderer->BindTexture(nullptr);
// 	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
// 	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));

// 	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
// 	Vec2 mousePositionInGameCam = AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)).GetPointAtUV(mouseUV);
// 	DebugDrawCircle(20.f, mousePositionInGameCam,Rgba8::CYAN);
	//Vec2 mousePosInWorldPlace = mousePositionInGameCam - Vec2(0.5 * g_cameraDimensions.x, 0.5 * g_cameraDimensions.y);
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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		EnterGameplayMode();
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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
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
	g_gameUISystem->PopAllPanel();
}

void Game::ExitGameplayMode()
{
	g_gameUISystem->PopAllPanel();
}

void Game::EnterAttractMode()
{
	g_gameUISystem->PushPanel(&m_menuPanel);
}

void Game::EnterGameplayMode()
{

}

bool Game::BtnEvent_StartNew(EventArgs& args)
{
	UNUSED(args);
	m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	return true;
}

bool Game::BtnEvent_Load(EventArgs& args)
{
	UNUSED(args);
	m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	return true;
}

bool Game::BtnEvent_Exit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true;
	return true;
}










