#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"

SoundPlaybackID bgm;
SpriteSheet* g_terrianSpriteSheet=nullptr;
SpriteSheet* g_explosionAnimSpriteSheet = nullptr;
SpriteSheet* g_decoSpriteSheet = nullptr;
SpriteAnimDefinition* g_explosionAnimDef_Slow = nullptr;
SpriteAnimDefinition* g_explosionAnimDef_Fast = nullptr;


bool SelectFood(EventArgs& args)
{
	DebuggerPrintf("SelectFood() was called\n");
	std::string myFood = args.GetValue("MyFood", "");
	DebuggerPrintf(myFood.c_str());
	return false;
}

Game::Game()
{
	g_theEventSystem->SubscribeEventCallbackFuction("SelectFoodEvent", SelectFood);

	m_clickSound = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	m_attractModeBgm= g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	m_gameplayModeBgm= g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	m_pause= g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_unpause = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");

	m_success = g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");
	m_fall = g_theAudio->CreateOrGetSound("Data/Audio/GameOver.mp3");
	m_playerShoot = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	m_enemyShoot = g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	m_enemyDead = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	m_playerDead = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.mp3");
	m_enemyHit= g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	m_playerHit= g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	m_bulletBounce= g_theAudio->CreateOrGetSound("Data/Audio/BulletBounce.wav");

	Texture* tileMapTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	Texture* playerTankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	Texture* playerTankTopTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	Texture* gameOverTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");
	Texture* explosionAnimTexture= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	Texture* decoSpriteSheet = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Extras_4x4.png");
	UNUSED(playerTankBaseTexture);
	UNUSED(playerTankTopTexture);
	UNUSED(gameOverTexture);
	UNUSED(explosionAnimTexture);

	g_terrianSpriteSheet =new SpriteSheet(*tileMapTexture, IntVec2(8,8));
	g_decoSpriteSheet = new SpriteSheet(*decoSpriteSheet, IntVec2(4, 4));
	TileDefinition::InitializeTileDefinitionFromFile();
	MapDefinition::InitializeMapDefinitionFromFile();

	g_explosionAnimSpriteSheet = new SpriteSheet(*explosionAnimTexture, IntVec2(5, 5));
	g_explosionAnimDef_Slow = new SpriteAnimDefinition(*g_explosionAnimSpriteSheet, 0, 24,
		30, SpriteAnimPlaybackType::ONCE);
	g_explosionAnimDef_Fast = new SpriteAnimDefinition(*g_explosionAnimSpriteSheet, 0, 23,
		50, SpriteAnimPlaybackType::ONCE);

	m_maps.reserve(10);
	Map* map1 = new Map(MapDefinition::s_mapDefinitions[0]);
	Map* map2 = new Map(MapDefinition::s_mapDefinitions[1]);
	Map* map3 = new Map(MapDefinition::s_mapDefinitions[2]);
	Map* map4 = new Map(MapDefinition::s_mapDefinitions[3]);
	Map* map5 = new Map(MapDefinition::s_mapDefinitions[4]);
	Map* map6 = new Map(MapDefinition::s_mapDefinitions[5]);
	m_maps.push_back(map1); 
	m_maps.push_back(map2);
	m_maps.push_back(map3);
	m_maps.push_back(map4);
	m_maps.push_back(map5);
	m_maps.push_back(map6);
	m_currentMap = m_maps[m_curMapIndex];
	
	Vec2 playerStartPoint = Vec2((float)m_currentMap->m_startCoords.x + 0.5f, (float)m_currentMap->m_startCoords.y + 0.5f);
	m_player = dynamic_cast<Player*>(m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_PLAYER,FACTION_GOOD, playerStartPoint, 45.f));
	m_currentMap->m_player = m_player;
	m_currentMap->m_player->m_turretOrientationDegrees = 45.f;

	float screenCameraSizeX = g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f);
	float screenCameraSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", 800.f);
	//float tileSizeX = g_gameConfigBlackboard.GetValue("tileSizeX", 1.f);
	//float tileSizeY = g_gameConfigBlackboard.GetValue("tileSizeY", 1.f);
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(screenCameraSizeX, screenCameraSizeY));
	
	SetDebugCameraOrtho(g_gameConfigBlackboard.GetValue("tileSizeX", 1.f), g_gameConfigBlackboard.GetValue("tileSizeY", 1.f));

	bgm = g_theAudio->StartSound(m_attractModeBgm, true);


	m_isStartMap = true;
}

Game::~Game()
{
	g_theAudio->StopSound(bgm);
	delete g_terrianSpriteSheet;

	delete g_explosionAnimSpriteSheet;
	delete g_explosionAnimDef_Slow;
	delete g_decoSpriteSheet;
}


void Game::Update(float deltaSeconds)
{

	UpdateInput(deltaSeconds);
	
	if (m_isAttractMode)
	{
		UpdateAttractMode(deltaSeconds);
	}
	else
	{
		UpdateGameplayMode(deltaSeconds);
	}
}

void Game::Renderer() const
{
	if (m_isAttractMode)
	{
		RenderAttractMode();
		return;
	}
	//---------------------------------------------------------
	Camera const*curCamera;
	if (m_isUseDebugCam)
	{
		curCamera = &m_debugCamera;
	}
	else
	{
		curCamera = &m_worldCamera;
	}
	g_theRenderer->BeginCamera(*curCamera);

	m_currentMap->Render();

	if (m_isDebugMode)
	{
		RenderDebugMode();
	}
	
	g_theRenderer->EndCamera(*curCamera);
	//---------------------------------------------------------
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(),m_screenCamera.GetOrthoTopRight()),g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::UpdateInput(float& deltaTime)
{
	//-----------------------------------
	if (g_theInput->WasKeyJustPressed('1'))
	{
		g_theDevConsole->Execute("SelectFoodEvent MyFood=apple\nSelectFoodEvent MyFood=banana");
	}
	//--------------------------------------
	if (m_isAttractMode)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			g_theAudio->StartSound(m_clickSound);
			g_theApp->m_isQuitting = true;
		}

		if ( g_theInput->WasKeyJustPressed('P')|| g_theInput->GetController(0).GetButton(XboxButtonID::START).m_isPressed)
		{
			m_isAttractMode = false;
			m_isPause = false;

			g_theAudio->StartSound(m_clickSound);
			g_theAudio->StopSound(bgm);
			bgm = g_theAudio->StartSound(m_gameplayModeBgm, true, 0.8f);

			m_startTransTime = (float)GetCurrentTimeSeconds();
		}
	}
	else
	{
		if (g_theInput->WasKeyJustPressed('P'))
		{
			if (m_isSuccess)
			{
				ResetAndBackToAttractMode();
			}
			else
			{
				m_isPause = !m_isPause;

				if (!m_isPause)
				{
					g_theAudio->StartSound(m_pause);
					g_theAudio->SetSoundPlaybackSpeed(bgm, 1.f);
				}
				else
				{
					g_theAudio->StartSound(m_unpause);
					g_theAudio->SetSoundPlaybackSpeed(bgm, 0.f);
				}
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			if (m_isSuccess)
			{
				ResetAndBackToAttractMode();
			}
			else
			{
				if (m_isPause)
				{
					ResetAndBackToAttractMode();
				}
				else
				{
					m_isPause = true;
					g_theAudio->StartSound(m_pause);
					g_theAudio->SetSoundPlaybackSpeed(bgm, 0.f);
				}
			}
		}
		if (g_theInput->WasKeyJustPressed('N') || g_theInput->GetController(0).GetButton(XboxButtonID::START).m_isPressed)
		{
			if (m_isGameOver)
			{
				m_isPause = false;
				m_isGameOver = false;
				m_player->m_isDead = false;
				m_player->ResetHealth();
			}
			if (m_isSuccess)
			{
				ResetAndBackToAttractMode();
			}
		}
		UpdateDeveloperCheats(deltaTime);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->ToggleMode(OPEN_FULL);
		}
		else
		{
			g_theDevConsole->ToggleMode(HIDDEN);
		}
	}
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
}

void Game::UpdateGameplayMode(float deltaTime)
{
	m_currentMap->Update(deltaTime);
	CheckIsPlayerAlive();
	if (m_isChangeMap)
	{
		if (GetCurrentTimeSeconds() - m_startTransTime > m_transitionTime)
		{
			m_isChangeMap = false;
			m_player->m_curSpinSpeed = 0.f;
			m_alphaBlackFront = 255.f;
			m_player->m_scale = 0.f;
			ChangeMap(m_maps[m_curMapIndex], 45.f);
		}
		m_alphaBlackFront += (255.f * deltaTime )/ m_transitionTime;
	}
	if (m_isStartMap)
	{
		if (GetCurrentTimeSeconds() - m_startTransTime > m_transitionTime)
		{
			m_isStartMap = false;
			m_player->m_curSpinSpeed = 0.f;
			m_player->m_scale = 1.f;
			m_alphaBlackFront = 0.f;
		}
		m_alphaBlackFront -= (255.f * deltaTime) / m_transitionTime;
	}
	//----------------------------------------------------------------------------
	Vec2 cameraTransDist(0.f, 0.f);
	if (!m_isGapShake)
	{
		if (m_isShakeScreen)
		{
			m_startGapShakeTime = GetCurrentTimeSeconds();
			m_isGapShake = true;
			double shakeTime = GetCurrentTimeSeconds() - m_startShakeTime;
			if (shakeTime > 1.0)
			{
				m_isShakeScreen = false;
			}
			else
			{
				float range = RangeMap((float)shakeTime, 0.f, 1.f, 0.1f, 0.f);
				float x = m_rng.RollRandomFloatInRange(-range, range);
				float y = m_rng.RollRandomFloatInRange(-range, range);
				cameraTransDist = Vec2{ x,y };
			}
		}
		m_worldCamera.Translate2DFromPosition(cameraTransDist, m_worldCamera.GetOrthoBottomLeft(), m_worldCamera.GetOrthoTopRight());
	}
	if (GetCurrentTimeSeconds() - m_startGapShakeTime > 0.01)
	{
		m_isGapShake = false;
	}
	
}

void Game::UpdateDeveloperCheats(float& deltaTime)
{
	AdjustForPauseAndTimeDitortion(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_isUseDebugCam = !m_isUseDebugCam;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_isNoclip = !m_isNoclip;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		ChangeToNextMap();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_isHeatMapCount++;
		if (m_isHeatMapCount > 4)
		{
			m_isHeatMapCount = 0;
		}
	}
}

void Game::AdjustForPauseAndTimeDitortion(float& deltaSeconds)
{	
	m_isSlow = g_theInput->IsKeyDown('T');
	m_isSpeedUp= g_theInput->IsKeyDown('Y');

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
	if (m_isSpeedUp)
	{
		deltaSeconds *= 4.f;
	}
	if (m_pauseAfterUpdate)
	{
		m_isPause = true;
		m_pauseAfterUpdate = false;
	}
}

void Game::RenderAttractMode() const
{
	float screenCameraSizeX = g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f);
	float screenCameraSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", 800.f);

	g_theRenderer->BeginCamera(m_screenCamera);
	//----------------------------------------------------------------------------------------------------------
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png"));
	AABB2 texturedAABB2_menu(0.f, 0.f, screenCameraSizeX, screenCameraSizeY);
	std::vector<Vertex_PCU> menuVerts;
	AddVertsForAABB2D(menuVerts, texturedAABB2_menu, Rgba8(255, 255, 255, 255), Vec2(0.f, 0.f), Vec2(1.f, 1.f));
	g_theRenderer->DrawVertexArray(menuVerts);
	//----------------------------------------------------------------------------------------------------------
	g_theRenderer->BindTexture(nullptr);
	float delta_radius = 50*SinDegrees(50 * (float) GetCurrentTimeSeconds());
	float delta_thickness = 8 * CosDegrees(200 *(float) GetCurrentTimeSeconds());
	DebugDrawRing(15.f+ delta_thickness, 100.f+ delta_radius, Rgba8(0, 180, 100), Vec2(screenCameraSizeX * 0.5f, screenCameraSizeY * 0.5f));
	//----------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> textVerts;
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.f, 200.f), 30.f, "Hello, world");
	g_testFont->AddVertsForText2D(textVerts, Vec2(250.f, 400.f), 15.f, "It's nice to have options!", Rgba8::RED, 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
	//----------------------------------------------------------------------------------------------------------
	float aliX = abs(CosDegrees((float)GetCurrentTimeSeconds() * 50.f));
	float aliY = abs(SinDegrees((float)GetCurrentTimeSeconds() * 50.f));
	float cellHeight = 5.f + 30.f * abs(SinDegrees((float)GetCurrentTimeSeconds() * 50.f));
	float tempY = 80.f + 60.f * abs(SinDegrees((float)GetCurrentTimeSeconds() * 50.f));
	float tempX = 400.f + 600.f * abs(CosDegrees((float)GetCurrentTimeSeconds() * 50.f));
	std::vector<Vertex_PCU> boxTextVerts;
	AABB2 testTextBox = AABB2(Vec2(300.f, 300.f), Vec2(300.f+tempX, 300.f+tempY));
	DebugDrawBox(Vec2(300.f, 300.f), Vec2(300.f + tempX, 300.f + tempY), Rgba8::HILDA);
	g_testFont->AddVertsForTextInBox2D(boxTextVerts, "hello world\nMy Name is Hilda\nto be or not to be, that's a question.",
		testTextBox, cellHeight, Rgba8::BLUE,1.0f,Vec2(aliX, aliY),SHRINK_TO_FIT);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(boxTextVerts);
	//----------------------------------------------------------------------------------------------------------
	SpriteDefinition curExplosionAniSpriteDef=g_explosionAnimDef_Slow->GetSpriteDefAtTime((float)GetCurrentTimeSeconds());
	g_theRenderer->BindTexture(&curExplosionAniSpriteDef.GetTexture());
	AABB2 texturedAABB2_ani(600.f, 100.f, 700.f,200.f);
	std::vector<Vertex_PCU> aniVerts;
	AddVertsForAABB2D(aniVerts, texturedAABB2_ani, Rgba8(255, 255, 255, 255), curExplosionAniSpriteDef.GetUVs().m_mins, curExplosionAniSpriteDef.GetUVs().m_maxs);
	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	g_theRenderer->DrawVertexArray(aniVerts);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	//----------------------------------------------------------------------------------------------------------
	g_theDevConsole->Render(texturedAABB2_menu,g_theRenderer);
	//----------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderUI() const
{
	float screenCameraSizeX = g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f);
	float screenCameraSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", 800.f);
	if (m_isHeatMapCount>0)
	{
		std::string debugString;
		switch (m_isHeatMapCount)
		{
		case 1:
			debugString = "Heat Map Debug: Distance Map from start (F6 for next mode)";
			break;
		case 2:
			debugString = "Heat Map Debug: Solid Map for amphibians (F6 for next mode)";
			break;
		case 3:
			debugString = "Heat Map Debug: Solid Map for land-based (F6 for next mode)";
			break;
		case 4:
			debugString = "Heat Map Debug: Distance Map to selected Entity's goal (F6 for next mode)";
			break;
		}
		std::vector<Vertex_PCU> textVerts;
		g_testFont->AddVertsForText2D(textVerts, Vec2(22.f, 768.f), 15.f, debugString, Rgba8(0,0,0,200), 0.8f);
		g_testFont->AddVertsForText2D(textVerts, Vec2(20.f, 770.f), 15.f, debugString,Rgba8::GREEN,0.8f);
		g_theRenderer->BindTexture(&g_testFont->GetTexture());
		g_theRenderer->DrawVertexArray(textVerts);
	}
	if (m_isPause)
	{
		g_theRenderer->BindTexture(nullptr);
		DebugDrawBox(Vec2(0.f, 0.f), Vec2(screenCameraSizeX, screenCameraSizeY),Rgba8(0,0,0,80));
		if (m_isGameOver)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png"));
			AABB2 texturedAABB2_sheet(0.f, 0.f, screenCameraSizeX, screenCameraSizeY);
			std::vector<Vertex_PCU> testVerts;
			AddVertsForAABB2D(testVerts, texturedAABB2_sheet, Rgba8(255, 255, 255, 255), Vec2(0.f,0.f), Vec2(1.f, 1.f));
			g_theRenderer->DrawVertexArray(testVerts);
		}

		if (m_isSuccess)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg"));
			AABB2 texturedAABB2_sheet(0.f, 0.f, screenCameraSizeX, screenCameraSizeY);
			std::vector<Vertex_PCU> testVerts;
			AddVertsForAABB2D(testVerts, texturedAABB2_sheet, Rgba8(255, 255, 255, 255), Vec2(0.f, 0.f), Vec2(1.f, 1.f));
			g_theRenderer->DrawVertexArray(testVerts);
		}
	}
	if (m_isChangeMap||m_isStartMap)
	{
		g_theRenderer->BindTexture(nullptr);
		AABB2 BlackFrontAABB2(0.f, 0.f, screenCameraSizeX, screenCameraSizeY);
		std::vector<Vertex_PCU> blackFrontVerts;
		AddVertsForAABB2D(blackFrontVerts, BlackFrontAABB2, Rgba8(0, 0,0, (unsigned char)m_alphaBlackFront), Vec2(0.f, 0.f), Vec2(1.f, 1.f));
		g_theRenderer->DrawVertexArray(blackFrontVerts);
	}

}

void Game::RenderDebugMode()const
{
	m_currentMap->RenderDebugDraw();
}

void Game::ChangeMap(Map* nextMap,float nextPlayerOrientation)
{
	float tileSizeX = g_gameConfigBlackboard.GetValue("tileSizeX", 1.f);
	float tileSizeY = g_gameConfigBlackboard.GetValue("tileSizeY", 1.f);
	m_currentMap->RemoveEntityFromMap(*m_player);
	m_currentMap = nextMap;
	Vec2 playerStartPoint = Vec2((float)m_currentMap->m_startCoords.x + 0.5f, (float)m_currentMap->m_startCoords.y + 0.5f);
	m_currentMap->AddEntityToMap(*m_player, playerStartPoint,nextPlayerOrientation);
	m_currentMap->m_player = m_player;
	m_currentMap->m_player->m_turretOrientationDegrees = nextPlayerOrientation;
	SetDebugCameraOrtho(tileSizeX, tileSizeY);
	m_isStartMap = true;
	m_startTransTime = (float)GetCurrentTimeSeconds();
}

void Game::SetDebugCameraOrtho(float tileSizeX, float tileSizeY)
{
	float ratio = (float)m_currentMap->m_dimensions.y / (float)m_currentMap->m_dimensions.x;
	if (ratio < 0.5)
	{
		m_debugCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(m_currentMap->m_dimensions.x * tileSizeX, 0.5f * m_currentMap->m_dimensions.x * tileSizeX));
	}
	else
	{
		m_debugCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(2.0f * m_currentMap->m_dimensions.y * tileSizeY, m_currentMap->m_dimensions.y * tileSizeY));
	}
}

void Game::CheckIsPlayerAlive()
{
	if (m_player != nullptr && m_player->m_isDead)
	{
		if (!m_isPlayerDeadCountDownStart)
		{
			m_playerDeadCountDownStart = GetCurrentTimeSeconds();
			m_isPlayerDeadCountDownStart = true;
		}
		if (GetCurrentTimeSeconds() - m_playerDeadCountDownStart > 3.0)
		{
			m_isPlayerDeadCountDownStart = false;
			GameOverState();
		}
	}
}

void Game::GameOverState()
{
	m_isPause = true;
	m_isGameOver = true;
	g_theAudio->StartSound(m_fall);
}

void Game::ResetAndBackToAttractMode()
{
	m_isGameOver = false;
	m_isSuccess = false;
	m_isPause = false;

	m_curMapIndex = 0;
	ChangeMap(m_maps[0], 45.f);
	m_player->m_scale = 0.f;
	m_player->m_isDead = false;
	m_player->ResetHealth();
	m_isAttractMode = true;
	g_theAudio->StartSound(m_clickSound);
	g_theAudio->StopSound(bgm);
	bgm = g_theAudio->StartSound(m_attractModeBgm, true, 0.8f);
}

bool Game::IsNoClip()
{
	return m_isNoclip;
}

void Game::ChangeToNextMap()
{
	m_curMapIndex++;
	if (m_curMapIndex >= (int)m_maps.size())
	{
		m_isPause = true;
		m_isSuccess = true;
		g_theAudio->StartSound(m_success);
	}
	else
	{
		m_isChangeMap = true;
		m_startTransTime = (float)GetCurrentTimeSeconds();
	}
}

void Game::ShakeScreen()
{
	m_isShakeScreen = true;
	m_startShakeTime = GetCurrentTimeSeconds();
}












