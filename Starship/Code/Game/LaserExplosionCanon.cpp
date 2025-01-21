#include "LaserExplosionCanon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"

LaserExplosionCanon::LaserExplosionCanon(Game* game, float x, float y, int level) : Entity(game, x, y)
{
	m_level = level;
	InitializedVertsL1(vertices_L1);
}

void LaserExplosionCanon::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_orientationDegrees = m_game->m_playerShip->m_orientationDegrees;
	m_position = m_game->m_playerShip->m_position;
}

void LaserExplosionCanon::Render() const
{
	if (m_level == 1)
	{
		Vertex_PCU temp_vertices[NUM_LASER_CANON_VERTS];
		for (int i = 0; i < NUM_LASER_CANON_VERTS; i++)
		{
			temp_vertices[i] = vertices_L1[i];
		}
		TransformVertexArrayXY3D(NUM_LASER_CANON_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_LASER_CANON_VERTS, temp_vertices);
	}
}

void LaserExplosionCanon::Die()
{
}

void LaserExplosionCanon::InitializedVertsL1(Vertex_PCU* vertsToFillIn)
{
	//AIMMING LASER
	vertsToFillIn[0] = Vertex_PCU(Vec3(1.f, 0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(90.f, 0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(90.f, -0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));

	vertsToFillIn[3] = Vertex_PCU(Vec3(1.f, 0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(1.f, -0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(90.f, -0.1f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	//C
	vertsToFillIn[6] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[7] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[8] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	//d
	vertsToFillIn[9] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[10] = Vertex_PCU(Vec3(3.f, 0.f, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[11] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	//B
	vertsToFillIn[12] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[13] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
	vertsToFillIn[14] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), Rgba8(172, 234, 233), Vec2(0.f, 0.f));
}
