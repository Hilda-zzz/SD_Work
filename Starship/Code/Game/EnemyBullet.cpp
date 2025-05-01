#include "Game/EnemyBullet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

EnemyBullet::EnemyBullet(Game* game, float x, float y) : Entity(game, x, y)
{
	m_cosmeticRadius =	ENEMYBULLET_COSMETIC_RADIUS;
	m_physicsRadius = ENEMYBULLET_PHYSICS_RADIUS;
	m_orientationDegrees = 0;
	InitializedVerts(vertices);
}

void EnemyBullet::Update(float deltaTime)
{
	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_SPEED);
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;
	if (IsOffscreen())
	{
		Die();
	}
}

void EnemyBullet::Render() const
{
	Vertex_PCU temp_vertices[NUM_BULLET_VERTS];
	for (int i = 0; i < NUM_BULLET_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_BULLET_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, temp_vertices);
}

void EnemyBullet::Die()
{
	m_isGarbage = true;
}

void EnemyBullet::InitializedVerts(Vertex_PCU* vertsToFillIn)
{
	vertsToFillIn[0] = Vertex_PCU(Vec3(-3.f, 0, 0.f), Rgba8(0, 0, 255, 0), Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(0, -0.5, 0.f), Rgba8(22, 212, 38, 255), Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(0, 0.5, 0.f), Rgba8(22, 212, 38, 255), Vec2(0.f, 0.f));
	
	vertsToFillIn[3] = Vertex_PCU(Vec3(0, -0.5, 0.f), Rgba8(22, 212, 38, 255), Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(0.5, 0, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(0, 0.5, 0.f), Rgba8(22, 212, 38, 255), Vec2(0.f, 0.f));
}
