#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/Time.hpp>
#include "Engine/Math/AABB3.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Engine/Math/AABB2.hpp"
#include "TileDefinition.hpp"
#include "MapDefinition.hpp"
#include "Map.hpp"

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;


SpriteSheet* g_terrianSpriteSheet = nullptr;

Game::Game()
{
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();

	

	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));


	std::string logString = "\
	Mouse x-axis / Right stick x-axis       Yaw\n\
	Mouse y-axis / Right stick y-axis       Pitch\n\
	Q / E / Left trigger / right trigger    Roll\n\
	A / D / Left stick x-axis               Move left or right, relative to player orientation\n\
	W / S / Left stick y-axis               Move forward or back, relative to player orientation\n\
	Z / C / Left shoulder / right shoulder  Move down or up, relative to the world\n\
	H / Start button                        Reset position and orientation to zero\n\
	Shift / A button                        Increase speed by a factor of 10 while held\n\
	P                                       Pause the game\n\
	O                                       Single step frame\n\
	T                                       Slow motion mode";
	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString);

	Texture* tileMapTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_terrianSpriteSheet = new SpriteSheet(*tileMapTexture, IntVec2(8, 8));
	TileDefinition::InitializeTileDefinitionFromFile();
	MapDefinition::InitializeMapDefinitionFromFile();

}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;

	delete g_terrianSpriteSheet;
	g_terrianSpriteSheet = nullptr;
}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();
	//UpdateCamera(deltaSeconds);

	if (m_curGameState != m_nextState)
	{
		ExitState(m_curGameState);
		EnterState(m_nextState);
		m_curGameState = m_nextState;
	}
	
	//----------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->SetMode(OPEN_FULL);
			m_isDevConsole = true;
			g_theApp->SetCursorMode(CursorMode::POINTER);
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
			m_isDevConsole = false;

			if (m_curGameState == GameState::PLAYING)
			{
				g_theApp->SetCursorMode(CursorMode::FPS);
			}
		}
	}
	//----------------------------------------------------------------
	switch (m_curGameState)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		UpdateAttractMode(deltaSeconds);
		break;
	case GameState::LOBBY:
		break;
	case GameState::PLAYING:
		UpdateGameplayMode(deltaSeconds);
		break;
	case GameState::COUNT:
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
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		RenderAttractMode();
		break;
	case GameState::LOBBY:
		break;
	case GameState::PLAYING:
		g_theRenderer->BeginCamera(m_player->m_playerCam);
		m_curMap->Render();
		DebugRenderWorld(m_player->m_playerCam);
		g_theRenderer->EndCamera(m_player->m_playerCam);
		//--------------------------------------------------------
		g_theRenderer->BeginCamera(m_screenCamera);
		DebugRenderScreen(m_screenCamera);
		g_theRenderer->EndCamera(m_screenCamera);
		break;
	case GameState::COUNT:
		break;
	default:
		break;
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		EnterAttractMode();
		break;
	case GameState::LOBBY:
		break;
	case GameState::PLAYING:
		EnterPlayingMode();
		break;
	case GameState::COUNT:
		break;
	default:
		break;
	}
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::NONE:
		break;
	case GameState::ATTRACT:
		break;
	case GameState::LOBBY:
		break;
	case GameState::PLAYING:
		ExitPlayingMode();
		break;
	case GameState::COUNT:
		break;
	default:
		break;
	}
}

void Game::EnterAttractMode()
{
	//m_nextState = GameState::ATTRACT;
	g_theApp->SetCursorMode(CursorMode::POINTER);
}

void Game::EnterPlayingMode()
{
	//m_nextState = GameState::PLAYING;
	g_theApp->SetCursorMode(CursorMode::FPS);

	m_maps.reserve(10);
	Map* map1 = new Map(this, &MapDefinition::s_mapDefinitions[0]);
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
	m_curMap = m_maps[m_curMapIndex];
	m_curMap->InitializeMap();

	m_player = new Player(this);
	m_player->m_curMap = m_curMap;
	m_player->m_playerCam.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);

	m_curMap->SetPlayer(m_player);
}

void Game::ExitPlayingMode()
{
	for (Map* map : m_maps)
	{
		delete map;
		map = nullptr;
	}
	m_maps.clear();
	delete m_player;
	m_player = nullptr;
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
		//EnterState(GameState::PLAYING);
		m_nextState = GameState::PLAYING;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	m_player->Update(deltaTime);
	m_curMap->Update(deltaTime);
	//----------------------------------------------------------------------------------------
	/*if (g_theInput->WasKeyJustPressed('1'))
	{
		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
		Vec3 endPoint = m_player->m_position + dir* 20.f;
		DebugAddWorldLine(m_player->m_position, endPoint, 0.05f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW,DebugRenderMode::X_RAY);
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		DebugAddWorldPoint(Vec3(m_player->m_position.x, m_player->m_position.y,0.f), 0.25f, 60.f, Rgba8(150,75,0), Rgba8(150, 75, 0));
	}

	if (g_theInput->WasKeyJustPressed('3'))
	{
		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
		Vec3 center = m_player->m_position + dir * 2.f;
		DebugAddWorldWireSphere(center, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED,DebugRenderMode::USE_DEPTH);
	}

	if (g_theInput->WasKeyJustPressed('4'))
	{
		Mat44 transform = Mat44::MakeTranslation3D(m_player->m_position);
		transform.Append(m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
		DebugAddWorldBasis(transform, 10.f);
	}

	if (g_theInput->WasKeyJustPressed('5'))
	{
		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
		Vec3 center = m_player->m_position + dir * 2.f;

		char buffer[256];
		snprintf(buffer, sizeof(buffer),
			"position: %.2f, %.2f, %.2f, orientation: %.2f, %.2f, %.2f",
			m_player->m_position.x, m_player->m_position.y, m_player->m_position.z,
			m_player->m_orientation.m_rollDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_yawDegrees);
		DebugAddWorldBillboardText(std::string(buffer), center, 0.3f, Vec2(0.5f, 0.5f), 10.f, Rgba8::WHITE, Rgba8::RED);
	}

	if (g_theInput->WasKeyJustPressed('6'))
	{
		DebugAddWorldWireCylinder(m_player->m_position - Vec3(0.f, 0.f, 0.2f), m_player->m_position + Vec3(0.f, 0.f, 0.2f), 0.2f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}


	if (g_theInput->WasKeyJustPressed('7'))
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer),
			"player orientation: %.2f, %.2f, %.2f",
			m_player->m_orientation.m_rollDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_yawDegrees);
		DebugAddMessage(std::string(buffer), 5.f, Rgba8::HILDA, Rgba8::RED);
	}

	char timeBuffer[256];
	snprintf(timeBuffer, sizeof(timeBuffer),
		"Time: %.2f  FPS: %.2f  Scale: %.2f",
		g_systemClock->GetTotalSeconds(), 1.f/g_systemClock->GetDeltaSeconds(), 1.f);
	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 720.f), Vec2(1590.f, 790.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);*/

	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		//ExitState(GameState::PLAYING);
		//EnterState(GameState::ATTRACT);
		m_nextState = GameState::ATTRACT;
	}
}

void Game::UpdateDeveloperCheats(float deltaTime)
{
	UNUSED(deltaTime);
	AdjustForPauseAndTimeDitortion();
}

void Game::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	
	m_player->m_playerCam.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
}

void Game::AdjustForPauseAndTimeDitortion()
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_previousTimeScale = (float)m_gameClock->GetTimeScale();
	}

	if (g_theInput->IsKeyDown('T'))
	{
		m_gameClock->SetTimeScale(0.1f);
	}

	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_gameClock->SetTimeScale(m_previousTimeScale);
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock->StepSingleFrame();
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE,Vec2(SCREEN_SIZE_X*0.5f,SCREEN_SIZE_Y*0.5f));
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderPlayingModeHUD() const
{
	g_theRenderer->BindTexture(nullptr);
	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
}

void Game::RenderDebugMode()const
{

}

//void Game::AddVertsForGroundGrid()
//{
//	// add verts for ground grid
//	for (int i = 1; i <= 101; i++)
//	{
//		if (i % 5 == 1)
//		{
//			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.03f, -50.f, -0.03f);
//			Vec3 yTr = yBl + Vec3(0.06f, 100.f, 0.06f);
//			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8::GREEN);
//
//			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.03f, -0.03f);
//			Vec3 xTr = xBl + Vec3(100.f, 0.06f, 0.06f);
//			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8::RED);
//		}
//
//		else
//		{
//			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.01f, -50.f, -0.01f);
//			Vec3 yTr = yBl + Vec3(0.02f, 100.f, 0.02f);
//			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8(180, 180, 180, 255));
//
//			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.01f, -0.01f);
//			Vec3 xTr = xBl + Vec3(100.f, 0.02f, 0.02f);
//			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8(180, 180, 180, 255));
//		}
//
//	}
//}
//
//void Game::AddVertsForCubes()
//{
//	//add cube vertexs to m_cube
//	//X
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
//	//-X
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
//	//Y
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
//	//-Y
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
//	//Z
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
//	//-Z
//	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
//
//
//	//X
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
//	//-X
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
//	//Y
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
//	//-Y
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
//	//Z
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
//	//-Z
//	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
//}

//void Game::AddEntityToList(Entity& thisEntity, EntityList& list)
//{
//	for (int i = 0; i < static_cast<int>(list.size()); i++)
//	{
//		if (list[i] == nullptr)
//		{
//			list[i] = &thisEntity;
//			return;
//		}
//	}
//	list.push_back(&thisEntity);
//}













