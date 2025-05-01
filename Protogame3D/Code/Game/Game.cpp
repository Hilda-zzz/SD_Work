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

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;

Game::Game()
{
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();

	m_cube = new Prop(this);
	m_cube2 = new Prop(this);
	m_cube->m_position = Vec3(2.f, 2.f, 0.f);
	m_cube2->m_position = Vec3(-2.f, -2.f, 0.f);
	m_cube2->m_color = Rgba8(100,100,100,255);
	m_player = new Player(this);
	m_groundGrid = new Prop(this);

	m_sphere = new Prop(this);
	m_sphere->m_position = Vec3(10.f, -5.f, 1.f);
	m_sphere->m_texture = m_gridTex;
	AddVertsForSphere3D(m_sphere->m_vertexs, m_sphere->m_position, 1.f, Rgba8::WHITE);

	AddEntityToList(*m_cube, m_allEntities);
	AddEntityToList(*m_cube2, m_allEntities);
	AddEntityToList(*m_sphere, m_allEntities);

	AddVertsForGroundGrid();
	AddVertsForCubes();

	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
	m_screenCamera.SetViewport(viewport);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));
	m_player->m_playerCam.SetViewport(viewport);

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
			m_isDevConsole = true;
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
			m_isDevConsole = false;
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
	g_theRenderer->BeginCamera(m_player->m_playerCam);

	m_cubeSkybox->Render();

	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

	m_cube->Render();
	m_cube2->Render();

	m_sphere->Render();

	m_groundGrid->Render();

	g_theRenderer->EndCamera(m_player->m_playerCam);

	DebugRenderWorld(m_player->m_playerCam);

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
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_isAttractMode = false;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	m_player->Update((float)g_systemClock->GetDeltaSeconds());
	//-----------------------------------------------------------------------------------------
	float cube2Color = CosDegrees(50.f * (float)m_gameClock->GetTotalSeconds());
	cube2Color =RangeMapClamped(cube2Color, -1.f, 1.f, 0.f, 1.f);
	m_cube2->m_color = Rgba8((unsigned char)(cube2Color*255.f), (unsigned char)(cube2Color * 255.f), (unsigned char)(cube2Color * 255.f), 255);
	//-----------------------------------------------------------------------------------------
	m_cube->m_orientation.m_rollDegrees += 30.f * deltaTime;
	m_cube->m_orientation.m_pitchDegrees += 30.f * deltaTime;
	//-----------------------------------------------------------------------------------------
	m_sphere->m_orientation.m_yawDegrees += 45.f * deltaTime;
	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed('1'))
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
	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 720.f), Vec2(1590.f, 790.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);

	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isAttractMode=true;
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
	
	m_player->m_playerCam.SetPerspectiveView(m_player->m_playerCam.GetViewport().GetDimensions().x/ m_player->m_playerCam.GetViewport().GetDimensions().y, 60.f, 0.1f, 100.f);
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

void Game::RenderUI() const
{
	g_theRenderer->BindTexture(nullptr);
	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
}

void Game::RenderDebugMode()const
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













