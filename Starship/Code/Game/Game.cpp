#include <Game/Game.hpp>
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include"Engine/Math/RandomNumberGenerator.hpp"
#include "Game/BkgStar.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
//extern App*			g_theApp;
//extern InputSystem* g_theInput;
//extern Renderer*	g_theRenderer;
//extern AudioSystem* g_theAudio;
BitmapFont* g_testFont = nullptr;

SoundPlaybackID bgm;

Game::Game()
{
	g_testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	clickSound = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	playerNormalShootSound= g_theAudio->CreateOrGetSound("Data/Audio/Laser_Shoot49.mp3");
	playerExpShootSound= g_theAudio->CreateOrGetSound("Data/Audio/EXPShoot2.mp3");
	playerHurtSound= g_theAudio->CreateOrGetSound("Data/Audio/Hit_Hurt22.mp3");
	playerDiedSound = g_theAudio->CreateOrGetSound("Data/Audio/Explosion13.mp3");
	playerChangeWeaponSound= g_theAudio->CreateOrGetSound("Data/Audio/ChangeWeapon.mp3");
	attrackmodeBGM = g_theAudio->CreateOrGetSound("Data/Audio/BGM_quiet.mp3");
	playmodeBGM= g_theAudio->CreateOrGetSound("Data/Audio/BGM_playmode2.mp3");
	playerSaberSound=g_theAudio->CreateOrGetSound("Data/Audio/LightSaberStart.mp3");

	asteroidHurtSound = g_theAudio->CreateOrGetSound("Data/Audio/AsteroidHurt.mp3");
	asteroidExpSound = g_theAudio->CreateOrGetSound("Data/Audio/AsteroidDie.mp3");
	expBulletShootSound = g_theAudio->CreateOrGetSound("Data/Audio/LightSaberStart.mp3");
	enemyHurtSound = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHurt.mp3");
	enmeyDiedSound = g_theAudio->CreateOrGetSound("Data/Audio/LightSaberStart.mp3");
	enemyShootSound = g_theAudio->CreateOrGetSound("Data/Audio/LightSaberStart.mp3");

	playerRespawn= g_theAudio->CreateOrGetSound("Data/Audio/playerRespawn.wav");
	finalDefeat= g_theAudio->CreateOrGetSound("Data/Audio/finalDefeat.wav");
	finalSuccess= g_theAudio->CreateOrGetSound("Data/Audio/finalSuccess.wav");
	newWave=g_theAudio->CreateOrGetSound("Data/Audio/newWave.mp3");
	bulletTimeStart= g_theAudio->CreateOrGetSound("Data/Audio/bulletTimeStart.wav");
	setBulletTimeMove= g_theAudio->CreateOrGetSound("Data/Audio/setBulletTimeMove.wav");
	dash = g_theAudio->CreateOrGetSound("Data/Audio/dash.wav");

	g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Moon.png");

	m_rng = new RandomNumberGenerator();
	//m_playerShip = new PlayerShip(this,WORLD_CENTER_X,WORLD_CENTER_Y);
	//m_rng = new RandomNumberGenerator();
	//GenerateAsteroids(WAVE1_ASTEROIDS);
	//GenerateBeetles(WAVE1_BEETLES);
	//GenerateWasps(WAVE1_WASP);
	//m_isAsteroidsClear = false;
	//m_isBeetlesClear = false;
	//m_isWaspsClear = false;
	m_startTime = GetCurrentTimeSeconds();
	m_isArractMode = true;
	m_enemyNumInEachWaves[0] = EnemyNumInWave{ 60,2,2,2,3 };
	m_enemyNumInEachWaves[1] = EnemyNumInWave{ 10,3,3,3,4 };
	m_enemyNumInEachWaves[2] = EnemyNumInWave{ 20,4,4,4,5 };
	m_enemyNumInEachWaves[3] = EnemyNumInWave{ 20,4,5,5,3 };
	m_enemyNumInEachWaves[4] = EnemyNumInWave{ 20,4,6,6,3 };

	m_worldCamera = new Camera();
	m_worldCamera->SetOrthoView(Vec2{ 0.f,0.f }, Vec2{ 200.f,100.f });
	m_oriWorldScreenBL = m_worldCamera->GetOrthoBottomLeft();
	m_oriWorldScreenTR = m_worldCamera->GetOrthoTopRight();
	m_screenCamera = new Camera();
	m_screenCamera->SetOrthoView(Vec2{ 0.f,0.f }, Vec2{ 1600.f,800.f });

	bgm= g_theAudio->StartSound(attrackmodeBGM,true);
}

Game::~Game()
{
	g_theAudio->StopSound(bgm);
	delete m_playerShip;
	m_playerShip = nullptr;

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;

		}
	}
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] != nullptr)
		{
			delete m_enmBullets[i];
			m_enmBullets[i] = nullptr;

		}
	}
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			delete m_expBullets[i];
			m_expBullets[i] = nullptr;
		}
	}

	DeleteCurAllEnemies();

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}


	for (int i = 0; i < MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			delete m_expDebris[i];
			m_expDebris[i] = nullptr;
		}
	}

	for (int i = 0; i < 1000; i++)
	{
		delete m_bkgStarsL1[i];
		m_bkgStarsL1[i] = nullptr;

		delete m_bkgStarsL2[i];
		m_bkgStarsL2[i] = nullptr;

		if (i < 300)
		{
			delete m_bkgStarsL4[i];
			m_bkgStarsL4[i] = nullptr;
		}
	}

	delete m_rng;
	m_rng = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;
	delete m_screenCamera;
	m_screenCamera = nullptr;
}

void Game::InitGameplay()
{
	//m_turret=new Turret()
	
	m_playerShip = new PlayerShip(this, WORLD_CENTER_X, WORLD_CENTER_Y);
	m_followedAim = m_playerShip->m_position;
	GenerateBkgStar();

	m_waveState = 0;
	GenerateEachWave(m_enemyNumInEachWaves[0]);

	//m_turret[0] = new Turret(this, 250.f, 50.f);

	m_isAsteroidsClear = false;
	m_isBeetlesClear = false;
	m_isWaspsClear = false;
	m_isShootWaspsClear = false;
	//audio system test
	
	g_theAudio->StartSound(clickSound);
	m_playerShip->m_startCanonCoolDownTime = GetCurrentTimeSeconds();

	g_theAudio->StopSound(bgm);
	bgm = g_theAudio->StartSound(playmodeBGM,true,0.8f);
}

void Game::ResetGameplayBackToAttractMode()
{
	g_theAudio->StopSound(bgm);
	bgm = g_theAudio->StartSound(attrackmodeBGM,true);
	m_isArractMode = true;
	m_playerShip->m_isUseUpLives = false;
	m_isWin = false;

	DeleteCurAllEnemies();
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;
			
		}
	}
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			delete m_expBullets[i];
			m_expBullets[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] != nullptr)
		{
			delete m_enmBullets[i];
			m_enmBullets[i] = nullptr;

		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
			
		}
	}
	for (int i = 0; i < MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			delete m_expDebris[i];
			m_expDebris[i] = nullptr;
		}
	}
	delete m_playerShip;
	m_playerShip = nullptr;


	for (int i = 0; i < 1000; i++)
	{
		delete m_bkgStarsL1[i];
		m_bkgStarsL1[i] = nullptr;

		delete m_bkgStarsL2[i];
		m_bkgStarsL2[i] = nullptr;
		if (i < 300)
		{
			delete m_bkgStarsL4[i];
			m_bkgStarsL4[i] = nullptr;
		}
		
	}

	delete m_turret[0];
	m_turret[0] = nullptr;
}

void Game::Update(float deltaSeconds)
{
	AdjustForPauseAndTimeDitortion(deltaSeconds);
	if (!m_isArractMode)
	{
		if (m_isBulletTime)
		{
			deltaSeconds *= m_deltaTimeRatio;
		}
		UpdateGameplayMode(deltaSeconds);
	}
	else
	{
		UpdateAttractMode(deltaSeconds);
	}

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

void Game::Renderer() const
{
	if (!m_isArractMode)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BeginCamera(*m_worldCamera);
		RenderBkg();
		RenderBullet();
		m_playerShip->Render();
		RenderExpBullet();
		RenderEnmBullet();
		RenderAsteroids();
		RenderBeetles();
		RenderWasps();
		RenderShootWasps();
		RenderDebris();
		RenderTurrets();
		//m_turret[0]->Render();
		
		if (isDebugMode)
		{
			RenderDebugMode();
		}
		g_theRenderer->BeginCamera(*m_screenCamera);
		RenderUI();
	}
	else
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BeginCamera(*m_screenCamera);
		RenderAttractMode();
	}
}
void Game::RenderBkg() const
{
	for (int i = 0; i < 1000; i++)
	{
		m_bkgStarsL1[i]->Render();
		m_bkgStarsL2[i]->Render();
		if (i < 300)
			m_bkgStarsL4[i]->Render();
	}

}
void Game::RenderBullet()const
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->Render();
		}
	}
}
void Game::RenderExpBullet() const
{
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			m_expBullets[i]->Render();
		}
	}
}
void Game::RenderEnmBullet() const
{
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] != nullptr)
		{
			m_enmBullets[i]->Render();
		}
	}
}
void Game::RenderAsteroids() const
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->Render();
		}
	}
}
void Game::RenderBeetles() const
{
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			m_beetles[i]->Render();
			m_beetles[i]->RenderUI();
		}
	}
}
void Game::RenderWasps() const
{
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			m_wasps[i]->Render();
			m_wasps[i]->RenderUI();
		}
	}
}
void Game::RenderShootWasps() const
{
	for (int i = 0; i < MAX_SHOOTWASPS; i++)
	{
		if (m_shootWasps[i] != nullptr)
		{
			m_shootWasps[i]->Render();
			m_shootWasps[i]->RenderUI();
		}
	}
}
void Game::RenderTurrets() const
{
	for (int i = 0; i < MAX_TURRET; i++)
	{
		if (m_turret[i] != nullptr)
		{
			m_turret[i]->Render();
			m_turret[i]->RenderUI();
		}
	}
}
void Game::RenderDebris() const
{
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			m_debris[i]->Render();
		}
	}
	for (int i = 0; i < MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			m_expDebris[i]->Render();
		}
	}
}
void Game::RenderDebugMode()const
{
	//grey line
	Rgba8 color_grey{ 50,50,50,255 };
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, m_bullets[i]->m_position, 0.2f, color_grey);
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, m_asteroids[i]->m_position, 0.2f, color_grey);
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, m_wasps[i]->m_position, 0.2f, color_grey);
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, m_beetles[i]->m_position, 0.2f, color_grey);
		}
	}

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, m_debris[i]->m_position, 0.2f, color_grey);
		}
	}
	//fwd red line in cos radius
	Rgba8 color_red{255,0,0,255 };
	Rgba8 color_green{ 0,255,0,255 };
	Rgba8 color_yellow { 255, 255, 0, 255 };
	Rgba8 color_magenta{ 255, 0, 255, 255 };
	Rgba8 color_cyan{ 0, 255, 255, 255 };
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			Vec2 end_bullet = m_bullets[i]->m_position + m_bullets[i]->GetForwardNormal() * m_bullets[i]->m_cosmeticRadius;
			Vec2 endL_bullet = m_bullets[i]->m_position + m_bullets[i]->GetForwardNormal().GetRotated90Degrees() * m_bullets[i]->m_cosmeticRadius;
			Vec2 endV_bullet = m_bullets[i]->m_position + m_bullets[i]->m_velocity;
			DebugDrawLine(m_bullets[i]->m_position, end_bullet, 0.2f, color_red);
			DebugDrawLine(m_bullets[i]->m_position, endL_bullet, 0.2f, color_green);
			DebugDrawRing(0.2f, m_bullets[i]->m_cosmeticRadius, color_magenta,m_bullets[i]->m_position);
			DebugDrawRing(0.2f, m_bullets[i]->m_physicsRadius, color_cyan, m_bullets[i]->m_position);
			DebugDrawLine(m_bullets[i]->m_position, endV_bullet, 0.2f, color_yellow);
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			Vec2 end_ast = m_asteroids[i]->m_position + m_asteroids[i]->GetForwardNormal() * m_asteroids[i]->m_cosmeticRadius;
			Vec2 endL_ast = m_asteroids[i]->m_position + m_asteroids[i]->GetForwardNormal().GetRotated90Degrees() * m_asteroids[i]->m_cosmeticRadius;
			Vec2 endV_ast = m_asteroids[i]->m_position + m_asteroids[i]->m_velocity;
			DebugDrawLine(m_asteroids[i]->m_position, end_ast, 0.2f, color_red);
			DebugDrawLine(m_asteroids[i]->m_position, endL_ast, 0.2f, color_green);
			DebugDrawRing(0.2f, m_asteroids[i]->m_cosmeticRadius, color_magenta,m_asteroids[i]->m_position);
			DebugDrawRing(0.2f, m_asteroids[i]->m_physicsRadius, color_cyan, m_asteroids[i]->m_position);
			DebugDrawLine(m_asteroids[i]->m_position, endV_ast, 0.2f, color_yellow);
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			Vec2 end_ast = m_wasps[i]->m_position + m_wasps[i]->GetForwardNormal() * m_wasps[i]->m_cosmeticRadius;
			Vec2 endL_ast = m_wasps[i]->m_position + m_wasps[i]->GetForwardNormal().GetRotated90Degrees() * m_wasps[i]->m_cosmeticRadius;
			Vec2 endV_ast = m_wasps[i]->m_position + m_wasps[i]->m_velocity;
			DebugDrawLine(m_wasps[i]->m_position, end_ast, 0.2f, color_red);
			DebugDrawLine(m_wasps[i]->m_position, endL_ast, 0.2f, color_green);
			DebugDrawRing(0.2f, m_wasps[i]->m_cosmeticRadius, color_magenta, m_wasps[i]->m_position);
			DebugDrawRing(0.2f, m_wasps[i]->m_physicsRadius, color_cyan, m_wasps[i]->m_position);
			DebugDrawLine(m_wasps[i]->m_position, endV_ast, 0.2f, color_yellow);
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			Vec2 end_ast = m_beetles[i]->m_position + m_beetles[i]->GetForwardNormal() * m_beetles[i]->m_cosmeticRadius;
			Vec2 endL_ast = m_beetles[i]->m_position + m_beetles[i]->GetForwardNormal().GetRotated90Degrees() * m_beetles[i]->m_cosmeticRadius;
			Vec2 endV_ast = m_beetles[i]->m_position + m_beetles[i]->m_velocity;
			DebugDrawLine(m_beetles[i]->m_position, end_ast, 0.2f, color_red);
			DebugDrawLine(m_beetles[i]->m_position, endL_ast, 0.2f, color_green);
			DebugDrawRing(0.2f, m_beetles[i]->m_cosmeticRadius, color_magenta, m_beetles[i]->m_position);
			DebugDrawRing(0.2f, m_beetles[i]->m_physicsRadius, color_cyan, m_beetles[i]->m_position);
			DebugDrawLine(m_beetles[i]->m_position, endV_ast, 0.2f, color_yellow);
		}
	}

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			Vec2 end_ast = m_debris[i]->m_position + m_debris[i]->GetForwardNormal() * m_debris[i]->m_cosmeticRadius;
			Vec2 endL_ast = m_debris[i]->m_position + m_debris[i]->GetForwardNormal().GetRotated90Degrees() * m_debris[i]->m_cosmeticRadius;
			Vec2 endV_ast = m_debris[i]->m_position + m_debris[i]->m_velocity;
			DebugDrawLine(m_debris[i]->m_position, end_ast, 0.2f, color_red);
			DebugDrawLine(m_debris[i]->m_position, endL_ast, 0.2f, color_green);
			DebugDrawRing(0.2f, m_debris[i]->m_cosmeticRadius, color_magenta, m_debris[i]->m_position);
			DebugDrawRing(0.2f, m_debris[i]->m_physicsRadius, color_cyan, m_debris[i]->m_position);
			DebugDrawLine(m_debris[i]->m_position, endV_ast, 0.2f, color_yellow);
		}
	}

	Vec2 end_ship = m_playerShip->m_position + m_playerShip->GetForwardNormal() * m_playerShip->m_cosmeticRadius;
	Vec2 endL_ship = m_playerShip->m_position + m_playerShip->GetForwardNormal().GetRotated90Degrees() * m_playerShip->m_cosmeticRadius;
	Vec2 endV_ship = m_playerShip->m_position + m_playerShip->m_velocity;
	DebugDrawLine(m_playerShip->m_position, end_ship, 0.2f, color_red);
	DebugDrawLine(m_playerShip->m_position, endL_ship, 0.2f, color_green);
	DebugDrawRing(0.2f, m_playerShip->m_cosmeticRadius, color_magenta,m_playerShip->m_position);
	DebugDrawRing(0.2f, m_playerShip->m_physicsRadius, color_cyan, m_playerShip->m_position);
	DebugDrawLine(m_playerShip->m_position, endV_ship, 0.2f, color_yellow);

}
void Game::RenderUI() const
{
	//extra ship icon
	Vec2 oriIconPos{ 400.f,95.f*7.9f };
	for (int i = 0; i <m_playerShip->m_lives - 1; i++)
	{
		Vertex_PCU playerShipIcon[NUM_SHIP_VERTS];
		PlayerShip::InitializedVerts(&playerShipIcon[0], Rgba8(26, 148, 154, 255));
		TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon, 7.5f, 90.f, oriIconPos + Vec2{ i * 40.f,0.f }); //
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon); //NUM_SHIP_VERTS
	}
	//small map
	DebugDrawBoxLine(Vec2(1350.f, 660.f), Vec2(1550.f, 760.f), 2.f, Rgba8(80, 80, 125, 255));

	float shipMapPosX = RangeMapClamped(m_playerShip->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
	float shipMapPosY = RangeMapClamped(m_playerShip->m_position.y, -100.f, 200.f, 660.f, 760.f);
	DebugDrawCircle(2.f, Vec2(shipMapPosX, shipMapPosY), Rgba8(0, 125, 125, 255));
	
	float camBoundBotLX = RangeMapClamped(m_followedAim.x-WORLD_CENTER_X, -200.f, 400.f, 1350.f, 1550.f);
	float camBoundBotLY = RangeMapClamped(m_followedAim.y- WORLD_CENTER_Y, -100.f, 200.f, 660.f, 760.f);
	float camBoundTopRX = RangeMapClamped(m_followedAim.x + WORLD_CENTER_X, -200.f, 400.f, 1350.f, 1550.f);
	float camBoundTopRY = RangeMapClamped(m_followedAim.y+ WORLD_CENTER_Y, -100.f, 200.f, 660.f, 760.f);
	DebugDrawBoxLine(Vec2(camBoundBotLX, camBoundBotLY), Vec2(camBoundTopRX, camBoundTopRY), 2.f, Rgba8(0, 125, 100, 255));

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i]!= nullptr)
		{
			float asteroidMapPosX = RangeMapClamped(m_asteroids[i]->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
			float asteroidMapPosY = RangeMapClamped(m_asteroids[i]->m_position.y, -100.f, 200.f, 660.f, 760.f);
			DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8(116, 116, 116, 255));
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			float asteroidMapPosX = RangeMapClamped(m_beetles[i]->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
			float asteroidMapPosY = RangeMapClamped(m_beetles[i]->m_position.y, -100.f, 200.f, 660.f, 760.f);
			DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8::GREEN);
		}
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			float asteroidMapPosX = RangeMapClamped(m_wasps[i]->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
			float asteroidMapPosY = RangeMapClamped(m_wasps[i]->m_position.y, -100.f, 200.f, 660.f, 760.f);
			DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8::YELLOW);
		}
	}
	for (int i = 0; i < MAX_TURRET; i++)
	{
		if (m_turret[i] != nullptr)
		{
			float asteroidMapPosX = RangeMapClamped(m_turret[i]->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
			float asteroidMapPosY = RangeMapClamped(m_turret[i]->m_position.y, -100.f, 200.f, 660.f, 760.f);
			DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8::MAGNETA);
		}
	}
	for (int i = 0; i < MAX_SHOOTWASPS; i++)
	{
		if (m_shootWasps[i] != nullptr)
		{
			float asteroidMapPosX = RangeMapClamped(m_shootWasps[i]->m_position.x, -200.f, 400.f, 1350.f, 1550.f);
			float asteroidMapPosY = RangeMapClamped(m_shootWasps[i]->m_position.y, -100.f, 200.f, 660.f, 760.f);
			DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8::BLUE);
		}
	}
		
	//hp bar
	DebugDrawBox(Vec2(55.f, 740.f), Vec2(350.f, 760.f), Rgba8(80, 80, 125, 255));
	DebugDrawBox(Vec2(58.f, 743.f), Vec2(347.f, 757.f), Rgba8(34, 34, 104, 255));
	float hpLen = RangeMapClamped(m_playerShip->m_health, 0.f,m_playerShip->m_maxHealth, 0.f, 283.f);
	DebugDrawBox(Vec2(61.f, 746.f), Vec2(61.f+hpLen, 754.f), Rgba8(26, 148, 154, 255));
	//mg bar
	DebugDrawBoxLine(Vec2(55.f, 50.f), Vec2(95.f, 125.f), 2.f, Rgba8(0, 125, 100, 255));
	DebugDrawBoxLine(Vec2(60.f, 45.f), Vec2(90.f, 50.f), 2.f, Rgba8(0, 125, 100, 255));
	DebugDrawBoxLine(Vec2(60.f, 125.f), Vec2(90.f, 130.f), 2.f, Rgba8(0, 125, 100, 255));
	float mpLen = RangeMapClamped(m_playerShip->m_magicPoint, 0.f, m_playerShip->m_maxMagicPoint, 0.f, 65.f);
	DebugDrawBox(Vec2(60.f, 55.f), Vec2(90.f, 55.f+mpLen), Rgba8(226, 146, 80, 180));
	//main(first) arm position
	if (m_playerShip->m_FirstWeaponState == 1)
	{
		if (m_playerShip->m_Canon->m_level==1)
		{
			Vertex_PCU firstWeaponIcon[NUM_CANONL1_VERTS];
			Canon::InitializedVertsL1(&firstWeaponIcon[0]);
			TransformVertexArrayXY3D(NUM_CANONL1_VERTS, firstWeaponIcon, 8.f, 45.f, Vec2(158.f, 83.f)); //
			g_theRenderer->DrawVertexArray(NUM_CANONL1_VERTS, firstWeaponIcon);
		}
		else if (m_playerShip->m_Canon->m_level == 2)
		{
			Vertex_PCU firstWeaponIcon[NUM_CANONL2_VERTS];
			Canon::InitializedVertsL2(&firstWeaponIcon[0]);
			TransformVertexArrayXY3D(NUM_CANONL2_VERTS, firstWeaponIcon, 8.f, 45.f, Vec2(155.f, 80.f)); //
			g_theRenderer->DrawVertexArray(NUM_CANONL2_VERTS, firstWeaponIcon);
		}
		else if (m_playerShip->m_Canon->m_level == 3)
		{
			Vertex_PCU firstWeaponIcon[NUM_CANONL3_VERTS];
			Canon::InitializedVertsL3(&firstWeaponIcon[0]);
			TransformVertexArrayXY3D(NUM_CANONL3_VERTS, firstWeaponIcon, 8.f, 45.f, Vec2(155.f, 80.f)); //
			g_theRenderer->DrawVertexArray(NUM_CANONL3_VERTS, firstWeaponIcon);
		}
		
	}
	else if (m_playerShip->m_FirstWeaponState == 2)
	{
		if (m_playerShip->m_laserExplosionCanon->m_level==1)
		{
			Vertex_PCU firstWeaponIcon[NUM_LASER_CANON_VERTS];
			LaserExplosionCanon::InitializedVertsL1(&firstWeaponIcon[0]);
			TransformVertexArrayXY3D(NUM_LASER_CANON_VERTS-6, &firstWeaponIcon[6], 8.f, 45.f, Vec2(158.f, 83.f)); //
			g_theRenderer->DrawVertexArray(NUM_LASER_CANON_VERTS-6, &firstWeaponIcon[6]);
		}
	}
	//second arm position
	DebugDrawBoxLine(Vec2(205.f, 50.f), Vec2(265.f, 110.f), 2.f, Rgba8(80, 80, 125, 255));
	DebugDrawBox(Vec2(230.f, 60.f), Vec2(240.f, 100.f), Rgba8(226, 146, 80, 200));
	DebugDrawBox(Vec2(215.f, 75.f), Vec2(255.f, 85.f), Rgba8(226, 146, 80, 200));
	

	//third arm position
	DebugDrawBoxLine(Vec2(285.f, 50.f), Vec2(345.f, 110.f), 2.f, Rgba8(80, 80, 125, 255));
	DebugDrawHighCircle(15.f, Vec2(315.f, 80.f), Rgba8::BLUE);
	DebugDrawRing(3.f,20.f, Rgba8(0,255,255,150), Vec2(315.f, 80.f));

	DebugDrawBoxLine(Vec2(125.f, 50.f), Vec2(185.f, 110.f), 2.f, Rgba8(80, 80, 125, 255));
	std::vector<Vertex_PCU> textVerts;
	g_testFont->AddVertsForText2D(textVerts, Vec2(115.f, 113.f), 12.f, "C/B Switch Light Saber Bullet Time", Rgba8(150, 150, 100, 255), 0.618f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(105.f, 30.f), 12.f, "   Space/A      J/Y       RT+X", Rgba8(180, 120, 100, 255), 0.618f);
	if (m_waveState + 1 <= 5)
	{
		g_testFont->AddVertsForText2D(textVerts, Vec2(700.f, 700.f), 30.f, "MISSION  " + std::to_string(m_waveState + 1), Rgba8::HILDA, 0.618f);
	}
	else
		g_testFont->AddVertsForText2D(textVerts, Vec2(700.f, 700.f), 30.f, " SUCCESS", Rgba8(0,150,200,255), 0.618f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
}
void Game::RenderAttractMode() const
{
	//Vertex_PCU playerShipIcon[NUM_SHIP_VERTS];
	//PlayerShip::InitializedVerts(&playerShipIcon[0], Rgba8(102, 153, 204, 255));
	//TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon, 10.f, 0.f, Vec2(40.f,50.f)); 
	//g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon); 
	//Vertex_PCU playerShipIcon2[NUM_SHIP_VERTS];
	//PlayerShip::InitializedVerts(&playerShipIcon2[0], Rgba8(102, 153, 204, 255));
	//TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon2, 10.f, 180.f, Vec2(160.f, 50.f));
	//g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon2);
	//unsigned char uc_a = static_cast<unsigned char>(m_curAttractTriangle_a);
	//Vertex_PCU triangleGreen[3];
	//triangleGreen[0] = Vertex_PCU(Vec3(85, 35, 0.f), Rgba8(0,200,130, uc_a), Vec2(0.f, 0.f));
	//triangleGreen[1] = Vertex_PCU(Vec3(85, 65, 0.f), Rgba8(0, 200, 130, uc_a), Vec2(0.f, 0.f));
	//triangleGreen[2] = Vertex_PCU(Vec3(115, 50, 0.f), Rgba8(0, 200, 130, uc_a), Vec2(0.f, 0.f));
	//g_theRenderer->DrawVertexArray(3, triangleGreen);

	Vertex_PCU playerShipIcon3[NUM_SHIP_VERTS];
	PlayerShip::InitializedVerts(&playerShipIcon3[0], m_at_shipColor);
	Vec2 orbitCenter = Vec2(800.f, 400.f);
	Vec2 shipOriPos = Vec2::MakeFromPolarDegrees(m_shipRevolutionDeg, ORBIT_LEN);
	TransformPosition2D(shipOriPos, Vec2(1.f, 0.f), Vec2(0.f, 0.1f), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon3, m_shipScale, m_shipRevolutionDeg+90.f, orbitCenter+shipOriPos);
	if ((orbitCenter + shipOriPos).y >= orbitCenter.y)
	{
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon3);
		//DebugDrawHighCircle(200.f, orbitCenter, Rgba8(243, 243, 197));
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Moon.png"));
		AABB2 texturedAABB2_menu(550.f, 150.f, 1050.f, 650.f);
		std::vector<Vertex_PCU> moonVerts;
		AddVertsForAABB2D(moonVerts, texturedAABB2_menu, Rgba8(255, 255, 255, 255), Vec2(0.f, 0.f), Vec2(1.f, 1.f));
		g_theRenderer->DrawVertexArray(moonVerts);
		g_theRenderer->BindTexture(nullptr);
	}
	else
	{
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Moon.png"));
		AABB2 texturedAABB2_menu(550.f, 150.f, 1050.f, 650.f);
		std::vector<Vertex_PCU> moonVerts;
		AddVertsForAABB2D(moonVerts, texturedAABB2_menu, Rgba8(255, 255, 255, 255), Vec2(0.f, 0.f), Vec2(1.f, 1.f));
		g_theRenderer->DrawVertexArray(moonVerts);
		g_theRenderer->BindTexture(nullptr);

		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon3);
	}
	std::vector<Vertex_PCU> textVerts;
	g_testFont->AddVertsForText2D(textVerts, Vec2(115.f, 80.f),80.f, "Starship Gold",Rgba8(150,150,100,100),0.618f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(112.f, 78.f),80.f, "Starship Gold", Rgba8::HILDA, 0.618f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(1100.f, 90.f), 30.f, "Press Space/A to Start", Rgba8(243, 243, 197), 0.618f);
	//g_testFont->AddVertsForText2D(textVerts, Vec2(250.f, 400.f), 15.f, "It's nice to have options!", Rgba8::RED, 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
}

void Game::UpdateBullet(float deltaTime)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->Update(deltaTime);
		}
	}
}
void Game::UpdateEnmBullet(float deltaTime)
{
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] != nullptr)
		{
			m_enmBullets[i]->Update(deltaTime);
		}
	}
}
void Game::UpdateExpBullet(float deltaTime)
{
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			m_expBullets[i]->Update(deltaTime);
		}
	}
}
void Game::UpdateAsteroids(float deltaTime)
{
	bool isClear = true;
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			isClear = false;
			m_asteroids[i]->Update(deltaTime);
		}
	}
	m_isAsteroidsClear = isClear;
}
void Game::UpdateBeetles(float deltaTime)
{
	bool isClear = true;
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			isClear = false;
			m_beetles[i]->Update(deltaTime);
		}
	}
	m_isBeetlesClear = isClear;
}
void Game::UpdateWasps(float deltaTime)
{
	bool isClear = true;
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			isClear = false;
			m_wasps[i]->Update(deltaTime);
		}
	}
	m_isWaspsClear = isClear;
}
void Game::UpdateShootWasps(float deltaTime)
{
	bool isClear = true;
	for (int i = 0; i < MAX_SHOOTWASPS; i++)
	{
		if (m_shootWasps[i] != nullptr)
		{
			isClear = false;
			m_shootWasps[i]->Update(deltaTime);
		}
	}
	m_isShootWaspsClear = isClear;
}
void Game::UpdateTurrets(float deltaTime)
{
	for (int i = 0; i < MAX_TURRET; i++)
	{
		if (m_turret[i] != nullptr)
		{
			//isClear = false;
			m_turret[i]->Update(deltaTime);
		}
	}
}
void Game::UpdateDebris(float deltaTime)
{
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			m_debris[i]->Update(deltaTime);
		}
	}
	for (int i = 0;i < MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			m_expDebris[i]->Update(deltaTime);
		}
	}
}
void Game::UpdateBkg(float deltaTime)
{
	for (int i = 0; i < 1000; i++)
	{
		m_bkgStarsL1[i]->Update(deltaTime);
		m_bkgStarsL2[i]->Update(deltaTime);
		if (i < 300)
			m_bkgStarsL4[i]->Update(deltaTime);
	}
}
void Game::UpdateAttractMode(float deltaTime)
{
	//m_playerShip->m_completeDead = false;
	m_currentTime += deltaTime;
	float temp = RangeMap(SinDegrees((float)m_currentTime * 200.f), -1.f, 1.f, 0.f, 1.f);
	m_curAttractTriangle_a = 255.f * temp;

	if (m_shipRevolutionDeg >= 360.f)
	{
		m_shipRevolutionDeg = 0.f;
	}

	m_shipRevolutionDeg += SHIP_REVOLUTION_SPEED * deltaTime;
	m_shipScale= RangeMap(-SinDegrees(m_shipRevolutionDeg), -1.f, 1.f, 8.f, 12.f);
	m_at_shipColor = Rgba8((unsigned char)RangeMap(-SinDegrees(m_shipRevolutionDeg), -1.f, 1.f, 157.f, 44.f),
		(unsigned char)RangeMap(-SinDegrees(m_shipRevolutionDeg), -1.f, 1.f, 166.f, 46.f),
		(unsigned char)RangeMap(-SinDegrees(m_shipRevolutionDeg), -1.f, 1.f, 249.f, 124.f), 255);

	if (g_theInput->IsKeyDown('N')|| g_theInput->IsKeyDown(0x20)||
		g_theInput->GetController(0).GetButton(XboxButtonID::A).m_isPressed||
		g_theInput->GetController(0).GetButton(XboxButtonID::START).m_isPressed)
	{
		m_isArractMode = false;
		//init
		InitGameplay();
	}

	if (g_theInput->WasKeyJustPressed(27))
	{
		g_theApp->m_isQuitting = true;
	}
}
void Game::UpdateGameplayMode(float deltaTime)
{
	if (g_theInput->WasKeyJustPressed('I'))
	{
		bool canNewAsteroid = false;
		//initiate asteroids
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] == nullptr)
			{
				GenerateEachAsteroids(i);
				canNewAsteroid = true;
				break;
			}
		}
		if (!canNewAsteroid)
		{
			//open erro window
			ERROR_RECOVERABLE("no more asteroids");
		}
	}
	UpdateCheatInput();
	m_playerShip->Update(deltaTime);
	UpdateBullet(deltaTime);
	UpdateExpBullet(deltaTime);
	UpdateEnmBullet(deltaTime);
	UpdateAsteroids(deltaTime);
	UpdateBeetles(deltaTime);
	UpdateWasps(deltaTime);
	UpdateShootWasps(deltaTime);
	UpdateDebris(deltaTime);
	UpdateBkg(deltaTime);
	UpdateTurrets(deltaTime);
	//m_turret[0]->Update(deltaTime);

	CheckBulletsVsEnemies();
	CheckPlayerShipVsEnemies();
	CheckExpBulletsVsEnemies();
	CheckExpDebrisVSEnemies();
	if ((m_playerShip->m_isSaber||m_playerShip->m_isControllerSaber) && m_playerShip->m_magicPoint > MIN_SABER_MP)
	{
		CheckLightSaberVsEnemies();
	}
	
	UpdateWorldCamera();
	if ( m_isBeetlesClear && m_isWaspsClear&&m_isShootWaspsClear)
	{
		if(!m_isWin)
			m_waveState++;
		if (m_waveState < 5)
		{
			GenerateEachWave(m_enemyNumInEachWaves[m_waveState]);
		}
		else if (m_waveState >= 5 && m_isWin == false)
		{
			//win
			m_isWin = true;
			g_theAudio->StartSound(finalSuccess);
			m_winTime = (float)GetCurrentTimeSeconds();
		}
	}
	
	if (g_theInput->WasKeyJustPressed(0x70))
	{
		isDebugMode = !isDebugMode;
	}

	if (m_playerShip->m_isCanonAutoCoolDown == true)
	{
		float curTime = (float)GetCurrentTimeSeconds();
		if (curTime - m_playerShip->m_startCanonCoolDownTime >= m_playerShip->m_canonCoolDownTime)
		{
			m_playerShip->m_isCanonAutoCoolDown = false;
		}
	}

	if (g_theInput->WasKeyJustPressed(27) || m_flagToReset == true)
	{
		m_flagToReset = false;
		ResetGameplayBackToAttractMode();
	}
	DeleteGarbageEntities();
}
void Game::UpdateWorldCamera()
{
	m_followedAim = m_playerShip->m_position;
	if (m_playerShip->m_position.x < WORLDSPACE_SIZE_BL_X +100.f)
	{
		m_followedAim.x = WORLDSPACE_SIZE_BL_X + 100.f;
	}
	else if (m_playerShip->m_position.x > WORLDSPACE_SIZE_TR_X-100.f)
	{
		m_followedAim.x = WORLDSPACE_SIZE_TR_X - 100.f;
	}
	if (m_playerShip->m_position.y < WORLDSPACE_SIZE_BL_Y+50.f)
	{
		m_followedAim.y = WORLDSPACE_SIZE_BL_Y + 50.f;
	}
	else if (m_playerShip->m_position.y > WORLDSPACE_SIZE_TR_Y-50.f)
	{
		m_followedAim.y = WORLDSPACE_SIZE_TR_Y - 50.f;
	}
	//m_worldCamera->Translate2DToFollowedObj(m_followedAim);
	Translate2DToFollowedObj(*m_worldCamera, m_followedAim);
	Vec2 cameraTransDist(0.f, 0.f);
	if (m_isShakeScreen)
	{
		double shakeTime = GetCurrentTimeSeconds() - m_startShakeTime;
		if (shakeTime > 1.0)
		{
			m_isShakeScreen = false;
		}
		else
		{
			float range = RangeMap((float)shakeTime, 0.f, 1.f, 1.5f, 0.f);
			float x = m_rng->RollRandomFloatInRange(-range, range);
			float y = m_rng->RollRandomFloatInRange(-range, range);
			cameraTransDist = Vec2{ x,y };
		}
	}
	m_worldCamera->Translate2DFromPosition(cameraTransDist, m_worldCamera->GetOrthoBottomLeft(), m_worldCamera->GetOrthoTopRight());

}
void Game::Translate2DToFollowedObj(Camera& worldCamera,const Vec2& aimPos)
{
	Vec2 blTrans = Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y)+aimPos;
	Vec2 trTrans = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y) + aimPos;
	worldCamera.SetOrthoView(blTrans, trTrans);
}
void Game::UpdateQuitGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
}
void Game::UpdateCheatInput()
{
	if (g_theInput->WasKeyJustPressed('1'))
	{
		WaveHandler(0);
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		WaveHandler(1);
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		WaveHandler(2);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		WaveHandler(3);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		WaveHandler(4);
	}
	if (g_theInput->WasKeyJustPressed('K'))
	{
		DeleteCurAllEnemies();
	}
}

void Game::DeleteGarbageEntities()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			if (m_bullets[i]->IsGarbage())
			{
				delete m_bullets[i];
				m_bullets[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			if (m_expBullets[i]->IsGarbage())
			{
				delete m_expBullets[i];
				m_expBullets[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] != nullptr)
		{
			if (m_enmBullets[i]->IsGarbage())
			{
				delete m_enmBullets[i];
				m_enmBullets[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			if (m_asteroids[i]->IsGarbage())
			{
				delete m_asteroids[i];
				m_asteroids[i] = nullptr;
			}
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			if (m_beetles[i]->IsGarbage())
			{
				delete m_beetles[i];
				m_beetles[i] = nullptr;
			}
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			if (m_wasps[i]->IsGarbage())
			{
				delete m_wasps[i];
				m_wasps[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_SHOOTWASPS; i++)
	{
		if (m_shootWasps[i] != nullptr)
		{
			if (m_shootWasps[i]->IsGarbage())
			{
				delete m_shootWasps[i];
				m_shootWasps[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			if (m_debris[i]->IsGarbage())
			{
				delete m_debris[i];
				m_debris[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			if (m_expDebris[i]->IsGarbage())
			{
				delete m_expDebris[i];
				m_expDebris[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_TURRET; i++)
	{
		if (m_turret[i] != nullptr)
		{
			if (m_turret[i]->IsGarbage())
			{
				delete m_turret[i];
				m_turret[i] = nullptr;
			}
		}
	}
}

void Game::PlayerShipKeepShoot()
{
	if (m_playerShip->m_Canon->m_level == 1&&!m_playerShip->m_isCanonAutoCoolDown&& m_playerShip->m_magicPoint>2.f)
	{
		CanonShootL1();
	}
	else if (m_playerShip->m_Canon->m_level == 2 && !m_playerShip->m_isCanonAutoCoolDown && m_playerShip->m_magicPoint > 4.f)
	{
		CanonShootL2();
	}
	else if (m_playerShip->m_Canon->m_level == 3 && !m_playerShip->m_isCanonAutoCoolDown && m_playerShip->m_magicPoint > 6.f)
	{
		CanonShootL3();
	}
	else if (m_playerShip->m_Canon->m_level == 4 && !m_playerShip->m_isCanonAutoCoolDown)
	{
		CanonShootL4();
	}
}

void Game::CanonShootL1()
{
	Vertex_PCU curShootingPoint = Vertex_PCU(Vec3(1.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(1, &curShootingPoint, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);

	bool canShoot = false;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			m_bullets[i] = new Bullet(this, curShootingPoint.m_position.x, curShootingPoint.m_position.y);
			m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees;
			canShoot = true;
			m_playerShip->m_magicPoint -= 1;
			m_playerShip->m_isCanonAutoCoolDown = true;
			m_playerShip->m_startCanonCoolDownTime = GetCurrentTimeSeconds();

			g_theAudio->StartSound(playerNormalShootSound);
			break;
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}
}

void Game::CanonShootL2()
{
	Vertex_PCU curShootingPoint_L = Vertex_PCU(Vec3(0.f, 1.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	Vertex_PCU curShootingPoint_R = Vertex_PCU(Vec3(0.f, -1.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(1, &curShootingPoint_L, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);
	TransformVertexArrayXY3D(1, &curShootingPoint_R, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);
	bool canShoot = false;
	int countFlag = 0;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			countFlag++;
			if (countFlag == 1)
			{
				m_bullets[i] = new Bullet(this, curShootingPoint_L.m_position.x, curShootingPoint_L.m_position.y);
				m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees;
			}
			else if (countFlag == 2)
			{
				m_bullets[i] = new Bullet(this, curShootingPoint_R.m_position.x, curShootingPoint_R.m_position.y);
				m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees;
				canShoot = true;
				m_playerShip->m_magicPoint -= 2;
				m_playerShip->m_isCanonAutoCoolDown = true;
				m_playerShip->m_startCanonCoolDownTime = GetCurrentTimeSeconds();

				g_theAudio->StartSound(playerNormalShootSound);
				break;
			}
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}
}

void Game::CanonShootL3()
{
	Vertex_PCU curShootingPoint_M = Vertex_PCU(Vec3(1.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	Vertex_PCU curShootingPoint_L = Vertex_PCU(Vec3(0.f, 1.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	Vertex_PCU curShootingPoint_R = Vertex_PCU(Vec3(0.f, -1.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(1, &curShootingPoint_M, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);
	TransformVertexArrayXY3D(1, &curShootingPoint_L, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);
	TransformVertexArrayXY3D(1, &curShootingPoint_R, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);
	bool canShoot = false;
	int countFlag = 0;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			countFlag++;
			if (countFlag == 1)
			{
				m_bullets[i] = new Bullet(this, curShootingPoint_L.m_position.x, curShootingPoint_L.m_position.y);
				m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees+20.f;
			}
			else if (countFlag == 2)
			{
				m_bullets[i] = new Bullet(this, curShootingPoint_R.m_position.x, curShootingPoint_R.m_position.y);
				m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees-20.f;
			}
			else if (countFlag == 3)
			{
				m_bullets[i] = new Bullet(this, curShootingPoint_M.m_position.x, curShootingPoint_M.m_position.y);
				m_bullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees;
				canShoot = true;
				m_playerShip->m_magicPoint -= 3;
				m_playerShip->m_isCanonAutoCoolDown = true;
				m_playerShip->m_startCanonCoolDownTime = GetCurrentTimeSeconds();

				g_theAudio->StartSound(playerNormalShootSound);
				break;
			}
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}
}

void Game::CanonShootL4()
{
}

void Game::PlayerShipJustShoot()
{
	if (m_playerShip->m_magicPoint > 20.f)
	{
		LaserCanonExplosionShoot();
	}
}

void Game::LaserCanonExplosionShoot()
{
	Vertex_PCU curShootingPoint = Vertex_PCU(Vec3(3.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(1, &curShootingPoint, 1.f, m_playerShip->m_orientationDegrees, m_playerShip->m_position);

	bool canShoot = false;
	for (int i = 0; i <MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] == nullptr)
		{
			m_expBullets[i] = new ExplosionBullet(this, curShootingPoint.m_position.x, curShootingPoint.m_position.y);
			m_expBullets[i]->m_orientationDegrees = m_playerShip->m_orientationDegrees;
			canShoot = true;
			m_playerShip->m_magicPoint -= 20;

			g_theAudio->StartSound(playerExpShootSound);
			break;
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}
}

void Game::ShootWaspShootBullet(ShootWasp& thisShootWasp)
{
	Vertex_PCU curShootingPoint = Vertex_PCU(Vec3(1.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(1, &curShootingPoint, 1.f, thisShootWasp.m_orientationDegrees, thisShootWasp.m_position);

	bool canShoot = false;
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] == nullptr)
		{
			m_enmBullets[i] = new EnemyBullet(this, curShootingPoint.m_position.x, curShootingPoint.m_position.y);
			m_enmBullets[i]->m_orientationDegrees = thisShootWasp.m_orientationDegrees;
			canShoot = true;
			//m_isCanonAutoCoolDown = true;
			//m_startCanonCoolDownTime = GetCurrentTimeSeconds();
			break;
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}

}

void Game::TurretShootBullet(Turret& thisTurret)
{
	Vertex_PCU curShootingPoint[4] = { Vertex_PCU(Vec3(0.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.f,0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.f, 0.f, 0.f), Rgba8(0, 0, 0, 0), Vec2(0.f, 0.f)) };

	TransformVertexArrayXY3D(4, curShootingPoint, 1.f, thisTurret.m_orientationDegrees, thisTurret.m_position);

	bool canShoot = false;
	int index_shootingPoint = 0;
	for (int i = 0; i < MAX_ENMBULLETS; i++)
	{
		if (m_enmBullets[i] == nullptr)
		{
			m_enmBullets[i] = new EnemyBullet(this, curShootingPoint[index_shootingPoint].m_position.x, curShootingPoint[index_shootingPoint].m_position.y);
			m_enmBullets[i]->m_orientationDegrees = thisTurret.m_orientationDegrees+90.f*index_shootingPoint;
			index_shootingPoint++;
			//canShoot = true;
			//m_isCanonAutoCoolDown = true;
			//m_startCanonCoolDownTime = GetCurrentTimeSeconds();
			if (index_shootingPoint > 3)
			{
				canShoot = true;
				break;
			}
			
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}
}

void Game::GenerateEachWave(EnemyNumInWave curWaveEnemyNum)
{
	g_theAudio->StartSound(newWave);
	GenerateAsteroids(curWaveEnemyNum.numAsteroids);
	GenerateBeetles(curWaveEnemyNum.numBeetles);
	GenerateWasps(curWaveEnemyNum.numWasp);
	GenerateShootWasps(curWaveEnemyNum.numShootWasp);
	GenerateTurret(curWaveEnemyNum.numTurret);
}
void Game::GenerateAsteroids(int count)
{
	for (int i = 0; i < count; )
	{
		for (int j = 0; j < MAX_ASTEROIDS; j++)
		{
			if (m_asteroids[j] == nullptr)
			{
				GenerateEachAsteroids(j);
				i++;
				break;
			}
		}
	}
}
void Game::GenerateEachAsteroids(int index)
{
	int initialDirection = m_rng->RollRandomIntInRange(1, 4);//1 L 2 Top 3 R 4 Bot
	Vec2 shipPos = m_playerShip->m_position;
	if (initialDirection == 1)
	{
		float tempY = m_rng->RollRandomFloatInRange(WORLDSPACE_SIZE_BL_Y, WORLDSPACE_SIZE_TR_Y);
		m_asteroids[index] = new Asteroid(this, WORLDSPACE_SIZE_BL_X-ASTEROID_COSMETIC_RADIUS,tempY);
	}
	else if (initialDirection == 2)
	{
		float tempX = m_rng->RollRandomFloatInRange(WORLDSPACE_SIZE_BL_X, WORLDSPACE_SIZE_TR_X);
		m_asteroids[index] = new Asteroid(this, tempX, WORLDSPACE_SIZE_TR_Y+ ASTEROID_COSMETIC_RADIUS);
	}
	else if (initialDirection == 3)
	{
		float tempY = m_rng->RollRandomFloatInRange(WORLDSPACE_SIZE_BL_Y, WORLDSPACE_SIZE_TR_Y);
		m_asteroids[index] = new Asteroid(this, WORLDSPACE_SIZE_TR_X + ASTEROID_COSMETIC_RADIUS, tempY);
	}
	else
	{
		float tempX = m_rng->RollRandomFloatInRange(WORLDSPACE_SIZE_BL_X, WORLDSPACE_SIZE_TR_X);
		m_asteroids[index] = new Asteroid(this, tempX, WORLDSPACE_SIZE_BL_Y - ASTEROID_COSMETIC_RADIUS);
	}
}
void Game::GenerateBeetles(int count)
{
	for (int i = 0; i < count; i++)
	{
		GenerateEachBeetles(i);
	}
}
void Game::GenerateEachBeetles(int index)
{
	int initialDirection = m_rng->RollRandomIntInRange(1, 4);//1 L 2 Top 3 R 4 Bot
	if (initialDirection == 1)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_beetles[index] = new Beetle(this, -BEETLE_COSMETIC_RADIUS, tempY);
	}
	else if (initialDirection == 2)
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_beetles[index] = new Beetle(this, tempX, WORLD_SIZE_Y + BEETLE_COSMETIC_RADIUS);
	}
	else if (initialDirection == 3)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_beetles[index] = new Beetle(this, WORLD_SIZE_X + BEETLE_COSMETIC_RADIUS, tempY);
	}
	else
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_beetles[index] = new Beetle(this, tempX, -BEETLE_COSMETIC_RADIUS);
	}
	Vec2 direction = m_playerShip->m_position - m_beetles[index]->m_position;
	m_beetles[index]->m_orientationDegrees= direction.GetOrientationDegrees();
	m_beetles[index]->m_velocity = Vec2::MakeFromPolarDegrees(m_beetles[index]->m_orientationDegrees, BEETLE_SPEED);
}
void Game::GenerateWasps(int count)
{
	for (int i = 0; i < count; i++)
	{
		GenerateEachWasp(i);
	}
}
void Game::GenerateEachWasp(int index)
{
	int initialDirection = m_rng->RollRandomIntInRange(1, 4);//1 L 2 Top 3 R 4 Bot
	if (initialDirection == 1)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_wasps[index] = new Wasp(this, -WASP_COSMETIC_RADIUS, tempY);
	}
	else if (initialDirection == 2)
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_wasps[index] = new Wasp(this, tempX, WORLD_SIZE_Y + WASP_COSMETIC_RADIUS);
	}
	else if (initialDirection == 3)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_wasps[index] = new Wasp(this, WORLD_SIZE_X + WASP_COSMETIC_RADIUS, tempY);
	}
	else
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_wasps[index] = new Wasp(this, tempX, -WASP_COSMETIC_RADIUS);
	}
	Vec2 direction = m_playerShip->m_position - m_wasps[index]->m_position;
	m_wasps[index]->m_orientationDegrees = direction.GetOrientationDegrees();
	m_wasps[index]->m_velocity = Vec2::MakeFromPolarDegrees(m_wasps[index]->m_orientationDegrees, WASP_SPEED);
}
void Game::GenerateTurret(int count)
{
	for (int i = 0; i < count; i++)
	{
		GenerateEachTurret(i);
	}
}
void Game::GenerateEachTurret(int index)
{
	float posX = m_rng->RollRandomFloatInRange(-180.f,380.f);
	float posY = m_rng->RollRandomFloatInRange(-80.f, 180.f);
	m_turret[index] = new Turret(this, posX, posY);
}
void Game::GenerateDebris(Vec2 const& position, Rgba8 color, int count, float minSpeed,float maxSpeed, 
	float minRadius, float maxRadius,Vec2 const&  oriVelocity)
{
	int j = 0;
  	for (int i = 0; i < count; i++)
	{
		for (; j < MAX_DEBRIS; )
		{
			if (m_debris[j] == nullptr)
			{
				m_debris[j] = new Debris(this, position.x, position.y,minSpeed, maxSpeed, minRadius, maxRadius, color,oriVelocity,1);
				j += 1;
				break;
			}
			j++;
		}
	}
}

void Game::GenerateExpDebris(Vec2 const& position, Rgba8 color, int count, float minSpeed, float maxSpeed, float minRadius, float maxRadius, Vec2 const& oriVelocity)
{
	int j = 0;
	for (int i = 0; i < count; i++)
	{
		for (; j < MAX_EXPDEBRIS; )
		{
			if (m_expDebris[j] == nullptr)
			{
				m_expDebris[j] = new Debris(this, position.x, position.y, minSpeed, maxSpeed, minRadius, maxRadius, color, oriVelocity,2);
				j += 1;
				break;
			}
			j++;
		}
	}
}

void Game::GenerateShootWasps(int count)
{
	for (int i = 0; i < count; i++)
	{
		GenerateEachShootWasps(i);
	}
}

void Game::GenerateEachShootWasps(int index)
{
	int initialDirection = m_rng->RollRandomIntInRange(1, 4);//1 L 2 Top 3 R 4 Bot
	if (initialDirection == 1)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_shootWasps[index] = new ShootWasp(this, -SHOOTWASP_COSMETIC_RADIUS, tempY);
	}
	else if (initialDirection == 2)
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_shootWasps[index] = new ShootWasp(this, tempX, WORLD_SIZE_Y + SHOOTWASP_COSMETIC_RADIUS);
	}
	else if (initialDirection == 3)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_shootWasps[index] = new  ShootWasp(this, WORLD_SIZE_X + SHOOTWASP_COSMETIC_RADIUS, tempY);
	}
	else
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_shootWasps[index] = new  ShootWasp(this, tempX, -SHOOTWASP_COSMETIC_RADIUS);
	}
	Vec2 direction = m_playerShip->m_position - m_shootWasps[index]->m_position;
	m_shootWasps[index]->m_orientationDegrees = direction.GetOrientationDegrees();
	m_shootWasps[index]->m_velocity = Vec2::MakeFromPolarDegrees(m_shootWasps[index]->m_orientationDegrees, SHOOTWASP_SPEED);
}

void Game::GenerateBkgStar()
{
	for (int i = 0; i < 1000; i++)
	{
		float x = m_rng->RollRandomFloatInRange(-400.f, 600.f);
		float y = m_rng->RollRandomFloatInRange(-200.f, 300.f);
		m_bkgStarsL1[i] = new BkgStar(this, Vec2(x, y), 1);

		float x2 = m_rng->RollRandomFloatInRange(-400.f, 600.f);
		float y2 = m_rng->RollRandomFloatInRange(-200.f, 300.f);
		m_bkgStarsL2[i] = new BkgStar(this, Vec2(x2, y2), 2);

		if (i < 300)
		{
			float x4 = m_rng->RollRandomFloatInRange(-400.f, 600.f);
			float y4 = m_rng->RollRandomFloatInRange(-200.f, 300.f);
			m_bkgStarsL4[i] = new BkgStar(this, Vec2(x4, y4), 4);
		}
	}
}

void Game::WaveHandler(int aimWaveState)
{
	//delete all and generate specific wave
	m_waveState = aimWaveState;
	DeleteCurAllEnemies();
	GenerateEachWave(m_enemyNumInEachWaves[aimWaveState-1]);
}

void Game::DeleteCurAllEnemies()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			delete m_beetles[i];
			m_beetles[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			delete m_wasps[i];
			m_wasps[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_SHOOTWASPS; i++)
	{
		if (m_shootWasps[i] != nullptr)
		{
			delete m_shootWasps[i];
			m_shootWasps[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_TURRET; i++)
	{
		if (m_turret[i] != nullptr)
		{
			delete m_turret[i];
			m_turret[i] = nullptr;
		}
	}
}

void Game::CheckBulletsVsEnemies()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			Vec2 debrisOriVel = m_bullets[i]->m_velocity.GetRotatedDegrees(180.f);
			for (int j = 0; j < MAX_ASTEROIDS; j++)
			{
				if (m_asteroids[j] != nullptr && !m_asteroids[j]->m_isInvincibleState)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_asteroids[j]->m_position, m_asteroids[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						//m_asteroids[j]->m_health--;	
						m_asteroids[j]->GetHurt(8.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_BEETLES; j++)
			{
				if (m_beetles[j] != nullptr)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_beetles[j]->m_position, m_beetles[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						//m_beetles[j]->m_health--;
						m_beetles[j]->GetHurt(8.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_WASPS; j++)
			{
				if (m_wasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_wasps[j]->m_position, m_wasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						//m_wasps[j]->m_health--;
						m_wasps[j]->GetHurt(8.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_SHOOTWASPS; j++)
			{
				if (m_shootWasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_shootWasps[j]->m_position, m_shootWasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						m_shootWasps[j]->m_health--;
						m_shootWasps[j]->GetHurt(8.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_TURRET; j++)
			{
				if (m_turret[j] != nullptr)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_turret[j]->m_position, m_turret[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						m_turret[j]->m_health--;
						m_turret[j]->GetHurt(8.f);
						break;
					}
				}
			}
			
		}
	}
}

void Game::CheckPlayerShipVsEnemies()
{
	if (m_playerShip->IsAlive())
	{
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr&&!m_asteroids[i]->m_isInvincibleState)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_asteroids[i]->m_position, m_asteroids[i]->m_physicsRadius)
					&&!m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(10.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					//m_asteroids[i]->m_health--;
					m_asteroids[i]->GetHurt(15.f);
					break;
				}
			}
		}

		for (int j = 0; j < MAX_BEETLES; j++)
		{
			if (m_beetles[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_beetles[j]->m_position, m_beetles[j]->m_physicsRadius)
					&& !m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(15.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					//m_beetles[j]->m_health--;
					m_beetles[j]->GetHurt(10.f);
					break;
				}
			}
		}

		for (int j = 0; j < MAX_WASPS; j++)
		{
			if (m_wasps[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_wasps[j]->m_position, m_wasps[j]->m_physicsRadius)
					&& !m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(15.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					//m_wasps[j]->m_health--;
					m_wasps[j]->GetHurt(10.f);
					break;
				}
			}
		}

		for (int j = 0; j < MAX_SHOOTWASPS; j++)
		{
			if (m_shootWasps[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_shootWasps[j]->m_position, m_shootWasps[j]->m_physicsRadius)
					&& !m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(15.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					m_shootWasps[j]->m_health--;
					m_shootWasps[j]->GetHurt(10.f);
					break;
				}
			}
		}

		for (int j = 0; j < MAX_TURRET; j++)
		{
			if (m_turret[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_turret[j]->m_position, m_turret[j]->m_physicsRadius)
					&& !m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(15.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					//m_turret[j]->m_health--;
					m_turret[j]->GetHurt(10.f);
					break;
				}
			}
		}

		for (int j = 0; j < MAX_ENMBULLETS; j++)
		{
			if (m_enmBullets[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_enmBullets[j]->m_position, m_enmBullets[j]->m_physicsRadius)
					&& !m_playerShip->m_isInvincibleState)
				{
					m_playerShip->GetHurt(15.f);
					if (m_playerShip->m_health <= 0.f)
					{
						m_playerShip->Die();
					}
					m_enmBullets[j]->Die();
					//m_playerShip->m_health--;
					break;
				}
			}
		}
	}
}

void Game::CheckExpBulletsVsEnemies()
{
	for (int i = 0; i < MAX_EXPBULLETS; i++)
	{
		if (m_expBullets[i] != nullptr)
		{
			Vec2 debrisOriVel = m_expBullets[i]->m_velocity.GetRotatedDegrees(180.f);
			for (int j = 0; j < MAX_ASTEROIDS; j++)
			{
				if (m_asteroids[j] != nullptr && !m_asteroids[j]->m_isInvincibleState)
				{
					if (DoDiscsOverlap(m_expBullets[i]->m_position, m_expBullets[i]->m_physicsRadius, m_asteroids[j]->m_position, m_asteroids[j]->m_physicsRadius))
					{
						GenerateDebris(m_expBullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expBullets[i]->Die();
						//m_asteroids[j]->m_health--;	
						//explosion debris
						GenerateExpDebris(m_asteroids[j]->m_position, Rgba8(226,134, 215, 255),
								5, 55.f, 85.f, 1.5f, 2.0f, m_asteroids[j]->m_velocity);
						m_asteroids[j]->GetHurt(30.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_BEETLES; j++)
			{
				if (m_beetles[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expBullets[i]->m_position, m_expBullets[i]->m_physicsRadius, m_beetles[j]->m_position, m_beetles[j]->m_physicsRadius))
					{
						GenerateDebris(m_expBullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expBullets[i]->Die();
						m_beetles[j]->m_health--;
						break;
					}
				}
			}

			for (int j = 0; j < MAX_WASPS; j++)
			{
				if (m_wasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expBullets[i]->m_position, m_expBullets[i]->m_physicsRadius, m_wasps[j]->m_position, m_wasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_expBullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expBullets[i]->Die();
						m_wasps[j]->m_health--;
						break;
					}
				}
			}

			for (int j = 0; j < MAX_SHOOTWASPS; j++)
			{
				if (m_shootWasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expBullets[i]->m_position, m_expBullets[i]->m_physicsRadius, m_shootWasps[j]->m_position, m_shootWasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_expBullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expBullets[i]->Die();
						m_shootWasps[j]->m_health--;
						break;
					}
				}
			}

			for (int j = 0; j < MAX_TURRET; j++)
			{
				if (m_turret[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expBullets[i]->m_position, m_expBullets[i]->m_physicsRadius, m_turret[j]->m_position, m_turret[j]->m_physicsRadius))
					{
						GenerateDebris(m_expBullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expBullets[i]->Die();
						m_turret[j]->m_health--;
						break;
					}
				}
			}
		}
	}
}

void Game::CheckExpDebrisVSEnemies()
{
	for (int i = 0; i <MAX_EXPDEBRIS; i++)
	{
		if (m_expDebris[i] != nullptr)
		{
			Vec2 debrisOriVel = m_expDebris[i]->m_velocity.GetRotatedDegrees(180.f);
			for (int j = 0; j < MAX_ASTEROIDS; j++)
			{
				if (m_asteroids[j] != nullptr && !m_asteroids[j]->m_isInvincibleState)
				{
					if (DoDiscsOverlap(m_expDebris[i]->m_position, m_expDebris[i]->m_physicsRadius, m_asteroids[j]->m_position, m_asteroids[j]->m_physicsRadius))
					{
						GenerateDebris(m_expDebris[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expDebris[i]->Die();
						//m_asteroids[j]->m_health--;	
						m_asteroids[j]->GetHurt(5.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_BEETLES; j++)
			{
				if (m_beetles[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expDebris[i]->m_position, m_expDebris[i]->m_physicsRadius, m_beetles[j]->m_position, m_beetles[j]->m_physicsRadius))
					{
						GenerateDebris(m_expDebris[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expDebris[i]->Die();
						//m_beetles[j]->m_health--;
						m_beetles[j]->GetHurt(5.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_WASPS; j++)
			{
				if (m_wasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expDebris[i]->m_position, m_expDebris[i]->m_physicsRadius, m_wasps[j]->m_position, m_wasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_expDebris[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expDebris[i]->Die();
						//m_wasps[j]->m_health--;
						m_wasps[j]->GetHurt(5.f);
						break;
					}
				}
			}

			for (int j = 0; j < MAX_SHOOTWASPS; j++)
			{
				if (m_shootWasps[j] != nullptr)
				{
					if (DoDiscsOverlap(m_expDebris[i]->m_position, m_expDebris[i]->m_physicsRadius, m_shootWasps[j]->m_position, m_shootWasps[j]->m_physicsRadius))
					{
						GenerateDebris(m_expDebris[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_expDebris[i]->Die();
						//m_shootWasps[j]->m_health--;
						m_shootWasps[j]->GetHurt(5.f);
						break;
					}
				}
			}

		}
	}
}

void Game::CheckLightSaberVsEnemies()
{
	if (m_playerShip->m_lightSaber->m_level == 1)
	{
		AABB2 oriLightSaber = AABB2(Vec2(1.f, -LIGHTSABER_WIDTH / 2.f), Vec2(1.f + LIGHTSABER_LEN, LIGHTSABER_WIDTH / 2.f));
		
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < MAX_ASTEROIDS; j++)
			{

				if (m_asteroids[j] != nullptr && !m_asteroids[j]->m_isInvincibleState)
				{
					Vec2  curEntityCenter = m_asteroids[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;

					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();

					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));

					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_asteroids[j]->m_physicsRadius))
					{
						m_asteroids[j]->GetHurt(15.f);
					}
				}
			}
			for (int j = 0; j < MAX_BEETLES; j++)
			{
				if (m_beetles[j] != nullptr)
				{
					Vec2  curEntityCenter = m_beetles[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;
					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();
					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));
					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_beetles[j]->m_physicsRadius))
					{
						//m_beetles[j]->m_health-=1;
						m_beetles[j]->GetHurt(1.f);
					}
				}
			}
			for (int j = 0; j < MAX_WASPS; j++)
			{
				if (m_wasps[j] != nullptr)
				{
					Vec2  curEntityCenter = m_wasps[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;
					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();
					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));
					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_wasps[j]->m_physicsRadius))
					{
						//m_wasps[j]->m_health -= 1;
						m_wasps[j]->GetHurt(1.f);
					}
				}
			}
			for (int j = 0; j < MAX_SHOOTWASPS; j++)
			{
				if (m_shootWasps[j] != nullptr)
				{
					Vec2  curEntityCenter = m_shootWasps[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;
					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();
					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));
					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_shootWasps[j]->m_physicsRadius))
					{
						//m_shootWasps[j]->m_health -= 1;
						m_shootWasps[j]->GetHurt(1.f);
					}
				}
			}
			for (int j = 0; j < MAX_ENMBULLETS; j++)
			{
				if (m_enmBullets[j] != nullptr)
				{
					Vec2  curEntityCenter = m_enmBullets[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;
					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();
					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));
					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_enmBullets[j]->m_physicsRadius))
					{
						m_enmBullets[j]->Die();
					}
				}
			}
			for (int j = 0; j < MAX_TURRET; j++)
			{
				if (m_turret[j] != nullptr)
				{
					Vec2  curEntityCenter = m_turret[j]->m_position;
					curEntityCenter -= m_playerShip->m_position;
					Vec2 iBasis = Vec2::MakeFromPolarDegrees(LIGHTSABER_L1_SHIFT + i * 120.f + m_playerShip->m_orientationDegrees);
					Vec2 jBasis = iBasis.GetRotated90Degrees();
					Vec2 rotatedBackEntityCenter = Vec2(DotProduct2D(iBasis, curEntityCenter), DotProduct2D(jBasis, curEntityCenter));
					Vec2 nearPoint = oriLightSaber.GetNearestPoint(rotatedBackEntityCenter);
					if (IsPointInsideDisc2D(nearPoint, rotatedBackEntityCenter, m_turret[j]->m_physicsRadius))
					{
						//m_turret[j]->m_health -= 1;
						m_turret[j]->GetHurt(1.f);
					}
				}
			}
		}
	}
}

void Game::ShakeScreen()
{
	m_isShakeScreen = true;
	m_startShakeTime = GetCurrentTimeSeconds();
}





