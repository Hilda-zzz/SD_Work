#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Prop.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/Time.hpp>
#include "Engine/Math/AABB3.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Engine/Math/AABB2.hpp"
#include "GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/CubeSkyBox.hpp"
#include "BlockDefinition.hpp"
#include "World.hpp"
#include "Chunk.hpp"

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

Game::Game()
{
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();

	m_player = new Player(this);
	m_groundGrid = new Prop(this);

	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
	m_screenCamera.SetViewport(viewport);

	m_screenSize.x = SCREEN_SIZE_X;
	m_screenSize.y= ((float)clientDimensions.y* SCREEN_SIZE_X )/ (float)clientDimensions.x;
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), m_screenSize);
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));

	m_player->m_playerCam.SetViewport(viewport);
	float camAspect = viewport.GetDimensions().x / viewport.GetDimensions().y;
	m_player->m_playerCam.SetPerspectiveView(camAspect, 60.f, 0.1f, 300.f);

	std::string logString = "\
	Mouse x-axis / Right stick x-axis         Yaw\n\
	Mouse y-axis / Right stick y-axis         Pitch\n\
	A / D / Left stick x-axis                 Move left or right, relative to the camera\n\
	W / S / Left stick y-axis                 Move forward or back, relative to the camera\n\
	Q / E / Left shoulder / right shoulder    Move up or down, relative to the world\n\
	Shift / Left trigger / right trigger     Increase speed while held\n\
	F2 / A button                             Toggle debug draw\n\
	F8 / B button                             Reload";
	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString);

	//----------sky box--------------------
	std::string skyboxPaths[] = {
 	"Data/Images/SkyBox1/skyhsky_lf.png",
 	"Data/Images/SkyBox1/skyhsky_rt.png",
 	"Data/Images/SkyBox1/skyhsky_dn.png",
 	"Data/Images/SkyBox1/skyhsky_up.png",
 	"Data/Images/SkyBox1/skyhsky_ft.png",
 	"Data/Images/SkyBox1/skyhsky_bk.png",
	};

	std::string skyBoxShaderPath = "Data/Shaders/CubeSkyBox";

	m_cubeSkybox = new CubeSkyBox(g_theRenderer, skyboxPaths,&skyBoxShaderPath);

	//----------------------Definitions-----------------------------
	BlockDefinition::InitializeBlockDefinitionsFromFile();
	Chunk::SetBlockAtlasTexture(&BlockDefinition::s_blockSheet->GetTexture());
	m_world = new World(m_player);
}

Game::~Game()
{
	delete m_cube;
	m_cube = nullptr;

	delete m_cube2;
	m_cube2 = nullptr;

	delete m_player;
	m_player = nullptr;

	delete m_sphere;
	m_sphere = nullptr;

	delete m_groundGrid;
	m_groundGrid = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_cubeSkybox;
	m_cubeSkybox = nullptr;

	delete m_world;
	m_world = nullptr;

	BlockDefinition::ShutdownBlockDefinitions();
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
	DebugRenderScreen(m_screenCamera);
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
	XboxController const& controller = g_theInput->GetController(0);
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)||controller.WasButtonJustPressed(XboxButtonID::START))
	{
		m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);

	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F2)||controller.WasButtonJustPressed(XboxButtonID::A))
	{
		m_world->ToggleDebugDraw();
	}
	m_world->Update(deltaTime);
	
	char timeBuffer[256];
	snprintf(timeBuffer, sizeof(timeBuffer),
		"Time: %.2f  FPS: %.2f",
		g_systemClock->GetTotalSeconds(), 1.f/g_systemClock->GetDeltaSeconds());

 	float textWidth = 490.f;
 	float textHeight = 50.f;
 	float margin = 10.f;
 	Vec2 topRight = Vec2(m_screenSize.x - margin - textWidth, m_screenSize.y - margin - textHeight);
 	Vec2 bottomLeft = Vec2(m_screenSize.x - margin, m_screenSize.y - margin);

 	DebugAddScreenText(std::string(timeBuffer),
 		AABB2(topRight, bottomLeft),
 		20.f,
 		Vec2(1.f, 1.f),
 		0.f,
 		Rgba8::WHITE,
 		Rgba8::WHITE);

	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextGameState = GameState::GAME_STATE_ATTRACT;
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
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE,Vec2(m_screenSize.x*0.5f, m_screenSize.y*0.5f));
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	g_theRenderer->BeginCamera(m_player->m_playerCam);

	m_cubeSkybox->Render();
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	m_groundGrid->Render();

	m_world->Render();

	g_theRenderer->EndCamera(m_player->m_playerCam);

	DebugRenderWorld(m_player->m_playerCam);
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

void Game::AddVertsForGroundGrid()
{
	// add verts for ground grid
	for (int i = 1; i <= 101; i++)
	{
		if (i % 5 == 1)
		{
			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.03f, -50.f, -0.03f);
			Vec3 yTr = yBl + Vec3(0.06f, 100.f, 0.06f);
			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8::GREEN);

			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.03f, -0.03f);
			Vec3 xTr = xBl + Vec3(100.f, 0.06f, 0.06f);
			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8::RED);
		}

		else
		{
			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.01f, -50.f, -0.01f);
			Vec3 yTr = yBl + Vec3(0.02f, 100.f, 0.02f);
			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(yBl, yTr), Rgba8(180, 180, 180, 255));

			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.01f, -0.01f);
			Vec3 xTr = xBl + Vec3(100.f, 0.02f, 0.02f);
			AddVertsForAABB3D(m_groundGrid->m_vertexs, AABB3(xBl, xTr), Rgba8(180, 180, 180, 255));
		}

	}
}

void Game::AddVertsForCubes()
{
	//add cube vertexs to m_cube
	//X
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
	//-X
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
	//Y
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
	//-Y
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
	//Z
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
	//-Z
	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);


	//X
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
	//-X
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
	//Y
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
	//-Y
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
	//Z
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
	//-Z
	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);



}

void Game::AddEntityToList(Entity& thisEntity, EntityList& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i] == nullptr)
		{
			list[i] = &thisEntity;
			return;
		}
	}
	list.push_back(&thisEntity);
}













