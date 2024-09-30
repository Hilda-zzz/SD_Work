#include "Wasp.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Game.hpp"

extern Renderer* g_theRenderer;

Wasp::Wasp(Game* game, float x, float y) : Entity(game, x, y)
{
	//can use initialize list
	m_orientationDegrees = 0;
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_health = 3;

	vertices[0] = Vertex_PCU(Vec3(0.f, 1.7, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(1.7, 0.f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(0.f, -1.7, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));

	vertices[3] = Vertex_PCU(Vec3(0.f, 1.1, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(0.f, -1.1, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
}

void Wasp::Update(float deltaTime)
{
	if (m_game->m_playerShip->IsAlive())
	{
		Vec2 direction = m_game->m_playerShip->m_position - m_position;
		m_orientationDegrees = direction.GetOrientationDegrees();
	}

	Vec2 new_velocity=m_velocity+ Vec2::MakeFromPolarDegrees(m_orientationDegrees, 10.f * deltaTime);
	m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, 10.f * deltaTime);

	float cur_speed = new_velocity.GetLength();
	if (cur_speed > WASP_MAX_SPEED)
	{
		float scale = WASP_MAX_SPEED / cur_speed;
		m_velocity.x *= scale; 
		m_velocity.y *= scale;
	}

	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;

	if (m_health == 0)
	{
		Die();
	}
}

void Wasp::Render() const
{
	Vertex_PCU temp_vertices[NUM_WASP_VERTS];
	for (int i = 0; i < NUM_WASP_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_WASP_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_WASP_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void Wasp::Die()
{
	m_game->GenerateDebris(m_position, Rgba8(255, 255, 0, 130), 8, 15.f, 35.f, 1.5f, 2.f, m_velocity);
	m_isGarbage = true;
}