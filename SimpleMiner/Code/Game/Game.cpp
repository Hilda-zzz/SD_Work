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
#include "TerrainConfig.hpp"
#include "ThirdParty/imgui/imgui_internal.h"
#include "Curve1D.hpp"

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

Game::Game()
{
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
	m_player->m_playerCam.SetPerspectiveView(camAspect, 60.f, 0.01f, 100000.f);

	std::string logString = "\
Mouse x-axis / Right stick x-axis           Yaw\n\
Mouse y-axis / Right stick y-axis           Pitch\n\
A / D / Left stick x-axis                   Move left or right, relative to the camera\n\
W / S / Left stick y-axis                   Move forward or back, relative to the camera\n\
Q / E / Left shoulder / right shoulder      Move up or down, relative to the world\n\
Shift / Left trigger / right trigger        Increase speed while held\n\
C / Dpad Up                                 Toggle camera mode\n\
LMB / X button                              Digs a block\n\
RMB / Y button                              Places a block\n\
1 / Dpad left                               Select glowstone\n\
2 / Dpad down                               Select cobblestone\n\
3 / Dpad right                              Select chiseled brick\n\
F2 / A button                               Toggle debug draw\n\
F8 / B button                               Reload";

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

	// ---------------------------
	if (!FolderExists("Saves"))
	{
		CreateFolder("Saves");
	}
}

Game::~Game()
{
// 	delete m_cube;
// 	m_cube = nullptr;
// 
// 	delete m_cube2;
// 	m_cube2 = nullptr;

	delete m_player;
	m_player = nullptr;

// 	delete m_sphere;
// 	m_sphere = nullptr;

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
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_world->ToggleDebugChunkJobStatInfo();
	}

	RenderTerrainEditor();

	m_world->Update(deltaTime);
	
	AddDebugText();

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

	// m_cubeSkybox->Render();
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

void Game::RenderTerrainEditor()
{
	if (!m_showTerrainEditor)
		return;

	TerrainConfig& config = TerrainConfig::GetInstance();

	ImGui::SetNextWindowSize(ImVec2(450, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);

	ImGui::Begin("Terrain Editor", &m_showTerrainEditor, ImGuiWindowFlags_AlwaysAutoResize);

	// === Debug Visualization Section ===
	if (ImGui::CollapsingHeader("Debug Visualization", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Show Noise Debug", &config.m_debug.m_showNoiseDebug);

		if (config.m_debug.m_showNoiseDebug)
		{
			ImGui::Spacing();
			ImGui::Text("Select Noise Type:");
			ImGui::Separator();

			// --- Raw Noise Group ---
			ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Raw Noise Values:");
			ImGui::Indent();

			bool isNone = (config.m_debug.m_activeDebugMode == NoiseDebugMode::NONE);
			if (ImGui::RadioButton("None##NoiseDebug", isNone))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::NONE;
			}

			bool isContinentRaw = (config.m_debug.m_activeDebugMode == NoiseDebugMode::CONTINENT_RAW);
			if (ImGui::RadioButton("Continent (Raw)##NoiseDebug", isContinentRaw))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::CONTINENT_RAW;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Raw continent noise [-1, 1]");
			}

			bool isErosionRaw = (config.m_debug.m_activeDebugMode == NoiseDebugMode::EROSION_RAW);
			if (ImGui::RadioButton("Erosion (Raw)##NoiseDebug", isErosionRaw))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::EROSION_RAW;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Raw erosion noise [-1, 1]");
			}

			bool isPVRaw = (config.m_debug.m_activeDebugMode == NoiseDebugMode::PEAKS_VALLEYS_RAW);
			if (ImGui::RadioButton("Peaks & Valleys (Raw)##NoiseDebug", isPVRaw))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::PEAKS_VALLEYS_RAW;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Raw PV noise [-1, 1]");
			}

			bool isTempRaw = (config.m_debug.m_activeDebugMode == NoiseDebugMode::TEMPERATURE_RAW);
			if (ImGui::RadioButton("Temperature (Raw)##NoiseDebug", isTempRaw))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::TEMPERATURE_RAW;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Raw temperature noise [-1, 1]");
			}

			bool isHumidRaw = (config.m_debug.m_activeDebugMode == NoiseDebugMode::HUMIDITY_RAW);
			if (ImGui::RadioButton("Humidity (Raw)##NoiseDebug", isHumidRaw))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::HUMIDITY_RAW;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Raw humidity noise [-1, 1]");
			}

			ImGui::Unindent();
			ImGui::Spacing();
			ImGui::Separator();

			// --- Spline Mapped Group ---
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Spline-Mapped Values:");
			ImGui::Indent();

			bool isContinentOffset = (config.m_debug.m_activeDebugMode == NoiseDebugMode::CONTINENT_OFFSET_MAPPED);
			if (ImGui::RadioButton("Continent Offset (Mapped)##NoiseDebug", isContinentOffset))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::CONTINENT_OFFSET_MAPPED;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Height offset from continent spline (blocks)");
			}

			bool isErosionOffset = (config.m_debug.m_activeDebugMode == NoiseDebugMode::EROSION_OFFSET_MAPPED);
			if (ImGui::RadioButton("Erosion Offset (Mapped)##NoiseDebug", isErosionOffset))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::EROSION_OFFSET_MAPPED;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Height offset from erosion spline (blocks)");
			}

			bool isPVOffset = (config.m_debug.m_activeDebugMode == NoiseDebugMode::PV_OFFSET_MAPPED);
			if (ImGui::RadioButton("PV Offset (Mapped)##NoiseDebug", isPVOffset))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::PV_OFFSET_MAPPED;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Height offset from PV spline (blocks)");
			}

			bool isContinentAmp = (config.m_debug.m_activeDebugMode == NoiseDebugMode::CONTINENT_AMPLITUDE_MAPPED);
			if (ImGui::RadioButton("Continent Amplitude (Mapped)##NoiseDebug", isContinentAmp))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::CONTINENT_AMPLITUDE_MAPPED;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("3D noise amplitude from continent spline");
			}

			bool isErosionAmp = (config.m_debug.m_activeDebugMode == NoiseDebugMode::EROSION_AMPLITUDE_MAPPED);
			if (ImGui::RadioButton("Erosion Amplitude (Mapped)##NoiseDebug", isErosionAmp))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::EROSION_AMPLITUDE_MAPPED;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("3D noise amplitude from erosion spline");
			}

			ImGui::Unindent();
			ImGui::Spacing();
			ImGui::Separator();

			// --- NEW: Biome Level Group ---
			ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Biome Level Visualization:");
			ImGui::Indent();

			bool isContinentLevel = (config.m_debug.m_activeDebugMode == NoiseDebugMode::CONTINENT_LEVEL);
			if (ImGui::RadioButton("Continent Level##NoiseDebug", isContinentLevel))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::CONTINENT_LEVEL;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Continentalness levels (0-6): Deep Ocean -> Far Inland\nGrayscale: Black (0) to White (6)");
			}

			bool isErosionLevel = (config.m_debug.m_activeDebugMode == NoiseDebugMode::EROSION_LEVEL);
			if (ImGui::RadioButton("Erosion Level##NoiseDebug", isErosionLevel))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::EROSION_LEVEL;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Erosion levels (0-6): E0 -> E6\nGrayscale: Black (0) to White (6)");
			}

			bool isPVLevel = (config.m_debug.m_activeDebugMode == NoiseDebugMode::PV_LEVEL);
			if (ImGui::RadioButton("Peaks & Valleys Level##NoiseDebug", isPVLevel))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::PV_LEVEL;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("PV levels (0-4): Valleys -> Low -> Mid -> High -> Peaks\nGrayscale: Black (0) to White (4)");
			}

			bool isTempLevel = (config.m_debug.m_activeDebugMode == NoiseDebugMode::TEMPERATURE_LEVEL);
			if (ImGui::RadioButton("Temperature Level##NoiseDebug", isTempLevel))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::TEMPERATURE_LEVEL;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Temperature levels (0-4): T0 (Coldest) -> T4 (Hottest)\nGrayscale: Black (0) to White (4)");
			}

			bool isHumidLevel = (config.m_debug.m_activeDebugMode == NoiseDebugMode::HUMIDITY_LEVEL);
			if (ImGui::RadioButton("Humidity Level##NoiseDebug", isHumidLevel))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::HUMIDITY_LEVEL;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Humidity levels (0-4): H0 (Driest) -> H4 (Wettest)\nGrayscale: Black (0) to White (4)");
			}

			ImGui::Unindent();
			ImGui::Spacing();
			ImGui::Separator();

			// --- NEW: Biome Type Group ---
			ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Biome Type Visualization:");
			ImGui::Indent();

			bool isBiomeType = (config.m_debug.m_activeDebugMode == NoiseDebugMode::BIOME_TYPE);
			if (ImGui::RadioButton("Final Biome Type##NoiseDebug", isBiomeType))
			{
				config.m_debug.m_activeDebugMode = NoiseDebugMode::BIOME_TYPE;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Final biome types (0-15):");
				ImGui::BulletText("Ocean, Deep Ocean, Frozen Ocean");
				ImGui::BulletText("Beach, Snowy Beach");
				ImGui::BulletText("Plains, Snowy Plains, Desert, Savanna");
				ImGui::BulletText("Forest, Jungle, Taiga, Snowy Taiga");
				ImGui::BulletText("Stony Peaks, Snowy Peaks, Badlands");
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Grayscale: Black (Ocean) to White (Badlands)");
				ImGui::EndTooltip();
			}
			 //===========================================
			ImGui::Unindent();
			ImGui::Spacing();
		}
	}

	// ========================================
	// Seeds Section
	// ========================================
	if (ImGui::CollapsingHeader("Seeds", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushItemWidth(150);

		ImGui::InputScalar("Game Seed", ImGuiDataType_U32, &config.m_seeds.m_gameSeed);
		ImGui::InputScalar("Continent Seed", ImGuiDataType_U32, &config.m_seeds.m_continentSeed);
		ImGui::InputScalar("Erosion Seed", ImGuiDataType_U32, &config.m_seeds.m_erosionSeed);
		ImGui::InputScalar("Weirdness Seed", ImGuiDataType_U32, &config.m_seeds.m_weirdnessSeed);
		ImGui::InputScalar("Temperature Seed", ImGuiDataType_U32, &config.m_seeds.m_temperatureSeed);
		ImGui::InputScalar("Humidity Seed", ImGuiDataType_U32, &config.m_seeds.m_humiditySeed);

		ImGui::PopItemWidth();

		if (ImGui::Button("Regenerate All Seeds"))
		{
			//config.m_seeds.RegenerateSeeds();
		}

		ImGui::Separator();
	}

	// ========================================
	// Continent Section
	// ========================================
	if (ImGui::CollapsingHeader("Continent", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable##Continent", &config.m_continent.m_enabled);

		if (config.m_continent.m_enabled)
		{
			ImGui::PushItemWidth(200);

			ImGui::SliderFloat("Scale##Continent", &config.m_continent.m_scale,
				32.0f, 2048.0f, "%.1f");

			ImGui::SliderInt("Octaves##Continent", &config.m_continent.m_octaves,
				1, 32);

			ImGui::PopItemWidth();
		}

		ImGui::Separator();
	}

	// ========================================
	// Erosion Section
	// ========================================
	if (ImGui::CollapsingHeader("Erosion", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable##Erosion", &config.m_erosion.m_enabled);

		if (config.m_erosion.m_enabled)
		{
			ImGui::PushItemWidth(200);

			ImGui::SliderFloat("Scale##Erosion", &config.m_erosion.m_scale,
				32.0f, 2048.0f, "%.1f");

			ImGui::SliderInt("Octaves##Erosion", &config.m_erosion.m_octaves,
				1, 32);

			ImGui::PopItemWidth();
		}

		ImGui::Separator();
	}

	// ========================================
	// Peaks & Valleys Section
	// ========================================
	if (ImGui::CollapsingHeader("Peaks & Valleys", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable##PV", &config.m_peaksValleys.m_enabled);

		if (config.m_peaksValleys.m_enabled)
		{
			ImGui::PushItemWidth(200);

			ImGui::SliderFloat("Scale##PV", &config.m_peaksValleys.m_scale,
				32.0f, 2048.0f, "%.1f");

			ImGui::SliderInt("Octaves##PV", &config.m_peaksValleys.m_octaves,
				1, 32);

			ImGui::SliderFloat("Influence##PV", &config.m_peaksValleys.m_influence,
				0.0f, 50.0f, "%.1f");

			ImGui::PopItemWidth();
		}

		ImGui::Separator();
	}

	// ========================================
	// Temperature Section
	// ========================================
	if (ImGui::CollapsingHeader("Temperature", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable##Temperature", &config.m_temperature.m_enabled);

		if (config.m_temperature.m_enabled)
		{
			ImGui::PushItemWidth(200);

			ImGui::SliderFloat("Scale##Temperature", &config.m_temperature.m_scale,
				32.0f, 2048.0f, "%.1f");

			ImGui::SliderInt("Octaves##Temperature", &config.m_temperature.m_octaves,
				1, 32);

			ImGui::PopItemWidth();
		}

		ImGui::Separator();
	}

	// ========================================
	// Humidity Section
	// ========================================
	if (ImGui::CollapsingHeader("Humidity", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Enable##Humidity", &config.m_humidity.m_enabled);

		if (config.m_humidity.m_enabled)
		{
			ImGui::PushItemWidth(200);

			ImGui::SliderFloat("Scale##Humidity", &config.m_humidity.m_scale,
				32.0f, 2048.0f, "%.1f");

			ImGui::SliderInt("Octaves##Humidity", &config.m_humidity.m_octaves,
				1, 32);

			ImGui::PopItemWidth();
		}

		ImGui::Separator();
	}

	//======================= Splines Section ======================================

	if (ImGui::CollapsingHeader("Terrain Shaping Curves", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped("These curves control how 2D noise maps to terrain height and 3D noise amplitude.");
		ImGui::Spacing();

		// --- 高度偏移曲线 ---
		if (ImGui::CollapsingHeader("Height Offset Curves##ShapingGroup"))
		{
			RenderSplineEditor("Continent Offset",
				config.m_curves.m_continentOffset,
				-1.0f, 1.0f,   // 输入范围
				-60.0f, 100.0f); // 输出范围

			RenderSplineEditor("Erosion Offset",
				config.m_curves.m_erosionOffset,
				-1.0f, 1.0f,
				-40.0f, 30.0f);

			RenderSplineEditor("Peaks & Valleys Offset",
				config.m_curves.m_pvOffset,
				-1.0f, 1.0f,
				-40.0f, 50.0f);
		}

		// --- 3D 噪声振幅曲线 ---
		if (ImGui::CollapsingHeader("3D Noise Amplitude Curves##ShapingGroup"))
		{
			RenderSplineEditor("Continent Amplitude",
				config.m_curves.m_continentAmplitude,
				-1.0f, 1.0f,
				0.0f, 4.0f);

			RenderSplineEditor("Erosion Amplitude",
				config.m_curves.m_erosionAmplitude,
				-1.0f, 1.0f,
				0.0f, 3.0f);
		}

		// --- 垂直压缩曲线 ---
		if (ImGui::CollapsingHeader("Vertical Squeeze Curve##ShapingGroup"))
		{
			RenderSplineEditor("Height Squeeze",
				config.m_curves.m_heightSqueeze,
				0.0f, 128.0f,   // 输入：高度
				0.0f, 3.0f);    // 输出：压缩因子
		}
	}


	// ========================================
	// Action Buttons
	// ========================================
	//ImGui::Spacing();
	//ImGui::Spacing();

	//if (ImGui::Button("Reset to Defaults", ImVec2(200, 0)))
	//{
	//	//config.ResetToDefaults();
	//}

	//ImGui::SameLine();

	//if (ImGui::Button("Apply Changes", ImVec2(200, 0)))
	//{
	//	// Trigger terrain regeneration
	//	// You'll need to implement this in your World class
	//	// m_world->RegenerateAllChunks();

	//	// For now, show a message
	//	ImGui::OpenPopup("Apply Popup");
	//}

	//// Apply confirmation popup
	//if (ImGui::BeginPopupModal("Apply Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	//{
	//	ImGui::Text("Changes will be applied on next world load.");
	//	ImGui::Text("(Full regeneration not yet implemented)");
	//	ImGui::Separator();

	//	if (ImGui::Button("OK", ImVec2(120, 0)))
	//	{
	//		ImGui::CloseCurrentPopup();
	//	}

	//	ImGui::EndPopup();
	//}

	//ImGui::Spacing();

	//// File operations
	//if (ImGui::Button("Save Config", ImVec2(200, 0)))
	//{
	//	//config.SaveToFile("Data/TerrainConfig.xml");
	//}

	//ImGui::SameLine();

	//if (ImGui::Button("Load Config", ImVec2(200, 0)))
	//{
	//	//config.LoadFromFile("Data/TerrainConfig.xml");
	//}

	ImGui::End();
}

void Game::RenderSplineEditor(const char* label, PiecewiseCurve1D* spline,
	float inputMin, float inputMax,
	float outputMin, float outputMax,
	ImVec2 graphSize)
{
	UNUSED(graphSize);
	if (!spline)
		return;

	ImGui::PushID(label);
	if (ImGui::TreeNode(label))
	{
		// ========================================
		// 2. 调试信息（可选，帮助诊断问题）
		// ========================================
		if (ImGui::CollapsingHeader("Debug Info"))
		{
			ImGui::Text("Segments: %zu", spline->GetKeyCount());

			for (size_t i = 0; i < spline->GetKeyCount(); ++i)
			{
				float keyT = spline->GetKeyT(i);
				LinearCurve1D* curve = spline->GetKeyLinearCurve(i);
				if (curve)
				{
					ImGui::Text("Seg %zu: Key=%.2f, [%.2f->%.2f] => [%.2f->%.2f]",
						i, keyT,
						curve->GetStartT(), curve->GetEndT(),
						curve->GetStartV(), curve->GetEndV());

					// 测试几个点
					ImGui::Indent();
					float testPoints[] = { curve->GetStartT(),
										 (curve->GetStartT() + curve->GetEndT()) / 2.0f,
										 curve->GetEndT() };
					for (float t : testPoints)
					{
						float result = spline->Evaluate(t);
						ImGui::Text("  t=%.2f => %.3f", t, result);
					}
					ImGui::Unindent();
				}
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		// ========================================
		// 3. 控制点编辑（简化视图）
		// ========================================
		ImGui::Text("Control Points:");
		ImGui::Spacing();

		size_t numKeys = spline->GetKeyCount();

		// 提取所有控制点用于可视化
		struct ControlPoint {
			float input;
			float output;
			int segmentIndex;
			bool isStart;
		};

		std::vector<ControlPoint> controlPoints;

		for (size_t i = 0; i < numKeys; ++i)
		{
			LinearCurve1D* curve = spline->GetKeyLinearCurve(i);
			if (curve)
			{
				// 添加起点
				ControlPoint start;
				start.input = curve->GetStartT();
				start.output = curve->GetStartV();
				start.segmentIndex = (int)i;
				start.isStart = true;
				controlPoints.push_back(start);

				// 添加终点（如果是最后一个段）
				if (i == numKeys - 1)
				{
					ControlPoint end;
					end.input = curve->GetEndT();
					end.output = curve->GetEndV();
					end.segmentIndex = (int)i;
					end.isStart = false;
					controlPoints.push_back(end);
				}
			}
		}

		// 显示控制点表格
		if (!controlPoints.empty())
		{
			ImGui::Columns(3, "pointColumns", true);
			ImGui::Text("Input (X)");
			ImGui::NextColumn();
			ImGui::Text("Output (Y)");
			ImGui::NextColumn();
			ImGui::Text("Info");
			ImGui::NextColumn();
			ImGui::Separator();

			for (size_t i = 0; i < controlPoints.size(); ++i)
			{
				ImGui::PushID((int)i);

				ControlPoint& point = controlPoints[i];

				// Input 列
				ImGui::PushItemWidth(-1);
				float newInput = point.input;
				if (ImGui::DragFloat("##input", &newInput, 0.01f, inputMin, inputMax))
				{
					// 更新对应的段
					size_t segIdx = point.segmentIndex;
					LinearCurve1D* curve = spline->GetKeyLinearCurve(segIdx);
					if (curve)
					{
						if (point.isStart)
						{
							spline->SetKeyLinearCurve(segIdx, newInput, curve->GetEndT(),
								point.output, curve->GetEndV());
							// 同时更新 Key T
							spline->SetKeyT(segIdx, newInput);
						}
						else
						{
							spline->SetKeyLinearCurve(segIdx, curve->GetStartT(), newInput,
								curve->GetStartV(), point.output);
						}
					}
				}
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				// Output 列
				ImGui::PushItemWidth(-1);
				float newOutput = point.output;
				if (ImGui::DragFloat("##output", &newOutput, 0.1f, outputMin, outputMax))
				{
					// 更新对应的段
					size_t segIdx = point.segmentIndex;
					LinearCurve1D* curve = spline->GetKeyLinearCurve(segIdx);
					if (curve)
					{
						if (point.isStart)
						{
							spline->SetKeyLinearCurve(segIdx, curve->GetStartT(), curve->GetEndT(),
								newOutput, curve->GetEndV());
						}
						else
						{
							spline->SetKeyLinearCurve(segIdx, curve->GetStartT(), curve->GetEndT(),
								curve->GetStartV(), newOutput);
						}
					}
				}
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				// Info 列
				ImGui::Text("Seg %d %s", point.segmentIndex, point.isStart ? "(Start)" : "(End)");
				ImGui::NextColumn();

				ImGui::PopID();
			}

			ImGui::Columns(1);
		}
		else
		{
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No control points. Add a segment below.");
		}

		ImGui::Spacing();
		ImGui::Separator();

		// ========================================
		// 4. 段管理
		// ========================================
		ImGui::Text("Segments: %zu", numKeys);
		ImGui::Spacing();

		int segmentToDelete = -1;

		for (size_t i = 0; i < numKeys; ++i)
		{
			ImGui::PushID((int)i);

			LinearCurve1D* curve = spline->GetKeyLinearCurve(i);
			if (curve)
			{
				ImGui::Text("Segment %zu: [%.2f, %.2f] -> [%.2f, %.2f]",
					i,
					curve->GetStartT(), curve->GetEndT(),
					curve->GetStartV(), curve->GetEndV());
				ImGui::SameLine();

				if (ImGui::SmallButton("Del") && numKeys > 1)
				{
					segmentToDelete = (int)i;
				}
			}

			ImGui::PopID();
		}

		if (segmentToDelete >= 0)
		{
			spline->RemoveKey(segmentToDelete);
		}

		ImGui::Spacing();
		ImGui::Separator();

		// ========================================
		// 5. 添加新段
		// ========================================
		ImGui::Text("Add New Segment:");

		static float newStartT = 0.0f;
		static float newEndT = 1.0f;
		static float newStartV = 0.0f;
		static float newEndV = 1.0f;

		ImGui::PushItemWidth(150);
		ImGui::DragFloat("From T", &newStartT, 0.01f, inputMin, inputMax);
		ImGui::SameLine();
		ImGui::DragFloat("To T", &newEndT, 0.01f, inputMin, inputMax);

		ImGui::DragFloat("From V", &newStartV, 0.1f, outputMin, outputMax);
		ImGui::SameLine();
		ImGui::DragFloat("To V", &newEndV, 0.1f, outputMin, outputMax);
		ImGui::PopItemWidth();

		if (ImGui::Button("Add Segment"))
		{
			LinearCurve1D* newCurve = new LinearCurve1D(newStartT, newEndT, newStartV, newEndV);
			spline->AddKey(newStartT, newCurve);
		}

		ImGui::Spacing();
		ImGui::Separator();

		// ========================================
		// 7. 信息和测试
		// ========================================
		ImGui::Text("Info:");
		ImGui::Text("  Input Range: [%.2f, %.2f]", inputMin, inputMax);

		float minOut, maxOut;
		spline->GetOutputRange(minOut, maxOut);
		ImGui::Text("  Output Range: [%.2f, %.2f]", minOut, maxOut);

		ImGui::Spacing();
		static float testInput = 0.0f;
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat("Test", &testInput, inputMin, inputMax, "Input: %.3f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		float testOutput = spline->Evaluate(testInput);
		ImGui::Text("-> %.4f", testOutput);

		// 在曲线上标记测试点
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "<-- Move to see curve response");

		ImGui::TreePop();
	}
	ImGui::PopID();
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

// void Game::AddVertsForCubes()
// {
// 	//add cube vertexs to m_cube
// 	//X
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
// 	//-X
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
// 	//Y
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
// 	//-Y
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
// 	//Z
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
// 	//-Z
// 	AddVertsForQuad3D(m_cube->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
// 
// 
// 	//X
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::RED);
// 	//-X
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Rgba8::CYAN);
// 	//Y
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Rgba8::GREEN);
// 	//-Y
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Rgba8::MAGNETA);
// 	//Z
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f), Rgba8::BLUE);
// 	//-Z
// 	AddVertsForQuad3D(m_cube2->m_vertexs, Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Rgba8::YELLOW);
// 
// 
// 
// }

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

void Game::AddDebugText()
{
	float margin = 10.f;
	float textHeight = 50.f;
	//float fontSize = 20.f;
	float topY = m_screenSize.y - margin - textHeight;
	float bottomY = m_screenSize.y - margin;

	// Merge all info into one buffer
	char debugInfoBuffer[1024];
	const char* cameraModeName = (m_player->m_isSpectatorFull) ? "SpectatorFull" : "SpectatorXY";
	snprintf(debugInfoBuffer, sizeof(debugInfoBuffer),
		"[LMB] Dig  [RMB] Add  %s  [1] Glowstone  [2] Cobblestone  [3] ChiseledBrick      [C] Camera: %s     Time: %.2f  FPS: %.2f",
		m_player->m_curBlockBrushName.c_str(),
		cameraModeName,
		g_systemClock->GetTotalSeconds(),
		1.f / g_systemClock->GetDeltaSeconds());

	// Display the merged text across the full width
	Vec2 topLeft = Vec2(margin, topY);
	Vec2 bottomRight = Vec2(m_screenSize.x - margin, bottomY);
	DebugAddScreenText(std::string(debugInfoBuffer),
		AABB2(topLeft, bottomRight),
		textHeight,
		Vec2(0.f, 0.5f),  // Left-aligned, vertically centered
		0.f,
		Rgba8::YELLOW,
		Rgba8::YELLOW);

}













