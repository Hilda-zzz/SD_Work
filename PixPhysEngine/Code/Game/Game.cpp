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
#include "SampleImageUtils.hpp"
#include "HerringboneTileset.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Nova2DTestMap.hpp"
#include "GameMap.hpp"
#include "GamePlayer.hpp"
#include "ThirdParty/tracy/tracy/Tracy.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

// 在Game.cpp开头添加验证代码
#ifdef TRACY_ENABLE
#pragma message("✓ Tracy is ENABLED")
#else
#pragma message("✗ Tracy is DISABLED - Check preprocessor!")
#endif

extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

RandomNumberGenerator Game::s_rng = RandomNumberGenerator();

Game::Game()
{
	m_gameClock = new Clock();

	CellMatManager::InitializeMaterials();
	CellMatManager::InitializeMaterialUIInfo();

	m_testUtilImg = new Image("Data/Images/edge_earth_rainforest_ver.png");
	/*m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
	m_sandboxMap = new SandboxMap(m_sandboxPlayer,IntVec2(640,320));

	m_wangTileMap = new WangTileMap(m_sandboxPlayer);*/

	m_testTileset = new HerringboneTileset();
	m_testTileset->LoadFromImage("Data/Images/rainforest.png");

	//HbRegionGenParams params;
	//params.m_regionBottomLeftChunk = IntVec2(0, 0);       // 从 (0,0) chunk 开始
	//params.m_regionSizeInChunks = IntVec2(80, 40);          // 生成 2×2 chunks
	//params.m_tileset = m_testTileset;                     // 使用刚加载的 tileset
	//params.m_randomSeed = 12348;                          // 随机种子

	//// 调用生成器
	//m_generator.InitializeTileGrid(params);

	//TestComputeShader();
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

	if (m_nova2DMap)
	{
		delete m_nova2DMap;
		m_nova2DMap = nullptr;
	}

	if (m_gameWorldMap)
	{
		delete m_gameWorldMap;
		m_gameWorldMap = nullptr;
	}

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_testUtilImg;
	m_testUtilImg = nullptr;
}


void Game::Update()
{
	ZoneScoped;
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

	//------------------------------
	// RenderTileDebugUI();
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
	case GameMode::NOVA2D_TEST:
		m_nova2DMap->Update(deltaTime);
		break;
	case GameMode::GAME_WORLD:
		m_gameWorldMap->Update(deltaTime);
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

	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	DebugDrawRing(4.f, 20.f, Rgba8::WHITE, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));

	//DebugDrawCurrentTile();
	//m_generator.RenderHPixelGrid();
	//m_generator.RenderTileBoundaries();

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);

	RenderGameModeSelectionUI();

	//if (m_testUtilImg)
	//{
	//	std::vector<Vertex_PCU> verts;

	//	// Test parameters - change these to test different transforms
	//	Rotation testRotation = Rotation::ROTATE_0;      // Change to: ROTATE_90, ROTATE_180, ROTATE_270
	//	Symmetry testSymmetry = Symmetry::FLIP_BOTH;          // Change to: FLIP_X, FLIP_Y, FLIP_BOTH

	//	// Display settings
	//	const int imageSize = 16;      // Source image is 512x512
	//	const float displaySize = 21.f; // Display size in screen space
	//	const float startX = 544.f;      // Center on screen (1600/2 - 512/2)
	//	const float startY = 144.f;      // Center on screen (800/2 - 512/2)

	//	// Sample and render each pixel
	//	for (int py = -42; py < 42; py++)
	//	{
	//		for (int px = -42; px <42; px++)
	//		{
	//			// Sample color using SampleImage with transform
	//			Rgba8 m_color = SampleImage(m_testUtilImg, px, py, 1.0f, testRotation, testSymmetry);

	//			// Calculate pixel position in screen space
	//			float pixelSize = 10.f;
	//			float x0 = startX + px * pixelSize;
	//			float y0 = startY + py * pixelSize;
	//			float x1 = x0 + pixelSize;
	//			float y1 = y0 + pixelSize;

	//			// Add quad
	//			verts.push_back(Vertex_PCU(Vec3(x0, y0, 0.f), m_color, Vec2(0.f, 0.f)));
	//			verts.push_back(Vertex_PCU(Vec3(x1, y0, 0.f), m_color, Vec2(1.f, 0.f)));
	//			verts.push_back(Vertex_PCU(Vec3(x1, y1, 0.f), m_color, Vec2(1.f, 1.f)));

	//			verts.push_back(Vertex_PCU(Vec3(x0, y0, 0.f), m_color, Vec2(0.f, 0.f)));
	//			verts.push_back(Vertex_PCU(Vec3(x1, y1, 0.f), m_color, Vec2(1.f, 1.f)));
	//			verts.push_back(Vertex_PCU(Vec3(x0, y1, 0.f), m_color, Vec2(0.f, 1.f)));
	//		}
	//	}
	//	g_theRenderer->BindTexture(nullptr);
	//	g_theRenderer->SetModelConstants();
	//	g_theRenderer->DrawVertexArray(verts);
	//}

	//---------------------------------------
	

	
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
	case GameMode::NOVA2D_TEST:
		m_nova2DMap->Render();
		break;
	case GameMode::GAME_WORLD:
		m_gameWorldMap->Render();
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
		AABB2(Vec2(50.f, 700.f), Vec2(400.f, 780.f)), 18.f, Rgba8::CYAN, 0.7f, Vec2(0.f, 1.f));

	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->DrawVertexArray(title);

	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameModeSelectionUI() const
{
	// ImGui Game Mode Selection Window
	ImGui::SetNextWindowPos(ImVec2(SCREEN_SIZE_X * 0.8f - 150.f, SCREEN_SIZE_Y * 0.8f - 100.f), ImGuiCond_Always);
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
	if (ImGui::Button("Nova2D Map Mode", ImVec2(buttonWidth, buttonHeight)))
	{
		const_cast<Game*>(this)->m_selectedGameMode = GameMode::NOVA2D_TEST;
		const_cast<Game*>(this)->m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}

	ImGui::Spacing();

	ImGui::SetCursorPosX(buttonPosX);
	if (ImGui::Button("Game World Mode", ImVec2(buttonWidth, buttonHeight)))
	{
		const_cast<Game*>(this)->m_selectedGameMode = GameMode::GAME_WORLD;
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
	switch (m_selectedGameMode)
	{
	case GameMode::SANDBOX:
		m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
		m_sandboxPlayer->InitCamera(IntVec2(640, 320));
		m_sandboxMap = new SandboxMap(m_sandboxPlayer, IntVec2(640, 320));
		m_sandboxMap->Initialize();
		break;
	case GameMode::WANG_TILE_MAP:
		m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
		m_sandboxPlayer->InitCamera(IntVec2(640, 320));
		m_wangTileMap = new WangTileMap(m_sandboxPlayer);
		m_wangTileMap->Initialize();
		break;
	case GameMode::NOVA2D_TEST:
		m_sandboxPlayer = new SandboxPlayer(IntVec2(640, 320));
		m_sandboxPlayer->InitCamera(IntVec2(640, 320));
		m_nova2DMap = new Nova2DTestMap(m_sandboxPlayer);
		m_nova2DMap->Initialize();
		break;
	case GameMode::GAME_WORLD:
		m_gamePlayer = new GamePlayer(IntVec2(640, 320),Vec2(1024.f,1024.f));
		m_gamePlayer->InitCamera(IntVec2(640, 320));
		m_gameWorldMap = new GameMap(m_gamePlayer,IntVec2(6,6)); 
		m_gameWorldMap->SetHerringboneTileset(m_testTileset);
		m_gameWorldMap->Initialize();
		m_gamePlayer->SetGameMap(m_gameWorldMap);

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
	switch (m_selectedGameMode)
	{
	case GameMode::SANDBOX:
		delete m_sandboxMap;
		m_sandboxMap = nullptr;
		delete m_sandboxPlayer;
		m_sandboxPlayer = nullptr;
		break;
	case GameMode::WANG_TILE_MAP:
		delete m_wangTileMap;
		m_wangTileMap = nullptr;
		delete m_sandboxPlayer;
		m_sandboxPlayer = nullptr;
		break;
	case GameMode::NOVA2D_TEST:
		delete m_nova2DMap;
		m_nova2DMap = nullptr;
		delete m_sandboxPlayer;
		m_sandboxPlayer = nullptr;
		break;
	case GameMode::GAME_WORLD:
		delete m_gameWorldMap;
		m_gameWorldMap = nullptr;
		delete m_gamePlayer;
		m_gamePlayer = nullptr;
		break;
	default:
		break;
	}
}

void Game::DebugDrawCurrentTile() const {
	if (!m_testTileset || m_testTileIndex < 0 || m_testTileIndex >= m_testTileset->GetTileCount()) {
		return;
	}

	const HerringboneTile* tile = m_testTileset->GetTile(m_testTileIndex);
	if (!tile) return;

	// 获取tile信息
	IntVec2 contentSize = tile->GetContentSize();
	HbTileOrientation orientation = tile->GetOrientation();

	// 设置绘制位置（屏幕中心）
	Vec2 screenCenter = Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f);
	float scale = 4.0f;  // 放大倍数，便于观察

	// 准备顶点数据
	std::vector<Vertex_PCU> vertices;

	// 外边框（白色）
	AABB2 tileBounds(
		screenCenter.x - (contentSize.x * scale * 0.5f),
		screenCenter.y - (contentSize.y * scale * 0.5f),
		screenCenter.x + (contentSize.x * scale * 0.5f),
		screenCenter.y + (contentSize.y * scale * 0.5f)
	);
	AddVertsForAABB2D(vertices, tileBounds, Rgba8::MAGNETA);  // 线框
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());

	vertices.clear();
	// 绘制tile内容（每个像素一个四边形）
	for (int y = 0; y < contentSize.y; ++y) {
		for (int x = 0; x < contentSize.x; ++x) {
			Rgba8 pixelColor = tile->GetContentPixel(x, y);

			// 计算屏幕位置（左下角为原点）
			float left = screenCenter.x - (contentSize.x * scale * 0.5f) + (x * scale);
			float bottom = screenCenter.y - (contentSize.y * scale * 0.5f) + (y * scale);
			float right = left + scale;
			float top = bottom + scale;

			AABB2 pixelBox(left, bottom, right, top);
			AddVertsForAABB2D(vertices, pixelBox, pixelColor);
		}
	}

	// 渲染tile内容
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());

	// 绘制边缘约束标记（6个彩色点）
	vertices.clear();
	float markerSize = scale * 2.0f;

	if (orientation == HbTileOrientation::HORIZONTAL) {
		// 横向tile的6个边缘标记位置
		Vec2 markerPositions[6] = {
			Vec2(screenCenter.x - contentSize.x * scale * 0.25f, tileBounds.m_maxs.y + 10.0f),  // 顶部左
			Vec2(screenCenter.x + contentSize.x * scale * 0.25f, tileBounds.m_maxs.y + 10.0f),  // 顶部右
			Vec2(tileBounds.m_maxs.x + 10.0f, screenCenter.y),                                   // 右边
			Vec2(screenCenter.x + contentSize.x * scale * 0.25f, tileBounds.m_mins.y - 10.0f),  // 底部右
			Vec2(screenCenter.x - contentSize.x * scale * 0.25f, tileBounds.m_mins.y - 10.0f),  // 底部左
			Vec2(tileBounds.m_mins.x - 10.0f, screenCenter.y)                                    // 左边
		};

		for (int i = 0; i < 6; ++i) {
			HbEdgeConstraint edge = tile->GetEdgeConstraint(i);
			AABB2 markerBox(
				markerPositions[i].x - markerSize,
				markerPositions[i].y - markerSize,
				markerPositions[i].x + markerSize,
				markerPositions[i].y + markerSize
			);
			AddVertsForAABB2D(vertices, markerBox, edge.m_color);
		}
	}
	else {
		// 纵向tile的6个边缘标记位置
		Vec2 markerPositions[6] = {
			Vec2(screenCenter.x, tileBounds.m_maxs.y + 10.0f),                                   // 顶部
			Vec2(tileBounds.m_maxs.x + 10.0f, screenCenter.y + contentSize.y * scale * 0.25f),  // 右上
			Vec2(tileBounds.m_maxs.x + 10.0f, screenCenter.y - contentSize.y * scale * 0.25f),  // 右下
			Vec2(screenCenter.x, tileBounds.m_mins.y - 10.0f),                                   // 底部
			Vec2(tileBounds.m_mins.x - 10.0f, screenCenter.y - contentSize.y * scale * 0.25f),  // 左下
			Vec2(tileBounds.m_mins.x - 10.0f, screenCenter.y + contentSize.y * scale * 0.25f)   // 左上
		};

		for (int i = 0; i < 6; ++i) {
			HbEdgeConstraint edge = tile->GetEdgeConstraint(i);
			AABB2 markerBox(
				markerPositions[i].x - markerSize,
				markerPositions[i].y - markerSize,
				markerPositions[i].x + markerSize,
				markerPositions[i].y + markerSize
			);
			AddVertsForAABB2D(vertices, markerBox, edge.m_color);
		}
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());

	//// 绘制文本信息
	//std::vector<Vertex_PCU> textVerts;
	//BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	//std::string infoText = Stringf("Tile %d/%d | %s | Size: %dx%d",
	//	m_testTileIndex + 1,
	//	m_testTileset->GetTileCount(),
	//	orientation == HbTileOrientation::HORIZONTAL ? "HORIZONTAL" : "VERTICAL",
	//	contentSize.x,
	//	contentSize.y
	//);

	//font->AddVertsForText2D(textVerts, Vec2(10.0f, SCREEN_SIZE_Y - 30.0f), 20.0f, infoText, Rgba8::WHITE);

	//g_theRenderer->BindTexture(&font->GetTexture());
	//g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
}

void Game::RenderTileDebugUI() {
	if (!m_testTileset) return;

	ImGui::Begin("Herringbone Tile Debugger");

	int tileCount = m_testTileset->GetTileCount();

	// Tile选择器
	ImGui::Text("Tile Selection");
	ImGui::SliderInt("Tile Index", &m_testTileIndex, 0, tileCount - 1);

	// 前一个/后一个按钮
	if (ImGui::Button("< Previous")) {
		m_testTileIndex = (m_testTileIndex - 1 + tileCount) % tileCount;
	}
	ImGui::SameLine();
	if (ImGui::Button("Next >")) {
		m_testTileIndex = (m_testTileIndex + 1) % tileCount;
	}

	ImGui::Separator();

	// 当前Tile信息
	if (m_testTileIndex >= 0 && m_testTileIndex < tileCount) {
		const HerringboneTile* tile = m_testTileset->GetTile(m_testTileIndex);
		if (tile) {
			ImGui::Text("Tile ID: %d", tile->GetTileID());
			ImGui::Text("Orientation: %s",
				tile->GetOrientation() == HbTileOrientation::HORIZONTAL ? "Horizontal (40x20)" : "Vertical (20x40)");

			IntVec2 size = tile->GetContentSize();
			ImGui::Text("Content Size: %d x %d", size.x, size.y);

			ImGui::Separator();
			ImGui::Text("Edge Constraints:");

			for (int i = 0; i < 6; ++i) {
				HbEdgeConstraint edge = tile->GetEdgeConstraint(i);
				Rgba8 color = edge.m_color;

				ImGui::PushID(i);
				ImGui::Text("Edge %d:", i);
				ImGui::SameLine();

				// 显示颜色方块
				ImVec4 imColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
				ImGui::ColorButton("##color", imColor, ImGuiColorEditFlags_NoTooltip);
				ImGui::SameLine();

				ImGui::Text("RGB(%d,%d,%d) Hash:%08X", color.r, color.g, color.b, edge.m_colorHash);
				ImGui::PopID();
			}
		}
	}

	ImGui::Separator();

	// 整体统计
	ImGui::Text("Tileset Statistics");
	ImGui::Text("Total Tiles: %d", tileCount);

	// 统计横向/纵向数量
	int horizontalCount = 0;
	int verticalCount = 0;
	for (int i = 0; i < tileCount; ++i) {
		const HerringboneTile* tile = m_testTileset->GetTile(i);
		if (tile) {
			if (tile->GetOrientation() == HbTileOrientation::HORIZONTAL) {
				horizontalCount++;
			}
			else {
				verticalCount++;
			}
		}
	}
	ImGui::Text("Horizontal Tiles: %d", horizontalCount);
	ImGui::Text("Vertical Tiles: %d", verticalCount);

	ImGui::End();
}

void Game::TestComputeShader()
{
	// ===== 1. 准备测试数据 =====
	const int TEST_COUNT = 1000;
	std::vector<float> inputData(TEST_COUNT);

	// 填充输入数据：0, 1, 2, 3, ..., 999
	for (int i = 0; i < TEST_COUNT; ++i)
	{
		inputData[i] = (float)i;
		//inputData[i] = 1;
	}
	//inputData[0] = 1000;
	// ===== 2. 创建Buffers =====
	StructuredBuffer* inputBuffer = g_theRenderer->CreateStructuredBuffer(
		TEST_COUNT,
		sizeof(float),
		false  // 只读（SRV）
	);

	StructuredBuffer* outputBuffer = g_theRenderer->CreateStructuredBuffer(
		TEST_COUNT,
		sizeof(float),
		true  // 可读写（UAV）
	);

	// ===== 3. 上传数据 =====
	g_theRenderer->CopyDataToStructuredBuffer(inputBuffer, inputData.data(), TEST_COUNT);

	//std::vector<float> verifyData(TEST_COUNT);
	//g_theRenderer->ReadStructuredBuffer(inputBuffer, verifyData.data(), TEST_COUNT);

	//DebuggerPrintf("Verify upload: [0]=%.2f, [1]=%.2f, [999]=%.2f\n",
	//	verifyData[0], verifyData[1], verifyData[999]);

	// ===== 4. 创建参数Buffer =====
	struct TestParams
	{
		float multiplier;
		float addValue;
		uint32_t elementCount;
		uint32_t padding;
	};

	TestParams params;
	params.multiplier = 2.0f;
	params.addValue = 10.0f;
	params.elementCount = TEST_COUNT;
	params.padding = 0;

	ConstantBuffer* paramsCBO = g_theRenderer->CreateConstantBuffer(sizeof(TestParams));
	g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(TestParams), paramsCBO);

	// ===== 5. 加载Compute Shader =====
	ComputeShader* testShader = g_theRenderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/TestComputeShader");

	// ===== 6. 绑定并执行 =====
	g_theRenderer->BindComputeShader(testShader);
	g_theRenderer->BindStructuredBufferSRV(0, inputBuffer);   // t0
	g_theRenderer->BindStructuredBufferUAV(0, outputBuffer);  // u0
	g_theRenderer->BindConstantBufferCS(0, paramsCBO);          // b0

	// 计算线程组数：1000个元素，每组256线程 = 需要4组
	int threadGroups = (TEST_COUNT + 255) / 256;
	g_theRenderer->Dispatch(threadGroups, 1, 1);

	g_theRenderer->UnbindComputeShader();
	g_theRenderer->UnbindStructuredBuffers();

	// ===== 7. 读回结果验证 =====
	std::vector<float> outputData(TEST_COUNT);
	g_theRenderer->ReadStructuredBuffer(outputBuffer, outputData.data(), TEST_COUNT);

	// ===== 8. 验证结果 =====
	bool testPassed = true;
	for (int i = 0; i < TEST_COUNT; ++i)
	{
		float expected = (float)i * 2.0f + 10.0f;  // input * 2 + 10
		float actual = outputData[i];

		if (fabsf(expected - actual) > 0.001f)
		{
			DebuggerPrintf("TEST FAILED at index %d: expected %.2f, got %.2f\n",
				i, expected, actual);
			testPassed = false;
			//break;
		}
	}

	if (testPassed)
	{
		DebuggerPrintf("Compute Shader Test PASSED!\n");
		DebuggerPrintf("   Input[0] = %.2f → Output[0] = %.2f (expected %.2f)\n",
			inputData[0], outputData[0], 10.0f);
		DebuggerPrintf("   Input[999] = %.2f → Output[999] = %.2f (expected %.2f)\n",
			inputData[999], outputData[999], 999.0f * 2.0f + 10.0f);
	}

	// ===== 9. 清理 =====
	delete inputBuffer;
	delete outputBuffer;
	delete paramsCBO;
}










