//#include "Game/Game.hpp"
//#include "App.hpp"
//#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Renderer/Renderer.hpp"
//#include "Engine/Audio/AudioSystem.hpp"
//#include "Engine/Core/FileUtils.hpp"
//#include "Engine/Core/DevConsole.hpp"
//#include "Engine/Core/Clock.hpp"
//#include "Game/Prop.hpp"
//#include "Game/Player.hpp"
//#include "Engine/Core/VertexUtils.hpp"
//#include <Engine/Math/MathUtils.hpp>
//#include <Engine/Core/Time.hpp>
//#include "Engine/Math/AABB3.hpp"
//#include <Engine/Core/DebugRenderSystem.hpp>
//#include "Engine/Math/AABB2.hpp"
//#include "GameCommon.hpp"
//#include "Engine/Window/Window.hpp"
//#include "Engine/Renderer/CubeSkyBox.hpp"
//#include "Engine/Renderer/Shader.hpp"
//
//extern bool g_isDebugDraw;
//extern Renderer* g_theRenderer;
//extern Clock* g_systemClock;
//
//
//
//Game::Game()
//{
//	m_pbrShader=g_theRenderer->CreateShaderFromFile("Data/Shaders/PBRLighting", VertexType::VERTEX_PCUTBN);
//	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
//
//	m_gameClock = new Clock();
//
//	/*m_cube = new Prop(this);
//	m_cube2 = new Prop(this);
//	m_cube->m_position = Vec3(2.f, 2.f, 0.f);
//	m_cube2->m_position = Vec3(-2.f, -2.f, 0.f);
//	m_cube2->m_color = Rgba8(100,100,100,255);*/
//	m_player = new Player(this);
//	m_groundGrid = new Prop(this);
//
//	/*m_sphere = new Prop(this);
//	m_sphere->m_position = Vec3(10.f, -5.f, 1.f);
//	m_sphere->m_texture = m_gridTex;
//	AddVertsForSphere3D_WithTBN(m_sphere->m_vertexs, m_sphere->m_indices,Vec3(0.f,0.f,0.f), 1.f, Rgba8::WHITE);
//	m_sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_n.png");*/
//
//	//AddEntityToList(*m_cube, m_allEntities);
//	//AddEntityToList(*m_cube2, m_allEntities);
//	//AddEntityToList(*m_sphere, m_allEntities);
//
//	AddVertsForGroundGrid();
//	//AddVertsForCubes();
//
//	// PBR Material
//	InitializeMaterialMatrix();
//
//	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
//	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
//	m_screenCamera.SetViewport(viewport);
//	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
//	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));
//	m_player->m_playerCam.SetViewport(viewport);
//
//	std::string logString = "\
//	Mouse x-axis / Right stick x-axis       Yaw\n\
//	Mouse y-axis / Right stick y-axis       Pitch\n\
//	Q / E / Left trigger / right trigger    Roll\n\
//	A / D / Left stick x-axis               Move left or right, relative to player orientation\n\
//	W / S / Left stick y-axis               Move forward or back, relative to player orientation\n\
//	Z / C / Left shoulder / right shoulder  Move down or up, relative to the world\n\
//	H / Start button                        Reset position and orientation to zero\n\
//	Shift / A button                        Increase speed by a factor of 10 while held\n\
//	P                                       Pause the game\n\
//	O                                       Single step frame\n\
//	T                                       Slow motion mode";
//	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString);
//
//	//----------sky box--------------------
//	std::string skyboxPaths[] = {
// 	"Data/Images/SkyBox1/skyhsky_lf.png",
// 	"Data/Images/SkyBox1/skyhsky_rt.png",
// 	"Data/Images/SkyBox1/skyhsky_dn.png",
// 	"Data/Images/SkyBox1/skyhsky_up.png",
// 	"Data/Images/SkyBox1/skyhsky_ft.png",
// 	"Data/Images/SkyBox1/skyhsky_bk.png",
//	};
//
//	std::string skyBoxShaderPath = "Data/Shaders/CubeSkyBox";
//
//	m_cubeSkybox = new CubeSkyBox(g_theRenderer, skyboxPaths,&skyBoxShaderPath);
//}
//
//Game::~Game()
//{
//	//delete m_cube;
//	//m_cube = nullptr;
//
//	//delete m_cube2;
//	//m_cube2 = nullptr;
//
//	delete m_player;
//	m_player = nullptr;
//
//	//delete m_sphere;
//	//m_sphere = nullptr;
//
//	for (Entity* sphere : m_allEntities)
//	{
//		delete sphere;
//	}
//
//	delete m_groundGrid;
//	m_groundGrid = nullptr;
//
//	delete m_gameClock;
//	m_gameClock = nullptr;
//
//	delete m_cubeSkybox;
//	m_cubeSkybox = nullptr;
//}
//
//
//void Game::Update()
//{
//	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();
//	UpdateCamera(deltaSeconds);
//	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
//	{
//		if (g_theDevConsole->GetMode() == HIDDEN)
//		{
//			g_theDevConsole->SetMode(OPEN_FULL);
//			m_isDevConsole = true;
//		}
//		else
//		{
//			g_theDevConsole->SetMode(HIDDEN);
//			m_isDevConsole = false;
//		}
//	}
//	if (m_isAttractMode)
//	{
//		UpdateAttractMode(deltaSeconds);
//		return;
//	}
//	UpdateDeveloperCheats(deltaSeconds);
//	UpdateGameplayMode(deltaSeconds);
//}
//
//void Game::Renderer() const
//{
//	if (m_isAttractMode)
//	{
//		RenderAttractMode();
//		return;
//	}
//	g_theRenderer->BeginCamera(m_player->m_playerCam);
//
//	g_theRenderer->ClearTextureSlots();
//	m_cubeSkybox->Render();
//
//	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
//	Vec3 normalSunDirection = m_sunDirection.GetNormalized();
//	g_theRenderer->SetLightConstants(normalSunDirection, m_sunIntensity, m_ambientIntensity);
//
//	//m_cube->Render();
//	//m_cube2->Render();
//	//m_sphere->Render();
//
//	for (Entity* sphere : m_allEntities)
//	{
//		sphere->Render();
//	}
//
//	m_groundGrid->Render();
//
//	g_theRenderer->EndCamera(m_player->m_playerCam);
//
//	DebugRenderWorld(m_player->m_playerCam);
//
//	g_theRenderer->BeginCamera(m_screenCamera);
//	DebugRenderScreen(m_screenCamera);
//	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
//	g_theRenderer->EndCamera(m_screenCamera);
//}
//
//void Game::UpdateAttractMode(float deltaTime)
//{
//	UNUSED(deltaTime);
//	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
//	{
//		g_theApp->m_isQuitting = true;
//	}
//	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
//	{
//		m_isAttractMode = false;
//	}
//}
//
//void Game::UpdateGameplayMode(float deltaTime)
//{
//	UNUSED(deltaTime);
//	m_player->Update((float)g_systemClock->GetDeltaSeconds());
//	////-----------------------------------------------------------------------------------------
//	//float cube2Color = CosDegrees(50.f * (float)m_gameClock->GetTotalSeconds());
//	//cube2Color =RangeMapClamped(cube2Color, -1.f, 1.f, 0.f, 1.f);
//	//m_cube2->m_color = Rgba8((unsigned char)(cube2Color*255.f), (unsigned char)(cube2Color * 255.f), (unsigned char)(cube2Color * 255.f), 255);
//	////-----------------------------------------------------------------------------------------
//	//m_cube->m_orientation.m_rollDegrees += 30.f * deltaTime;
//	//m_cube->m_orientation.m_pitchDegrees += 30.f * deltaTime;
//	////-----------------------------------------------------------------------------------------
//	//m_sphere->m_orientation.m_yawDegrees += 45.f * deltaTime;
//	////-----------------------------------------------------------------------------------------
//	if (g_theInput->WasKeyJustPressed('1'))
//	{
//		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
//		Vec3 endPoint = m_player->m_position + dir* 20.f;
//		DebugAddWorldLine(m_player->m_position, endPoint, 0.05f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW,DebugRenderMode::X_RAY);
//	}
//	
//	if (g_theInput->WasKeyJustPressed('2'))
//	{
//		DebugAddWorldPoint(Vec3(m_player->m_position.x, m_player->m_position.y,0.f), 0.25f, 60.f, Rgba8(150,75,0), Rgba8(150, 75, 0));
//	}
//
//	if (g_theInput->WasKeyJustPressed('3'))
//	{
//		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
//		Vec3 center = m_player->m_position + dir * 2.f;
//		DebugAddWorldWireSphere(center, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED,DebugRenderMode::USE_DEPTH);
//	}
//
//	if (g_theInput->WasKeyJustPressed('4'))
//	{
//		Mat44 transform = Mat44::MakeTranslation3D(m_player->m_position);
//		transform.Append(m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
//		DebugAddWorldBasis(transform, 10.f);
//	}
//
//	if (g_theInput->WasKeyJustPressed('5'))
//	{
//		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
//		Vec3 center = m_player->m_position + dir * 2.f;
//
//		char buffer[256];
//		snprintf(buffer, sizeof(buffer),
//			"position: %.2f, %.2f, %.2f, orientation: %.2f, %.2f, %.2f",
//			m_player->m_position.x, m_player->m_position.y, m_player->m_position.z,
//			m_player->m_orientation.m_rollDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_yawDegrees);
//		DebugAddWorldBillboardText(std::string(buffer), center, 0.3f, Vec2(0.5f, 0.5f), 10.f, Rgba8::WHITE, Rgba8::RED);
//	}
//
//	if (g_theInput->WasKeyJustPressed('6'))
//	{
//		DebugAddWorldWireCylinder(m_player->m_position - Vec3(0.f, 0.f, 0.2f), m_player->m_position + Vec3(0.f, 0.f, 0.2f), 0.2f, 10.f, Rgba8::WHITE, Rgba8::RED);
//	}
//	
//
//	if (g_theInput->WasKeyJustPressed('7'))
//	{
//		char buffer[256];
//		snprintf(buffer, sizeof(buffer),
//			"player orientation: %.2f, %.2f, %.2f",
//			m_player->m_orientation.m_rollDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_yawDegrees);
//		DebugAddMessage(std::string(buffer), 5.f, Rgba8::HILDA, Rgba8::RED);
//	}
//
//	char timeBuffer[256];
//	snprintf(timeBuffer, sizeof(timeBuffer),
//		"Time: %.2f  FPS: %.2f  Scale: %.2f",
//		g_systemClock->GetTotalSeconds(), 1.f/g_systemClock->GetDeltaSeconds(), 1.f);
//	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 720.f), Vec2(1590.f, 790.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);
//
//	//-----------------------------------------------------------------------------------------
//	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
//	{
//		m_isAttractMode=true;
//	}
//}
//
//void Game::UpdateDeveloperCheats(float deltaTime)
//{
//	UNUSED(deltaTime);
//	AdjustForPauseAndTimeDitortion();
//}
//
//void Game::UpdateCamera(float deltaTime)
//{
//	UNUSED(deltaTime);
//	
//	m_player->m_playerCam.SetPerspectiveView(m_player->m_playerCam.GetViewport().GetDimensions().x/ m_player->m_playerCam.GetViewport().GetDimensions().y, 60.f, 0.1f, 100.f);
//}
//
//void Game::AdjustForPauseAndTimeDitortion()
//{
//	if (g_theInput->WasKeyJustPressed('P'))
//	{
//		m_gameClock->TogglePause();
//	}
//
//	if (g_theInput->WasKeyJustPressed('T'))
//	{
//		m_previousTimeScale = (float)m_gameClock->GetTimeScale();
//	}
//
//	if (g_theInput->IsKeyDown('T'))
//	{
//		m_gameClock->SetTimeScale(0.1f);
//	}
//
//	if (g_theInput->WasKeyJustReleased('T'))
//	{
//		m_gameClock->SetTimeScale(m_previousTimeScale);
//	}
//
//	if (g_theInput->WasKeyJustPressed('O'))
//	{
//		m_gameClock->StepSingleFrame();
//	}
//}
//
//void Game::RenderAttractMode() const
//{
//	g_theRenderer->BeginCamera(m_screenCamera);
//	g_theRenderer->BindTexture(nullptr);
//	DebugDrawRing(4.f, 20.f, Rgba8::WHITE,Vec2(SCREEN_SIZE_X*0.5f,SCREEN_SIZE_Y*0.5f));
//	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
//	g_theRenderer->EndCamera(m_screenCamera);
//}
//
//void Game::RenderUI() const
//{
//	g_theRenderer->BindTexture(nullptr);
//	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
//	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
//}
//
//void Game::RenderDebugMode()const
//{
//
//}
//
//void Game::AddVertsForGroundGrid()
//{
//	// add verts for ground grid
//	for (int i = 1; i <= 101; i++)
//	{
//		if (i % 5 == 1)
//		{
//			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.03f, -50.f, -0.03f);
//			Vec3 yTr = yBl + Vec3(0.06f, 100.f, 0.06f);
//			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs,m_groundGrid->m_indices, yBl,yTr, Rgba8::GREEN,AABB2::ZERO_TO_ONE);
//
//			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.03f, -0.03f);
//			Vec3 xTr = xBl + Vec3(100.f, 0.06f, 0.06f);
//			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, xBl, xTr, Rgba8::RED, AABB2::ZERO_TO_ONE);
//		}
//
//		else
//		{
//			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.01f, -50.f, -0.01f);
//			Vec3 yTr = yBl + Vec3(0.02f, 100.f, 0.02f);
//			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, yBl, yTr, Rgba8(180, 180, 180, 255), AABB2::ZERO_TO_ONE);
//
//			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.01f, -0.01f);
//			Vec3 xTr = xBl + Vec3(100.f, 0.02f, 0.02f);
//
//			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, xBl, xTr, Rgba8(180, 180, 180, 255), AABB2::ZERO_TO_ONE);
//		}
//
//	}
//}
//
////void Game::AddVertsForCubes()
////{
////	//add cube vertexs to m_cube
////	//X
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED, AABB2::ZERO_TO_ONE);
////	//-X
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN, AABB2::ZERO_TO_ONE);
////	//Y
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN, AABB2::ZERO_TO_ONE);
////	//-Y
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA, AABB2::ZERO_TO_ONE);
////	//Z
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE, AABB2::ZERO_TO_ONE);
////	//-Z
////	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW, AABB2::ZERO_TO_ONE);
////
////
////	//X
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs,m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED, AABB2::ZERO_TO_ONE);
////	//-X
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN, AABB2::ZERO_TO_ONE);
////	//Y
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN, AABB2::ZERO_TO_ONE);
////	//-Y
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA, AABB2::ZERO_TO_ONE);
////	//Z
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE, AABB2::ZERO_TO_ONE);
////	//-Z
////	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW, AABB2::ZERO_TO_ONE);
////
////
////
////}
//
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
//
//void Game::InitializeMaterialMatrix()
//{
//	for (int row = 0; row < MATERIAL_MATRIX_ROWS; row++)
//	{
//		for (int col = 0; col < MATERIAL_MATRIX_COLS; col++)
//		{
//			Prop* sphere = new Prop(this);
//
//			float xOffset = col * SPHERE_SPACING;
//			float zOffset = row * SPHERE_SPACING+0.5f;
//			sphere->m_position = Vec3(xOffset, 1.f, zOffset);
//			sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_n.png");
//			AddVertsForSphere3D_WithTBN(sphere->m_vertexs, sphere->m_indices,
//				Vec3(0.f, 0.f, 0.f), 0.8f, Rgba8::WHITE);
//
//			float metallic = col / (float)(MATERIAL_MATRIX_COLS - 1);  // 0.0 到 1.0
//			float roughness = row / (float)(MATERIAL_MATRIX_ROWS - 1); // 0.0 到 1.0
//
//			Vec3 albedo = Vec3(1.f, 0.f, 0.f);
//			sphere->m_pbrMat.SetParameters(metallic, roughness, albedo);
//
//			// 如果有纹理，可以设置
//			// sphere->m_pbrMat.m_albedoTexture = m_gridTex;
//			// sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/normal.png");
//
//			// 添加到列表
//			m_materialSpheres.push_back(sphere);
//			AddEntityToList(*sphere, m_allEntities);
//		}
//	}
//
//	// 可选：添加标签文字说明
//}

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
#include "Engine/Renderer/Shader.hpp"
#include "ThirdParty/imgui/imgui.h"
#include <Engine/Core/NamedProperties.hpp>

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;



Game::Game()
{
	m_pbrShader = g_theRenderer->CreateShaderFromFile("Data/Shaders/PBRLighting", VertexType::VERTEX_PCUTBN);
	m_gridTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_gameClock = new Clock();

	/*m_cube = new Prop(this);
	m_cube2 = new Prop(this);
	m_cube->m_position = Vec3(2.f, 2.f, 0.f);
	m_cube2->m_position = Vec3(-2.f, -2.f, 0.f);
	m_cube2->m_color = Rgba8(100,100,100,255);*/
	m_player = new Player(this);
	m_groundGrid = new Prop(this);

	/*m_sphere = new Prop(this);
	m_sphere->m_position = Vec3(10.f, -5.f, 1.f);
	m_sphere->m_texture = m_gridTex;
	AddVertsForSphere3D_WithTBN(m_sphere->m_vertexs, m_sphere->m_indices,Vec3(0.f,0.f,0.f), 1.f, Rgba8::WHITE);
	m_sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_n.png");*/

	//AddEntityToList(*m_cube, m_allEntities);
	//AddEntityToList(*m_cube2, m_allEntities);
	//AddEntityToList(*m_sphere, m_allEntities);

	AddVertsForGroundGrid();
	//AddVertsForCubes();

	// PBR Material
	InitializeMaterialMatrix();

	// Initialize controllable sphere
	m_controllableSphere = new Prop(this);
	m_controllableSphere->m_position = Vec3(-5.f, 10.f, 2.f);  // 放在矩阵旁边
	AddVertsForSphere3D_WithTBN(m_controllableSphere->m_vertexs, m_controllableSphere->m_indices,
		Vec3(0.f, 0.f, 0.f), 1.f, Rgba8::WHITE);
	m_controllableSphere->m_pbrMat.SetParameters(m_controlSphereMetallic, m_controlSphereRoughness,
		Vec3(m_controlSphereAlbedo[0], m_controlSphereAlbedo[1], m_controlSphereAlbedo[2]));
	AddEntityToList(*m_controllableSphere, m_allEntities);

	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
	m_screenCamera.SetViewport(viewport);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f, 0.f, 0.f));
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

	m_cubeSkybox = new CubeSkyBox(g_theRenderer, skyboxPaths, &skyBoxShaderPath);

	TestNamedProperties();
}

Game::~Game()
{
	//delete m_cube;
	//m_cube = nullptr;

	//delete m_cube2;
	//m_cube2 = nullptr;

	delete m_player;
	m_player = nullptr;

	//delete m_sphere;
	//m_sphere = nullptr;

	for (Entity* sphere : m_allEntities)
	{
		delete sphere;
	}

	// m_controllableSphere already deleted in m_allEntities loop
	m_controllableSphere = nullptr;

	delete m_groundGrid;
	m_groundGrid = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_cubeSkybox;
	m_cubeSkybox = nullptr;
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

	g_theRenderer->ClearTextureSlots();
	//m_cubeSkybox->Render();

	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	Vec3 normalSunDirection = m_sunDirection.GetNormalized();
	g_theRenderer->SetLightConstants(normalSunDirection, m_sunIntensity, m_ambientIntensity);

	//m_cube->Render();
	//m_cube2->Render();
	//m_sphere->Render();

	for (Entity* sphere : m_allEntities)
	{
		sphere->Render();
	}

	m_groundGrid->Render();

	g_theRenderer->EndCamera(m_player->m_playerCam);

	DebugRenderWorld(m_player->m_playerCam);

	// Render ImGui panel
	if (m_showImGuiPanel && g_theRenderer->IsImGuiEnabled())
	{
		g_theRenderer->BeginImguiFrame();

		ImGui::Begin("PBR Material Controller", &m_showImGuiPanel);

		ImGui::Text("Control Sphere Properties");
		ImGui::Separator();

		// Albedo color picker
		ImGui::ColorEdit3("Albedo Color", m_controlSphereAlbedo);

		// Metallic slider
		ImGui::SliderFloat("Metallic", &m_controlSphereMetallic, 0.0f, 1.0f);

		// Roughness slider
		ImGui::SliderFloat("Roughness", &m_controlSphereRoughness, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Lighting Properties");

		// Sun direction
		float sunDir[3] = { m_sunDirection.x, m_sunDirection.y, m_sunDirection.z };
		if (ImGui::DragFloat3("Sun Direction", sunDir, 0.01f, -10.0f, 10.0f))
		{
			m_sunDirection = Vec3(sunDir[0], sunDir[1], sunDir[2]);
		}

		// Sun intensity
		ImGui::SliderFloat("Sun Intensity", &m_sunIntensity, 0.0f, 2.0f);

		// Ambient intensity
		ImGui::SliderFloat("Ambient Intensity", &m_ambientIntensity, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Press 'I' to toggle this panel");

		ImGui::End();

		g_theRenderer->RenderImguiFrame();
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
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_isAttractMode = false;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	m_player->Update((float)g_systemClock->GetDeltaSeconds());

	// Toggle ImGui panel with 'I' key
	if (g_theInput->WasKeyJustPressed('I'))
	{
		m_showImGuiPanel = !m_showImGuiPanel;
	}

	// Update controllable sphere based on ImGui parameters
	if (m_controllableSphere)
	{
		Vec3 albedo(m_controlSphereAlbedo[0], m_controlSphereAlbedo[1], m_controlSphereAlbedo[2]);
		m_controllableSphere->m_pbrMat.SetParameters(m_controlSphereMetallic, m_controlSphereRoughness, albedo);

		// Optional: make it slowly rotate
		m_controllableSphere->m_orientation.m_yawDegrees += 20.f * deltaTime;
	}

	////-----------------------------------------------------------------------------------------
	//float cube2Color = CosDegrees(50.f * (float)m_gameClock->GetTotalSeconds());
	//cube2Color =RangeMapClamped(cube2Color, -1.f, 1.f, 0.f, 1.f);
	//m_cube2->m_color = Rgba8((unsigned char)(cube2Color*255.f), (unsigned char)(cube2Color * 255.f), (unsigned char)(cube2Color * 255.f), 255);
	////-----------------------------------------------------------------------------------------
	//m_cube->m_orientation.m_rollDegrees += 30.f * deltaTime;
	//m_cube->m_orientation.m_pitchDegrees += 30.f * deltaTime;
	////-----------------------------------------------------------------------------------------
	//m_sphere->m_orientation.m_yawDegrees += 45.f * deltaTime;
	////-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed('1'))
	{
		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
		Vec3 endPoint = m_player->m_position + dir * 20.f;
		DebugAddWorldLine(m_player->m_position, endPoint, 0.05f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		DebugAddWorldPoint(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), 0.25f, 60.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0));
	}

	if (g_theInput->WasKeyJustPressed('3'))
	{
		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
		Vec3 center = m_player->m_position + dir * 2.f;
		DebugAddWorldWireSphere(center, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USE_DEPTH);
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
		g_systemClock->GetTotalSeconds(), 1.f / g_systemClock->GetDeltaSeconds(), 1.f);
	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 720.f), Vec2(1590.f, 790.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);

	//-----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isAttractMode = true;
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

	m_player->m_playerCam.SetPerspectiveView(m_player->m_playerCam.GetViewport().GetDimensions().x / m_player->m_playerCam.GetViewport().GetDimensions().y, 60.f, 0.1f, 100.f);
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
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
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
			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, yBl, yTr, Rgba8::GREEN, AABB2::ZERO_TO_ONE);

			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.03f, -0.03f);
			Vec3 xTr = xBl + Vec3(100.f, 0.06f, 0.06f);
			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, xBl, xTr, Rgba8::RED, AABB2::ZERO_TO_ONE);
		}

		else
		{
			Vec3 yBl = Vec3(-50.f + (i - 1) - 0.01f, -50.f, -0.01f);
			Vec3 yTr = yBl + Vec3(0.02f, 100.f, 0.02f);
			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, yBl, yTr, Rgba8(180, 180, 180, 255), AABB2::ZERO_TO_ONE);

			Vec3 xBl = Vec3(-50.f, -50.f + (i - 1) - 0.01f, -0.01f);
			Vec3 xTr = xBl + Vec3(100.f, 0.02f, 0.02f);

			AddVertsForAABB3D_WithTBN(m_groundGrid->m_vertexs, m_groundGrid->m_indices, xBl, xTr, Rgba8(180, 180, 180, 255), AABB2::ZERO_TO_ONE);
		}

	}
}

//void Game::AddVertsForCubes()
//{
//	//add cube vertexs to m_cube
//	//X
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED, AABB2::ZERO_TO_ONE);
//	//-X
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN, AABB2::ZERO_TO_ONE);
//	//Y
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN, AABB2::ZERO_TO_ONE);
//	//-Y
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA, AABB2::ZERO_TO_ONE);
//	//Z
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE, AABB2::ZERO_TO_ONE);
//	//-Z
//	AddVertsForQuad3D_WithTBN(m_cube->m_vertexs, m_cube->m_indices, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW, AABB2::ZERO_TO_ONE);
//
//
//	//X
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs,m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED, AABB2::ZERO_TO_ONE);
//	//-X
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN, AABB2::ZERO_TO_ONE);
//	//Y
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN, AABB2::ZERO_TO_ONE);
//	//-Y
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA, AABB2::ZERO_TO_ONE);
//	//Z
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE, AABB2::ZERO_TO_ONE);
//	//-Z
//	AddVertsForQuad3D_WithTBN(m_cube2->m_vertexs, m_cube2->m_indices, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW, AABB2::ZERO_TO_ONE);
//
//
//
//}

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

void Game::InitializeMaterialMatrix()
{
	for (int row = 0; row < MATERIAL_MATRIX_ROWS; row++)
	{
		for (int col = 0; col < MATERIAL_MATRIX_COLS; col++)
		{
			Prop* sphere = new Prop(this);

			float xOffset = col * SPHERE_SPACING;
			float zOffset = row * SPHERE_SPACING + 0.5f;
			sphere->m_position = Vec3(xOffset, 1.f, zOffset);
			sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/wall3_n.png");
			AddVertsForSphere3D_WithTBN(sphere->m_vertexs, sphere->m_indices,
				Vec3(0.f, 0.f, 0.f), 0.8f, Rgba8::WHITE);

			float metallic = col / (float)(MATERIAL_MATRIX_COLS - 1);  // 0.0 到 1.0
			float roughness = row / (float)(MATERIAL_MATRIX_ROWS - 1); // 0.0 到 1.0

			Vec3 albedo = Vec3(1.f, 0.f, 0.f);
			sphere->m_pbrMat.SetParameters(metallic, roughness, albedo);

			// 如果有纹理，可以设置
			// sphere->m_pbrMat.m_albedoTexture = m_gridTex;
			// sphere->m_pbrMat.m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/normal.png");

			// 添加到列表
			m_materialSpheres.push_back(sphere);
			AddEntityToList(*sphere, m_allEntities);
		}
	}

	// 可选：添加标签文字说明
}

void Game::TestNamedProperties()
{
	// NamedProperties: example usage

	std::string lastName("Eiserloh");
	NamedProperties employmentInfoProperties;
	// ...

	NamedProperties p;
	p.SetValue("Height", 1.93f);
	p.SetValue("Age", 52);
	p.SetValue("IsMarried", true);
	p.SetValue("Position", Vec2(3.5f, 6.2f));
	p.SetValue("EyeColor", Rgba8(77, 38, 23));
	p.SetValue("LastName", lastName);	// Set as std::string data...
	p.SetValue("FirstName", "Squirrel"); 	// Set as c-string (char const*)? Store as std::string!	
	p.SetValue("EmployeeInfo", employmentInfoProperties); // NamedProperties inside NamedProperties!

	float height = p.GetValue("Height", 1.75f);
	int health = p.GetValue("Health", 100); // Returns 100 if “Health” was not present
	std::string m_firstName = p.GetValue("FirstName", m_firstName); // Variable as its own default value
	std::string m_lastName = p.GetValue("LastName", m_lastName);   // This is a common trick
	//int height2 = p.GetValue("Height", 76); // ERROR: Incorrect type!  Data value is float!

	NamedProperties employerInfo = p.GetValue("EmployeeInfo", NamedProperties()); // Nested!
	std::string ssn = employerInfo.GetValue("SSN", "00000000");

	// Note the subtleties in the Set, Get, and return types for each of the following examples:
	std::string unknownNameString = "UNKNOWN NAME";
	std::string lastName2 = p.GetValue("LastName", unknownNameString);
	std::string lastName3 = p.GetValue("LastName", "UNKNOWN"); // Explicit override of GetValue!
	std::string firstName2 = p.GetValue("FirstName", unknownNameString);
	std::string firstName3 = p.GetValue("FirstName", "UNKNOWN"); // Explicit override of SetValue!

	// For backward compatibility with NamedStrings, we need to be able to SetValue as strings but GetValue as typed!
	p.SetValue("GPA", "3.14");           // Actually stored as std::string data value
	p.SetValue("YearBorn", "1973");      // Actually stored as std::string data value
	p.SetValue("Street Address", "2210"); // Actually stored as std::string data value

	float GPA = p.GetValue("GPA", 0.f);        // Explicitly try to fetch std::string data value as float
	int   yearBorn = p.GetValue("YearBorn", 1970);   // Explicitly try to fetch std::string data value as int
	//Vec2* pointer = p.GetValue("Street Address", nullptr); // ERROR: Can't fetch std::string data value as other types!

}











