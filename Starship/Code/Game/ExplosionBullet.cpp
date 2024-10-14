#include "ExplosionBullet.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include <Engine/Renderer/Renderer.hpp>
extern Renderer* g_theRenderer;
ExplosionBullet::ExplosionBullet(Game* game, float x, float y) : Entity(game, x, y)
{
	m_cosmeticRadius = EXPBULLET_COSMETIC_RADIUS;
	m_physicsRadius = EXPBULLET_PHYSICS_RADIUS;
	m_orientationDegrees = 0;
	//m_velocity = Vec2{ 0.f,0.f };

	vertices[0] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[3] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(0.5f, 0.f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));

	vertices[6] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[7] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[8] = Vertex_PCU(Vec3(-3.f, 1.f, 0.f), Rgba8(221, 109, 157, 120), Vec2(0.f, 0.f));

	vertices[9] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[10] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), Rgba8(221, 109, 157, 255), Vec2(0.f, 0.f));
	vertices[11] = Vertex_PCU(Vec3(-3.f,-1.f, 0.f), Rgba8(221, 109, 157, 120), Vec2(0.f, 0.f));
}

void ExplosionBullet::Update(float deltaTime)
{
	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, EXPBULLET_SPEED);
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;
	if (IsOffscreen())
	{
		Die();
	}
}

void ExplosionBullet::Render() const
{
	Vertex_PCU temp_vertices[NUM_EXPLOSION_BULLET_VERTS];
	for (int i = 0; i < NUM_EXPLOSION_BULLET_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_EXPLOSION_BULLET_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_EXPLOSION_BULLET_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void ExplosionBullet::Die()
{
	m_isGarbage = true;
}
