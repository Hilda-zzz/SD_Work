#include "PlayerShip.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Game.hpp"
#include <Engine/Input/InputSystem.hpp>
#include <Engine/Core/Time.hpp>

extern Renderer* g_theRenderer;
extern App* g_theApp;
extern InputSystem* g_theInput;
PlayerShip::PlayerShip(Game* game, float x, float y): Entity(game, x, y) 
{
	//can use initialize list
	m_orientationDegrees = 0;
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_health = 4;
	m_thrustFraction = 0.f;
	vertices[0] = Vertex_PCU(Vec3(-2, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(0, 2, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(2, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));

	vertices[3] = Vertex_PCU(Vec3(-2, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(-2, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(0, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));

	vertices[6] = Vertex_PCU(Vec3(-2, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[7] = Vertex_PCU(Vec3(0, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[8] = Vertex_PCU(Vec3(0, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));

	vertices[9] = Vertex_PCU(Vec3(0, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[10] = Vertex_PCU(Vec3(1, 0, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[11] = Vertex_PCU(Vec3(0, 1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));

	vertices[12] = Vertex_PCU(Vec3(-2, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[13] = Vertex_PCU(Vec3(2, -1, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	vertices[14] = Vertex_PCU(Vec3(0, -2, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
}

void PlayerShip::Update(float deltaTime) 
{
	UpdateFromController(deltaTime);
	if (m_isDead == false)
	{
		//DetectAsteroids();
		//--------------------------------------------------------------------------------------
		//left rotate
		m_isShipRotateLeft = g_theInput->IsKeyDown('S');
		//right rotate
		m_isShipRotateRight = g_theInput->IsKeyDown('F');
		//accelerate
		m_isAccelerate = g_theInput->IsKeyDown('E');
		//shoot
		m_isShoot = g_theInput->WasKeyJustPressed(32);
	
		//respawn
		
		if (m_isShipRotateLeft)
		{
			m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaTime;
		}

		if (m_isShipRotateRight)
		{
			m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED * deltaTime;
		}

		if (m_isAccelerate)
		{
			m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaTime);
		}

		BounceOffWall();

		m_position.x += m_velocity.x * deltaTime;
		m_position.y += m_velocity.y * deltaTime;

		if (m_isShoot)
		{
			m_game->Shoot();
		}
	}
	
	if (m_isDead == true && g_theInput->WasKeyJustPressed('N') && m_health > 0)
	{
		RespawnShip();
	}

	if (m_isDead && m_health <= 1&&m_completeDead==false)
	{
		//wait to return menu
		m_completeDead = true;
		m_deadTime= GetCurrentTimeSeconds();
	}
	if (m_completeDead == true)
	{
		float curTime= GetCurrentTimeSeconds();
		if (curTime - m_deadTime >= 2)
		{
			//m_game->m_isArractMode = true;
			m_game->m_flagToReset = true;
		}
	}
}

void PlayerShip::UpdateFromController(float deltaSeconds)
{
	//UNUSED(deltaSceonds);
	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		if (m_isDead)
		{
			if (controller.WasButtonJustPressed(XboxButtonID::START) && m_health > 0)
			{
				RespawnShip();
			}
			return;
		}

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f&&!m_isShipRotateLeft&&!m_isShipRotateRight)
		{
			m_thrustFraction = leftStickMagnitude;
			m_orientationDegrees = controller.GetLeftStick().GetOrientationDegrees();
			m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaSeconds*m_thrustFraction);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::A))
		{
			m_game->Shoot();
		}
	}
}

void PlayerShip::Render() const
{
	if (m_isDead == false)
	{
		Vertex_PCU temp_vertices[NUM_SHIP_VERTS];
		for (int i = 0; i < NUM_SHIP_VERTS; i++)
		{
			temp_vertices[i] = vertices[i];
		}

		TransformVertexArrayXY3D(NUM_SHIP_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, temp_vertices); //NUM_SHIP_VERTS
	}	
}

void PlayerShip::Die() 
{
	m_game->GenerateDebris(m_position, Rgba8(102, 153, 204, 130), 8, 15.f, 35.f, 1.5f, 2.f, m_velocity);
	m_isDead = true;
	//m_game->SpawnNewDebrisCluster(10,m_position,m_velocity,10.f,10.f,m_color)
}

void PlayerShip::BounceOffWall()
{
	if (m_position.x < PLAYER_SHIP_PHYSICS_RADIUS )
	{
		m_position.x = PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.x > WORLD_SIZE_X- PLAYER_SHIP_PHYSICS_RADIUS)
	{
		m_position.x = WORLD_SIZE_X - PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.y < PLAYER_SHIP_PHYSICS_RADIUS )
	{
		m_position.y = PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.y = -m_velocity.y;
	}
	if (m_position.y > WORLD_SIZE_Y - PLAYER_SHIP_PHYSICS_RADIUS)
	{
		m_position.y = WORLD_SIZE_Y - PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.y = -m_velocity.y;
	}
}


void PlayerShip::RespawnShip()
{
	if (m_health >= 1)
	{
		m_health--;
	}
	m_orientationDegrees = 0;
	m_velocity.x = 0;
	m_velocity.y = 0;
	m_position.x = WORLD_CENTER_X;
	m_position.y = WORLD_CENTER_Y;
	m_isDead = false;
}

void PlayerShip::InitializedVerts(Vertex_PCU* vertsToFillIn, Rgba8 const& color)
{
	vertsToFillIn[0] = Vertex_PCU(Vec3(-2, 1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(0, 2, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(2, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[3] = Vertex_PCU(Vec3(-2, 1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[6] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[7] = Vertex_PCU(Vec3(0, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[8] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[9] = Vertex_PCU(Vec3(0, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[10] = Vertex_PCU(Vec3(1, 0, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[11] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[12] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[13] = Vertex_PCU(Vec3(2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[14] = Vertex_PCU(Vec3(0, -2, 0.f), color, Vec2(0.f, 0.f));
}
