#include "Asteroid.hpp"
#include <Engine/Math/RandomNumberGenerator.hpp>
#include "Game.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include <Engine/Math/MathUtils.hpp>
extern Renderer* g_theRenderer;
Asteroid::Asteroid(Game* game, float x, float y) : Entity(game, x, y)
{
	m_orientationDegrees = game->m_rng->RollRandomFloatInRange(0.f,360.f);
	m_angularVelocity = game->m_rng->RollRandomFloatInRange(-200.f, 200.f);
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_health = 3;
	float velocity_angle= game->m_rng->RollRandomFloatInRange(0.f, 360.f);
	m_velocity = Vec2::MakeFromPolarDegrees(velocity_angle, ASTEROID_SPEED);

	//m_velocity = Vec2{ 0.f,0.f };

	int j = 0;
	for (int i = 0; i < NUM_ASTEROID_VERTS / 3; i++)
	{
		m_point_len[i] = game->m_rng->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	}
	m_point_len[(NUM_ASTEROID_VERTS / 3)] = m_point_len[0];
	for (int i = 0; i < NUM_ASTEROID_VERTS; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(22.5f * j, m_point_len[j]);
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(22.5f * (j+1), m_point_len[j+1]);
		vertices[i] = Vertex_PCU(Vec3(0, 0, 0.f), Rgba8(100, 100, 100, 255), Vec2(0.f, 0.f));
		vertices[i+1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), Rgba8(100, 100, 100, 255), Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), Rgba8(100, 100, 100, 255 ), Vec2(0.f, 0.f));
		i += 3;
		j+=1;
	}
}

void Asteroid::Update(float deltaTime)
{
	m_orientationDegrees += m_angularVelocity * deltaTime;
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;
	if (IsOffscreen())
	{
		WrapAround();
	}
	//DetectPlayerShip();
	if (m_health == 0)
	{
		Die();
	}
}

void Asteroid::Render() const
{
	Vertex_PCU temp_vertices[NUM_ASTEROID_VERTS];
	for (int i = 0; i < NUM_ASTEROID_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void Asteroid::Die()
{
	//which one???
	//m_isDead = true;
	m_game->GenerateDebris(m_position, Rgba8(100, 100, 100, 130), 8, 10.f, 25.f, 1.5f, 2.0f, m_velocity);
	m_isGarbage = true;
}

//void Asteroid::DetectPlayerShip()
//{
//	if (m_game->m_playerShip != nullptr)
//	{
//		if (DoDiscsOverlap(m_position, m_physicsRadius, m_game->m_playerShip->m_position, m_game->m_playerShip->m_physicsRadius)
//			&& m_game->m_playerShip->IsAlive())
//		{
//			m_health--;
//			if (m_health == 0)
//			{
//				Die();
//			}
//		}
//	}
//	
//}

void Asteroid::WrapAround()
{
	if (m_position.x < -m_cosmeticRadius)
	{
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
	}
	else if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius)
	{
		m_position.x = -m_cosmeticRadius;
	}
	else if (m_position.y < -m_cosmeticRadius)
	{
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
	}
	else if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius)
	{
		m_position.y = -m_cosmeticRadius;
	}
}

//void Asteroid::DetectBullets()
//{
//	for (int i = 0; i < MAX_BULLETS; i++)
//	{
//		if (m_game->m_bullets[i] != nullptr)
//		{
//			if (DoDiscsOverlap(m_position, m_physicsRadius, m_game->m_bullets[i]->m_position, m_game->m_bullets[i]->m_physicsRadius))
//			{
//				m_health--;
//				if (m_health == 0)
//				{
//					Die();
//					break;
//				}
//			}
//		}
//	}
//}
