#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "MapDefinition.hpp"
#include "TileDefinition.hpp"
#include "Game/Map.hpp"
extern bool g_isDebugDraw;

SpriteSheet* g_terrianSpriteSheet = nullptr;
SpriteSheet* g_wallSpriteSheet = nullptr;

Game::Game()
{
	Texture* tileMapTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/bacground0.png");
	Texture* wallMapTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/WallSet01.png");
	g_terrianSpriteSheet = new SpriteSheet(*tileMapTexture, IntVec2(56, 24));
	g_wallSpriteSheet = new SpriteSheet(*wallMapTexture, IntVec2(4, 2));

	m_gameClock = new Clock();
	m_player = new Player(Vec2(10.f,10.f));

	MapDefinition::InitializeMapDefinitionFromFile();
	TileDefinition::InitializeTileDefinitionFromFile();
	TileDefinition::InitializeWallDefinitionFromFile();

	m_maps.reserve(10);
	Map* map1 = new Map(this, MapDefinition::s_mapDefinitions[0]);
	// 	Map* map2 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
	// 	Map* map3 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
	// 	Map* map4 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
	// 	Map* map5 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
	// 	Map* map6 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
	m_maps.push_back(map1);
	// 	m_maps.push_back(map2);
	// 	m_maps.push_back(map3);
	// 	m_maps.push_back(map4);
	// 	m_maps.push_back(map5);
	// 	m_maps.push_back(map6);
	m_curMap = m_maps[0];
	m_curMap->InitializeTileMapFromImage(m_curMap->m_mapDef.m_mapImageName,&m_curMap->m_tiles);
	m_curMap->InitializeTileMapFromImage(m_curMap->m_mapDef.m_mapWallImageName, &m_curMap->m_walls);
}

Game::~Game()
{
	delete m_player;
	m_player = nullptr;
	delete m_gameClock;
	m_gameClock = nullptr;

	for (Map* map : m_maps)
	{
		delete map;
		map = nullptr;
	}
	m_maps.clear();
}

void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();
	UpdateCamera(deltaSeconds);
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->SetMode(OPEN_FULL);
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
		}
	}
	if (m_isAttractMode)
	{
		UpdateAttractMode(deltaSeconds);
		return;
	}
	UpdateDeveloperCheats(deltaSeconds);
	UpdateGameplayMode(deltaSeconds);
}

void Game::Renderer() const
{
	if (m_isAttractMode)
	{
		RenderAttractMode();
		return;
	}
	else
	{
		RenderGameMode();
		return;
	}
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_isAttractMode = false;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isAttractMode = true;
	}
	m_player->Update(deltaTime);
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
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_gameCamera.SetOrthographicView(m_player->m_position-Vec2(GAME_SIZE_X/2.f, GAME_SIZE_Y/2.f), 
		m_player->m_position + Vec2(GAME_SIZE_X / 2.f, GAME_SIZE_Y / 2.f));
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

void Game::RenderGameMode() const
{
	g_theRenderer->BeginCamera(m_gameCamera);
	m_curMap->Render();
	m_player->Render();
	g_theRenderer->EndCamera(m_gameCamera);

	g_theRenderer->BeginCamera(m_screenCamera);
	//RenderUI();
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
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



















