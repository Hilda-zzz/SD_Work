#include "Bullet.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Game.hpp"

extern Renderer* g_theRenderer;
extern App* g_theApp;
Bullet::Bullet(Game* game, float x, float y) : Entity(game, x, y)
{
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_orientationDegrees = 0;
	//m_velocity = Vec2{ 0.f,0.f };

	vertices[0] = Vertex_PCU(Vec3(-2, 0, 0.f), Rgba8(255, 0, 0,0), Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(0, -0.5, 0.f), Rgba8(255, 0, 0,255), Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(0, 0.5, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
	vertices[3] = Vertex_PCU(Vec3(0, -0.5, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(0, 0.5, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(0.5, 0, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
}

void Bullet::Update(float deltaTime)
{
	m_velocity=Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_SPEED);
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;
	if (IsOffscreen())
	{
		Die();
	}
}

void Bullet::Render() const
{
	Vertex_PCU temp_vertices[NUM_BULLET_VERTS];
	for (int i = 0; i < NUM_BULLET_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_BULLET_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void Bullet::Die()
{
	m_isGarbage = true;
}

//void Bullet::DetectAsteroid()
//{
//	for (int i = 0; i < MAX_ASTEROIDS; i++)
//	{
//		if (m_game->m_asteroids[i] != nullptr)
//		{
//			if (DoDiscsOverlap(m_position, m_physicsRadius, m_game->m_asteroids[i]->m_position, m_game->m_asteroids[i]->m_physicsRadius))
//			{
//				Die();
//				break;
//			}
//		}
//	}
//}


