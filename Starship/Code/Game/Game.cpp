#include <Game/Game.hpp>
#include "App.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include"Engine/Math/RandomNumberGenerator.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Core/Time.hpp>
extern App* g_theApp;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;
Game::Game()
{
	//m_playerShip = new PlayerShip(this,WORLD_CENTER_X,WORLD_CENTER_Y);
	//m_rng = new RandomNumberGenerator();
	//GenerateAsteroids(WAVE1_ASTEROIDS);
	//GenerateBeetles(WAVE1_BEETLES);
	//GenerateWasps(WAVE1_WASP);
	//m_isAsteroidsClear = false;
	//m_isBeetlesClear = false;
	//m_isWaspsClear = false;
	startTime = GetCurrentTimeSeconds();
	m_isArractMode = true;
}

Game::~Game()
{
	delete m_playerShip;
	m_playerShip = nullptr;

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		delete m_bullets[i];
		m_bullets[i] = nullptr;
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		delete m_asteroids[i];
		m_asteroids[i] = nullptr;
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		delete m_beetles[i];
		m_beetles[i] = nullptr;
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		delete m_wasps[i];
		m_wasps[i] = nullptr;
	}
	delete m_rng;
	m_rng = nullptr;
}

void Game::InitGameplay()
{
	m_playerShip = new PlayerShip(this, WORLD_CENTER_X, WORLD_CENTER_Y);
	m_rng = new RandomNumberGenerator();
	GenerateAsteroids(WAVE1_ASTEROIDS);
	GenerateBeetles(WAVE1_BEETLES);
	GenerateWasps(WAVE1_WASP);
	m_isAsteroidsClear = false;
	m_isBeetlesClear = false;
	m_isWaspsClear = false;
	//startTime = GetCurrentTimeSeconds();
}

void Game::ResetGameplay()
{
	m_isArractMode = true;
	m_playerShip->m_completeDead = false;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;
			
		}
	}
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
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
			
		}
	}
	delete m_playerShip;
	m_playerShip = nullptr;
}

void Game::Update(float deltaSeconds)
{
	if (!m_isArractMode)
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

		m_playerShip->Update(deltaSeconds);
		UpdateBullet(deltaSeconds);
		UpdateAsteroids(deltaSeconds);
		UpdateBeetles(deltaSeconds);
		UpdateWasps(deltaSeconds);
		UpdateDebris(deltaSeconds);
		CheckBulletsVsAsteroids();
		CheckPlayerShipVsAsteroids();

		if (m_isAsteroidsClear && m_isBeetlesClear && m_isWaspsClear)
		{
			m_waveState++;
			if (m_waveState <= 3)
			{
				GenerateNewWave(m_waveState);
			}
		}

		if (g_theInput->WasKeyJustPressed(0x70))
		{
			//debug mode
			isDebugMode = !isDebugMode;
		}

		//order is important(?)
		DeleteGarbageEntities();

		//if (g_theInput->WasKeyJustPressed(0x77))
		//{
		//	//reset
		//	g_theApp->Shutdown();
		//	delete g_theApp;
		//	g_theApp = nullptr;
		//	g_theApp = new App();
		//	g_theApp->Startup();
		//}

		if (g_theInput->WasKeyJustPressed(27)||m_flagToReset==true)
		{
			//m_isArractMode = true;
			m_flagToReset = false;
			//destroy
			ResetGameplay();
		}
	}
	else
	{
		UpdateAttractMode(deltaSeconds);
	}
	
}

void Game::Renderer()const
{
	if (!m_isArractMode)
	{
		RenderBullet();
		m_playerShip->Render();
		RenderAsteroids();
		RenderBeetles();
		RenderWasps();
		RenderDebris();
		RenderUI();
		//RenderHealthIcon();
		if (isDebugMode)
		{
			RenderDebugMode();
		}
	}
	else
	{
		RenderAttractMode();
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

//x
void Game::RenderEnemyDebugMode(int count,Entity* entityGroup[]) const
{
	Rgba8 color_grey{ 50,50,50,255 };
	Rgba8 color_red{ 255,0,0,255 };
	Rgba8 color_green{ 0,255,0,255 };
	Rgba8 color_yellow{ 255, 255, 0, 255 };
	Rgba8 color_magenta{ 255, 0, 255, 255 };
	Rgba8 color_cyan{ 0, 255, 255, 255 };
	for (int i = 0; i < count; i++)
	{
		//grey line
		if (entityGroup[i] != nullptr)
		{
			DebugDrawLine(m_playerShip->m_position, entityGroup[i]->m_position, 0.2f, color_grey);
		}
	}
}

void Game::RenderUI() const
{
	Vec2 oriIconPos{ 5.f,95.f };
	for (int i = 0; i <m_playerShip->m_health - 1; i++)
	{
		Vertex_PCU playerShipIcon[NUM_SHIP_VERTS];
		PlayerShip::InitializedVerts(&playerShipIcon[0], Rgba8(102, 153, 204, 255));
		TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon, 1.f, 90.f, oriIconPos + Vec2{ i * 5.f,0.f }); //
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon); //NUM_SHIP_VERTS
	}
}
void Game::RenderAttractMode() const
{
	Vertex_PCU playerShipIcon[NUM_SHIP_VERTS];
	PlayerShip::InitializedVerts(&playerShipIcon[0], Rgba8(102, 153, 204, 255));
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon, 10.f, 0.f, Vec2(40.f,50.f)); 
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon); 

	Vertex_PCU playerShipIcon2[NUM_SHIP_VERTS];
	PlayerShip::InitializedVerts(&playerShipIcon2[0], Rgba8(102, 153, 204, 255));
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, playerShipIcon2, 10.f, 180.f, Vec2(160.f, 50.f));
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, playerShipIcon2);


	unsigned char uc_a = static_cast<unsigned char>(curAttractTriangle_a);
	Vertex_PCU triangleGreen[3];
	triangleGreen[0] = Vertex_PCU(Vec3(85, 35, 0.f), Rgba8(0,200,130, uc_a), Vec2(0.f, 0.f));
	triangleGreen[1] = Vertex_PCU(Vec3(85, 65, 0.f), Rgba8(0, 200, 130, uc_a), Vec2(0.f, 0.f));
	triangleGreen[2] = Vertex_PCU(Vec3(115, 50, 0.f), Rgba8(0, 200, 130, uc_a), Vec2(0.f, 0.f));
	g_theRenderer->DrawVertexArray(3, triangleGreen);
}
//void Game::RenderHealthIcon() const
//{
//	//Vec2 oriIconPosition  { 20,180 };
//	//for (int i = 0; i < m_playerShip->m_health - 1; i++)
//	//{
//	//	Vertex_PCU temp_vertices[NUM_SHIP_VERTS];
//	//	for (int i = 0; i < NUM_SHIP_VERTS; i++)
//	//	{
//	//		temp_vertices[i] = vertices[i];
//	//	}
//
//	//	TransformVertexArrayXY3D(NUM_SHIP_VERTS, temp_vertices, 1.f, 90.f, oriIconPosition);
//
//	//	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, temp_vertices); //NUM_SHIP_VERTS
//	//}
//}
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

void Game::UpdateDebris(float deltaTime)
{
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			m_debris[i]->Update(deltaTime);
		}
	}
}

void Game::UpdateAttractMode(float deltaTime)
{
	//m_playerShip->m_completeDead = false;
	currentTime += deltaTime;
	float temp = RangeMap(SinDegrees(currentTime * 200.f), -1.f, 1.f, 0.f, 1.f);
	curAttractTriangle_a = 255.f * temp;

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
}

void Game::Shoot()
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
			break;
		}
	}
	if (canShoot == false)
	{
		//bullet reach the max num
		ERROR_RECOVERABLE("no more bullet");
	}

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
	if (initialDirection == 1)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_asteroids[index] = new Asteroid(this, -ASTEROID_COSMETIC_RADIUS, tempY);
	}
	else if (initialDirection == 2)
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_asteroids[index] = new Asteroid(this, tempX, WORLD_SIZE_Y + ASTEROID_COSMETIC_RADIUS);
	}
	else if (initialDirection == 3)
	{
		float tempY = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_Y);
		m_asteroids[index] = new Asteroid(this, WORLD_SIZE_X + ASTEROID_COSMETIC_RADIUS, tempY);
	}
	else
	{
		float tempX = m_rng->RollRandomFloatInRange(0, WORLD_SIZE_X);
		m_asteroids[index] = new Asteroid(this, tempX, -ASTEROID_COSMETIC_RADIUS);
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

void Game::GenerateDebris(Vec2& const position, Rgba8 color, int count, float minSpeed,float maxSpeed, float minRadius, float maxRadius,Vec2& const oriVelocity)
{
	int j = 0;
	for (int i = 0; i < count; i++)
	{
		for (; j < MAX_DEBRIS; )
		{
			if (m_debris[j] == nullptr)
			{
				m_debris[j] = new Debris(this, position.x, position.y,minSpeed, maxSpeed, minRadius, maxRadius, color,oriVelocity);
				j += 1;
				break;
			}
			j++;
		}
	}
}

void Game::GenerateNewWave(int state)
{
	if (state == 1)
	{
		GenerateAsteroids(WAVE1_ASTEROIDS);
		GenerateBeetles(WAVE1_BEETLES);
		GenerateWasps(WAVE1_WASP);
	}
	else if (state == 2)
	{
		GenerateAsteroids(WAVE2_ASTEROIDS);
		GenerateBeetles(WAVE2_BEETLES);
		GenerateWasps(WAVE2_WASP);
	}
	else if (state == 3)
	{
		GenerateAsteroids(WAVE3_ASTEROIDS);
		GenerateBeetles(WAVE3_BEETLES);
		GenerateWasps(WAVE3_WASP);
	}
}

void Game::CheckBulletsVsAsteroids()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			Vec2 debrisOriVel = m_bullets[i]->m_velocity.GetRotatedDegrees(180.f);
			for (int j = 0; j < MAX_ASTEROIDS; j++)
			{
				if (m_asteroids[j] != nullptr)
				{
					if (DoDiscsOverlap(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, m_asteroids[j]->m_position, m_asteroids[j]->m_physicsRadius))
					{
						GenerateDebris(m_bullets[i]->m_position, Rgba8(255, 120, 100, 130), 5, 10.f, 25.f, 0.1f, 0.3f, debrisOriVel);
						m_bullets[i]->Die();
						m_asteroids[j]->m_health--;								
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
						m_beetles[j]->m_health--;
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
						m_wasps[j]->m_health--;
						break;
					}
				}
			}
		}
	}
}

void Game::CheckPlayerShipVsAsteroids()
{
	if (m_playerShip->IsAlive())
	{
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_asteroids[i]->m_position, m_asteroids[i]->m_physicsRadius))
				{
					m_playerShip->Die();
					m_asteroids[i]->m_health--;
					break;
				}
			}
		}

		for (int j = 0; j < MAX_BEETLES; j++)
		{
			if (m_beetles[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_beetles[j]->m_position, m_beetles[j]->m_physicsRadius))
				{
					m_playerShip->Die();
					m_beetles[j]->m_health--;
					//m_playerShip->m_health--;
					break;
				}
			}
		}

		for (int j = 0; j < MAX_WASPS; j++)
		{
			if (m_wasps[j] != nullptr)
			{
				if (DoDiscsOverlap(m_playerShip->m_position, m_playerShip->m_physicsRadius, m_wasps[j]->m_position, m_wasps[j]->m_physicsRadius))
				{
					m_playerShip->Die();
					m_wasps[j]->m_health--;
					//m_playerShip->m_health--;
					break;
				}
			}
		}
	}
}

void Game::CheckBulletsVsBeetles()
{

}

void Game::CheckPlayerShipVsBeetles()
{

}



