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
#include "ActorDefinition.hpp"
#include "WeaponDefinition.hpp"
#include "PlayerController.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"

extern bool g_isDebugDraw;
extern Renderer* g_theRenderer;
extern Clock* g_systemClock;

GameState Game::m_curGameState = GameState::ATTRACT;
GameState Game::m_nextState = GameState::ATTRACT;

SpriteSheet* g_terrianSpriteSheet = nullptr;

Game::Game()
{
	ParseGameConfig();
	m_gameClock = new Clock();
	m_font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	m_menuBtnFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/DariniaFont");

	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y)));
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_screenCamera.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f,0.f,0.f));

	std::string logString = "\
	Mouse                               Aim\n\
	A / D / Left stick x-axis           Move \n\
	W / S / Left stick y-axis           Move \n\
	Z / C                               Elevate\n\
	1                                   Pistol\n\
	2                                   PlasmaRifle\n\
    L/R Arrow                           Change Weapon\n\
	Shift / A button                    Sprint\n\
	P                                   Pause the game\n\
	O                                   Single step frame\n\
	T                                   Slow motion mode";
	g_theDevConsole->AddLine(DevConsole::EVENT_FEEDBACK, logString);

	Texture* tileMapTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_terrianSpriteSheet = new SpriteSheet(*tileMapTexture, IntVec2(8, 8));

	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Attract.png");
	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenu.png");

	InitializeMenuButtons();

	TileDefinition::InitializeTileDefinitionFromFile();
	MapDefinition::InitializeMapDefinitionFromFile();

	ActorDefinition::InitializeProjectileActorDefinitionFromFile();
	WeaponDefinition::InitializeWeaponDefinitionFromFile();
	ActorDefinition::InitializeActorDefinitionFromFile();

	SoundID menuSoundID = g_theAudio->CreateOrGetSound(m_mainMenuMusicPath, true);
	m_menuMusic=g_theAudio->StartSound(menuSoundID, true, m_musicVolume);
}

Game::~Game()
{
	m_gameClock->m_parent->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;

	delete g_terrianSpriteSheet;
	g_terrianSpriteSheet = nullptr;

	delete m_playerController0;
	m_playerController0 = nullptr;

	for (ActorDefinition* def : ActorDefinition::s_actorDefinitions)
	{
		delete def;
	}
	ActorDefinition::s_actorDefinitions.clear();

	for (ActorDefinition* def : ActorDefinition::s_projectileActorDefinitions)
	{
		delete def;
	}
	ActorDefinition::s_projectileActorDefinitions.clear();

}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

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
	case GameState::Menu:
		UpdateMenuMode();
		break;
	case GameState::ATTRACT:
		UpdateAttractMode(deltaSeconds);
		break;
	case GameState::LOBBY:
		UpdateLobbyMode(deltaSeconds);
		break;
	case GameState::GOLD:
		UpdateGameplayMode(deltaSeconds);
		break;
	case GameState::PLAYING:
		UpdateGameplayMode(deltaSeconds);
		break;
	case GameState::COUNT:
		break;
	default:
		break;
	}

	UpdateDeveloperCheats();
}

void Game::Renderer() const
{
	switch (m_curGameState)
	{
	case GameState::NONE:
		break;
	case GameState::Menu:
		RenderMenuMode();
		break;
	case GameState::ATTRACT:
		RenderAttractMode();
		break;
	case GameState::LOBBY:
		RenderLobbyMode();
		break;
	case GameState::GOLD:
		RenderPlayingMode();
		break;
	case GameState::PLAYING:
		RenderPlayingMode();
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
	case GameState::GOLD:
		EnterPlayingMode(true);
		break;
	case GameState::PLAYING:
		EnterPlayingMode(false);
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
		ExitLobbyMode();
		break;
	case GameState::GOLD:
		ExitPlayingMode();
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

	SoundID menuSoundID = g_theAudio->CreateOrGetSound(m_mainMenuMusicPath, true);
	if (!g_theAudio->IsPlaying(m_menuMusic))
	{
		m_menuMusic = g_theAudio->StartSound(menuSoundID, true, m_musicVolume);
	}
	
}

void Game::EnterPlayingMode(bool isGold)
{
	g_theAudio->StopSound(m_menuMusic);

	SoundID gameSoundID = g_theAudio->CreateOrGetSound(m_gameMusicPath, true);
	m_gameMusic = g_theAudio->StartSound(gameSoundID, true, m_musicVolume);

	//m_nextState = GameState::PLAYING;
	g_theApp->SetCursorMode(CursorMode::FPS);

	m_maps.reserve(10);
	Map* map1 = new Map(this, &MapDefinition::s_mapDefinitions[0], false);
	Map* map2 = new Map(this, &MapDefinition::s_mapDefinitions[1], true);
	Map* map3 = new Map(this, &MapDefinition::s_mapDefinitions[2], true);

	m_maps.push_back(map1);
	m_maps.push_back(map2);
	m_maps.push_back(map3);

	if (isGold)
	{
		m_curMapIndex = 2;
		m_curMap = m_maps[m_curMapIndex];
		m_curMap->m_isGold = true;
	}
	else
	{
		m_curMapIndex = 0;
		m_curMap = m_maps[m_curMapIndex];
		m_curMap->m_isGold = false;
	}
	


	if (m_playerController0)
	{
		m_playerController0->m_curMap = m_curMap;
	}
	if (m_playerController1)
	{
		m_playerController1->m_curMap = m_curMap;
	}

	m_curMap->InitializeMap();
}

void Game::ExitLobbyMode()
{
	
}

void Game::ExitPlayingMode()
{
	g_theAudio->StopSound(m_gameMusic);
	for (Map* map : m_maps)
	{
		delete map;
		map = nullptr;
	}
	m_maps.clear();

	if (m_playerController0)
	{
		delete m_playerController0;
		m_playerController0 = nullptr;
	}

	if (m_playerController1)
	{
		delete m_playerController1;
		m_playerController1 = nullptr;
	}
}

bool Game::BtnEvent_NormalMode(EventArgs& args)
{
	UNUSED(args);
	m_nextState = GameState::LOBBY;
	return true;
}

bool Game::BtnEvent_GoldlMode(EventArgs& args)
{
	UNUSED(args);
	m_nextState = GameState::GOLD;
	return true;
}

bool Game::BtnEvent_TutorialMode(EventArgs& args)
{
	UNUSED(args);
	m_nextState = GameState::PLAYING;
	return true;
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);

	XboxController const& controller0 = g_theInput->GetController(0);
	XboxController const& controller1 = g_theInput->GetController(1);

	SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSoundPath, true);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theAudio->StartSound(clickSoundID);
		g_theApp->m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		g_theAudio->StartSound(clickSoundID);
		m_nextState = GameState::Menu;
		if (m_playerController0 == nullptr)
		{
			m_playerController0 = new PlayerController(nullptr);
			m_playerController0->SetPlayerIndexAndControllerID(0, -1);
		}
	}

	//Controller Input
	if (controller0.IsConnected())
	{
		if (controller0.WasButtonJustPressed(XboxButtonID::BACK))
		{
			g_theAudio->StartSound(clickSoundID);
			g_theApp->m_isQuitting = true;
		}
		if (m_playerController0 == nullptr && controller0.WasButtonJustPressed(XboxButtonID::START))
		{
			g_theAudio->StartSound(clickSoundID);
			m_nextState = GameState::LOBBY;
			m_playerController0 = new PlayerController(nullptr);
			m_playerController0->SetPlayerIndexAndControllerID(0, 0);
		}
	}

	if (controller1.IsConnected())
	{
		if (controller1.WasButtonJustPressed(XboxButtonID::BACK))
		{
			g_theAudio->StartSound(clickSoundID);
			g_theApp->m_isQuitting = true;
		}
		if (m_playerController0 == nullptr && controller1.WasButtonJustPressed(XboxButtonID::START))
		{
			g_theAudio->StartSound(clickSoundID);
			m_nextState = GameState::LOBBY;
			m_playerController0 = new PlayerController(nullptr);
			m_playerController0->SetPlayerIndexAndControllerID(0, 1);
		}
	}

}

void Game::UpdateLobbyMode(float deltaTime)
{
	//m_lobbyTextVerts.clear();
	UNUSED(deltaTime);

	XboxController const& controller0 = g_theInput->GetController(0);
	XboxController const& controller1 = g_theInput->GetController(1);

	SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSoundPath, true);

	//KB Input
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		//back to atrract mode
		if (!(m_playerController0 && m_playerController1))
		{
			if (m_playerController0)
			{
				delete m_playerController0;
				m_playerController0 = nullptr;
			}

			if (m_playerController1)
			{
				delete m_playerController1;
				m_playerController1 = nullptr;
			}
			m_nextState = GameState::ATTRACT;
		}

		//pop up the specific player
		if (m_playerController0 != nullptr&&m_playerController0->GetPlayerControllerID()==-1&&m_playerController1)
		{
			g_theAudio->StartSound(clickSoundID);
			delete m_playerController0;
			m_playerController0 = nullptr;
		}
		if (m_playerController1 != nullptr && m_playerController1->GetPlayerControllerID() == -1 && m_playerController0)
		{
			g_theAudio->StartSound(clickSoundID);
			delete m_playerController1;
			m_playerController1 = nullptr;
		}

	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		//if is first player, enter the game
		if (m_playerController0&&m_playerController0->GetPlayerControllerID() == -1)
		{
			g_theAudio->StartSound(clickSoundID);
			m_nextState = GameState::PLAYING;
		}
		else if(m_playerController1&& m_playerController1->GetPlayerControllerID() == -1)
		{
			g_theAudio->StartSound(clickSoundID);
			m_nextState = GameState::PLAYING;
		}
		else
		{
			if (!m_playerController1)
			{
				g_theAudio->StartSound(clickSoundID);
				m_playerController1 = new PlayerController(m_curMap);
				m_playerController1->SetPlayerIndexAndControllerID(1, -1);
			}
			if (!m_playerController0)
			{
				g_theAudio->StartSound(clickSoundID);
				m_playerController0 = new PlayerController(m_curMap);
				m_playerController0->SetPlayerIndexAndControllerID(0, -1);
			}
		}
	}
	
	//Controller Input
	if (controller0.IsConnected())
	{
		if (controller0.WasButtonJustPressed(XboxButtonID::BACK))
		{
			if (m_playerController0 != nullptr && m_playerController0->GetPlayerControllerID() == 0 && m_playerController1)
			{
				g_theAudio->StartSound(clickSoundID);
				delete m_playerController0;
				m_playerController0 = nullptr;
			}
			if (m_playerController1 != nullptr && m_playerController1->GetPlayerControllerID() == 0 && m_playerController0)
			{
				g_theAudio->StartSound(clickSoundID);
				delete m_playerController1;
				m_playerController1 = nullptr;
			}
		}
		if (controller0.WasButtonJustPressed(XboxButtonID::START))
		{
			//if is first player, enter the game
			if (m_playerController0&&m_playerController0->GetPlayerControllerID() == 0)
			{
				g_theAudio->StartSound(clickSoundID);
				m_nextState = GameState::PLAYING;
			}
			else if (m_playerController1 && m_playerController1->GetPlayerControllerID() == 0)
			{
				g_theAudio->StartSound(clickSoundID);
				m_nextState = GameState::PLAYING;
			}
			else
			{
				if (!m_playerController1)
				{
					g_theAudio->StartSound(clickSoundID);
					m_playerController1 = new PlayerController(m_curMap);
					m_playerController1->SetPlayerIndexAndControllerID(1, 0);
				}
				if (!m_playerController0)
				{
					g_theAudio->StartSound(clickSoundID);
					m_playerController0 = new PlayerController(m_curMap);
					m_playerController0->SetPlayerIndexAndControllerID(0, 0);
				}
			}
		}

	}

	if (controller1.IsConnected())
	{
		if (controller1.WasButtonJustPressed(XboxButtonID::BACK))
		{
			if (m_playerController0 != nullptr && m_playerController0->GetPlayerControllerID() == 1)
			{
				g_theAudio->StartSound(clickSoundID);
				delete m_playerController0;
				m_playerController0 = nullptr;
			}
			if (m_playerController1 != nullptr && m_playerController1->GetPlayerControllerID() == 1)
			{
				g_theAudio->StartSound(clickSoundID);
				delete m_playerController1;
				m_playerController1 = nullptr;
			}
		}
		if (controller1.WasButtonJustPressed(XboxButtonID::START))
		{
			//if is first player, enter the game
			if (m_playerController0->GetPlayerControllerID() == 1)
			{
				g_theAudio->StartSound(clickSoundID);
				m_nextState = GameState::PLAYING;
			}
			else if (m_playerController1 && m_playerController1->GetPlayerControllerID() == 1)
			{
				g_theAudio->StartSound(clickSoundID);
				m_nextState = GameState::PLAYING;
			}
			else
			{
				if (!m_playerController1)
				{
					g_theAudio->StartSound(clickSoundID);
					m_playerController1 = new PlayerController(m_curMap);
					m_playerController1->SetPlayerIndexAndControllerID(1, 1);
				}
				if (!m_playerController0)
				{
					g_theAudio->StartSound(clickSoundID);
					m_playerController0 = new PlayerController(m_curMap);
					m_playerController0->SetPlayerIndexAndControllerID(0, 1);
				}
			}
		}
	}

	AddTextForLobby();
// 	//if only one player, center the text box
// 	if (!m_playerController1)
// 	{
// 		if (m_playerController0->GetPlayerControllerID() == -1)
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-1 Mouse & Keyboard", AABB2(Vec2(600.f, 200.f), Vec2(1000.f, 600.f)), 80.f);
// 		}
// 		else
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-1 Controller", AABB2(Vec2(600.f, 200.f), Vec2(1000.f, 400.f)), 80.f);
// 		}
// 	}
// 	if (!m_playerController0)
// 	{
// 		if (m_playerController1->GetPlayerControllerID() == -1)
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-2 Mouse & Keyboard", AABB2(Vec2(600.f, 200.f), Vec2(1000.f, 600.f)), 80.f);
// 		}
// 		else
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-2 Controller", AABB2(Vec2(600.f, 200.f), Vec2(1000.f, 400.f)), 80.f);
// 		}
// 	}
// 	// two players
// 	if (m_playerController0 && m_playerController1)
// 	{
// 		if (m_playerController0->GetPlayerControllerID() == -1)
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-1 Mouse & Keyboard", AABB2(Vec2(600.f, 500.f), Vec2(1000.f, 800.f)), 80.f);
// 		}
// 		else
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-1 Controller", AABB2(Vec2(600.f, 500.f), Vec2(1000.f, 800.f)), 80.f);
// 		}
// 
// 		if (m_playerController1->GetPlayerControllerID() == -1)
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-2 Mouse & Keyboard", AABB2(Vec2(600.f, 100.f), Vec2(1000.f, 300.f)), 80.f);
// 		}
// 		else
// 		{
// 			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player-2 Controller", AABB2(Vec2(600.f, 100.f), Vec2(1000.f, 400.f)), 80.f);
// 		}
// 	}
	
}

void Game::AddTextForLobby()
{
	float screenCenterX = 1600.0f / 2.0f;
	//float screenWidth = 1600.0f;
	float screenHeight = 800.0f;

	m_lobbyTextVerts.clear();

	//1
	if (m_playerController0 && !m_playerController1)
	{
		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player 1",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.7f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.8f)), 50.0f);

		if (m_playerController0->GetPlayerControllerID() == -1)
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Mouse and Keyboard",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.6f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.7f)), 40.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press SPACE to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.45f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.5f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press ESCAPE to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.38f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.43f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press START to join player",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.31f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.36f)), 20.0f);
		}
		else
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Controller",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.6f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.7f)), 40.0f);
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press START to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.45f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.5f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press BACK to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.38f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.43f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press SPACE to join player",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.31f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.36f)), 20.0f);
		}

		
	}
	else if (!m_playerController0 && m_playerController1)
	{
		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player 2",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.7f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.8f)), 50.0f);

		if (m_playerController1->GetPlayerControllerID() == -1)
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Mouse and Keyboard",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.6f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.7f)), 40.0f);
		}
		else
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Controller",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.6f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.7f)), 40.0f);
		}

		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press START to start game",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.45f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.5f)), 20.0f);

		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press BACK to leave game",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.38f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.43f)), 20.0f);
	}
	//2
	else if (m_playerController0 && m_playerController1)
	{
		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player 1",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.75f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.85f)), 50.0f);

		if (m_playerController0->GetPlayerControllerID() == -1)
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Mouse and Keyboard",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.65f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.75f)), 40.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press SPACE to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.58f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.63f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press ESCAPE to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.51f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.56f)), 20.0f);
		}
		else
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Controller",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.65f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.75f)), 40.0f);
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press START to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.58f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.63f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press BACK to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.51f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.56f)), 20.0f);
		}

		m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Player 2",
			AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.35f),
				Vec2(screenCenterX + 400.0f, screenHeight * 0.45f)), 50.0f);

		if (m_playerController1->GetPlayerControllerID() == -1)
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Mouse and Keyboard",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.25f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.35f)), 40.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press SPACE to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.18f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.23f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press ESC to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.11f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.16f)), 20.0f);
		}
		else
		{
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Controller",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.25f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.35f)), 40.0f);
			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press START to start game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.18f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.23f)), 20.0f);

			m_font->AddVertsForTextInBox2D(m_lobbyTextVerts, "Press BACK to leave game",
				AABB2(Vec2(screenCenterX - 400.0f, screenHeight * 0.11f),
					Vec2(screenCenterX + 400.0f, screenHeight * 0.16f)), 20.0f);
		}


	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);

	char timeBuffer[256];
	snprintf(timeBuffer, sizeof(timeBuffer),
		"FPS: %.2f",
		1.f / m_gameClock->GetDeltaSeconds());
	DebugAddScreenText(std::string(timeBuffer), AABB2(Vec2(1100.f, 770.f), Vec2(1590.f, 790.f)), 10.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);

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
		m_nextState = GameState::ATTRACT;
	}

	XboxController const& controller0 = g_theInput->GetController(0);
	XboxController const& controller1 = g_theInput->GetController(1);

	if (controller0.IsConnected() && controller0.WasButtonJustPressed(XboxButtonID::BACK))
	{
		m_nextState = GameState::ATTRACT;
	}

	if (controller1.IsConnected() && controller0.WasButtonJustPressed(XboxButtonID::BACK))
	{
		m_nextState = GameState::ATTRACT;
	}
}

void Game::UpdateDeveloperCheats()
{
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

	std::vector<Vertex_PCU> verts;
	verts.reserve(100);
	AddVertsForAABB2D(verts, AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)),Rgba8::WHITE);

	AABB2 textBox = AABB2(Vec2(250.f, 50.f), Vec2(1450.f, 120.f));

	AddVertsForAABB2D(verts, textBox, Rgba8::TRANSP_BLACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	Texture* bkgTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Attract.png");
	g_theRenderer->BindTexture(bkgTex);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve(100);
	m_menuBtnFont->AddVertsForTextInBox2D(textVerts, "Press Space to Start", textBox,30.f);
	
	g_theRenderer->BindTexture(&m_menuBtnFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderLobbyMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	DebugDrawBox(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight(), Rgba8::HILDA);

	g_theRenderer->BindTexture(&m_font->GetTexture());
	g_theRenderer->DrawVertexArray(m_lobbyTextVerts);

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderPlayingMode() const
{
	m_curMap->Render();

	if (m_playerController0)
	{
		DebugRenderWorld(m_playerController0->m_playerCam);
	}
	if (m_playerController1)
	{
		DebugRenderWorld(m_playerController1->m_playerCam);
	}
	
	//--------------------------------------------------------
	//RenderPlayingModeHUD();

	g_theRenderer->BeginCamera(m_screenCamera);
	DebugRenderScreen(m_screenCamera);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderPlayingModeHUD() const
{
// 	g_theRenderer->BindTexture(nullptr);
// 	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
// 	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));

	//m_playerController0->RenderPlayerHUD();
}

void Game::RenderDebugMode()const
{

}

void Game::ParseGameConfig()
{
	XmlDocument gameConfigDefsDoc;
	char const* filePath = "Data/GameConfig.xml";
	XmlResult result = gameConfigDefsDoc.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required game config file \"%s\"", filePath));

	XmlElement* rootElement = gameConfigDefsDoc.RootElement();
	if (!rootElement)
	{
		GUARANTEE_OR_DIE(rootElement, "Actor definitions file has no root element");
		return;
	}

	XmlElement* gameDefElement = rootElement;

	m_musicVolume= ParseXmlAttribute(gameDefElement, "musicVolume", m_musicVolume);
	m_mainMenuMusicPath = ParseXmlAttribute(gameDefElement, "mainMenuMusic", "");
	m_gameMusicPath = ParseXmlAttribute(gameDefElement, "gameMusic", "");
	m_buttonClickSoundPath = ParseXmlAttribute(gameDefElement, "buttonClickSound", "");

}

void Game::UpdateMenuMode()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

	SoundID clickSoundID = g_theAudio->CreateOrGetSound(m_buttonClickSoundPath, true);
	//input
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theAudio->StartSound(clickSoundID);
		g_theApp->m_isQuitting = true;
	}
	//Controller Input
	XboxController const& controller0 = g_theInput->GetController(0);
	if (controller0.IsConnected())
	{
		if (controller0.WasButtonJustPressed(XboxButtonID::BACK))
		{
			g_theAudio->StartSound(clickSoundID);
			g_theApp->m_isQuitting = true;
		}
	}

	m_btnMenuNormalMode.Update(deltaSeconds);
	m_btnMenuGoldMode.Update(deltaSeconds);
	m_btnMenuTutorialMode.Update(deltaSeconds);
}

void Game::RenderMenuMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> verts;
	verts.reserve(100);
	AddVertsForAABB2D(verts, AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)), Rgba8::WHITE);

	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	Texture* bkgTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenu.png");
	g_theRenderer->BindTexture(bkgTex);
	g_theRenderer->DrawVertexArray(verts);

	m_btnMenuNormalMode.Render();
	m_btnMenuGoldMode.Render();
	m_btnMenuTutorialMode.Render();

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::InitializeMenuButtons()
{
	Texture* menuBtnTexture1 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MenuBtn1.png");
	Texture* menuBtnTexture2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MenuBtn2.png");
	Texture* menuBtnTexture3 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MenuBtn3.png");

	float btnWidth = 307.0f;
	float btnHeight = 74.0f;

	float leftMargin = 100.0f; 
	float startY = 500.0f;     
	float buttonSpacing = 100.0f; 

	AABB2 bkgExtent = AABB2(-btnWidth / 2.0f, -btnHeight / 2.0f, btnWidth / 2.0f, btnHeight / 2.0f);

	AABB2 textExtent = AABB2(-btnWidth / 2.0f + 10.0f, -btnHeight / 2.0f + 10.0f,
		btnWidth / 2.0f - 10.0f, btnHeight / 2.0f - 10.0f);

	BitmapFont* gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/DariniaFont");

	float textHeight = 30.0f; 

	Vec2 btnNormalPos = Vec2(leftMargin + btnWidth / 2.0f, startY);
	m_btnMenuNormalMode = Button(btnNormalPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Normal", textHeight, gameFont, "StartNormalMode");

	Vec2 btnGoldPos = Vec2(leftMargin + btnWidth / 2.0f, startY - buttonSpacing);
	m_btnMenuGoldMode = Button(btnGoldPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Gold", textHeight, gameFont, "StartGoldMode");

	Vec2 btnTutorialPos = Vec2(leftMargin + btnWidth / 2.0f, startY - 2 * buttonSpacing);
	m_btnMenuTutorialMode = Button(btnTutorialPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Tutorial", textHeight, gameFont, "StartTutorial");

	//call back
	g_theEventSystem->SubscribeEventCallbackFuction("StartNormalMode", BtnEvent_NormalMode, true);
	g_theEventSystem->SubscribeEventCallbackFuction("StartGoldMode", BtnEvent_GoldlMode, true);
	g_theEventSystem->SubscribeEventCallbackFuction("StartTutorial", BtnEvent_TutorialMode, true);
}














