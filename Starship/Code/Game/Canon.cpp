#include "Game/Canon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"


Canon::Canon(Game* game, float x, float y, int level) : Entity(game, x, y)
{
	m_level = level;
	InitializedVertsL1(vertices_L1);
	InitializedVertsL2(vertices_L2);
	InitializedVertsL3(vertices_L3);
}

void Canon::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_orientationDegrees = m_game->m_playerShip->m_orientationDegrees;
	m_position = m_game->m_playerShip->m_position;
}

void Canon::Render() const
{
	if (m_level == 1)
	{
		Vertex_PCU temp_vertices[NUM_CANONL1_VERTS];
		for (int i = 0; i < NUM_CANONL1_VERTS; i++)
		{
			temp_vertices[i] = vertices_L1[i];
		}
		TransformVertexArrayXY3D(NUM_CANONL1_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_CANONL1_VERTS, temp_vertices);
	}
	else if (m_level == 2)
	{
		Vertex_PCU temp_vertices[NUM_CANONL2_VERTS];
		for (int i = 0; i < NUM_CANONL2_VERTS; i++)
		{
			temp_vertices[i] = vertices_L2[i];
		}
		TransformVertexArrayXY3D(NUM_CANONL2_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_CANONL2_VERTS, temp_vertices);
	}
	else if (m_level == 3)
	{
		Vertex_PCU temp_vertices[NUM_CANONL3_VERTS];
		for (int i = 0; i < NUM_CANONL3_VERTS; i++)
		{
			temp_vertices[i] = vertices_L3[i];
		}
		TransformVertexArrayXY3D(NUM_CANONL3_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_CANONL3_VERTS, temp_vertices);
	}
	else if (m_level == 4)
	{

	}
}

void Canon::Die()
{
}

void Canon::InitializedVertsL1(Vertex_PCU* vertsToFillIn)
{
	vertsToFillIn[0] = Vertex_PCU(Vec3(0, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(1, 0, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(0, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
}

void Canon::InitializedVertsL2(Vertex_PCU* vertsToFillIn)
{
	//a
	vertsToFillIn[0] = Vertex_PCU(Vec3(-2, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(0, 2, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(2, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	//e
	vertsToFillIn[3] = Vertex_PCU(Vec3(-2, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(2, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(0, -2, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
}

void Canon::InitializedVertsL3(Vertex_PCU* vertsToFillIn)
{
	//d
	vertsToFillIn[0] = Vertex_PCU(Vec3(0, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(1, 0, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(0, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	//e
	vertsToFillIn[3] = Vertex_PCU(Vec3(-2, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(2, -1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(0, -2, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	//a
	vertsToFillIn[6] = Vertex_PCU(Vec3(-2, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[7] = Vertex_PCU(Vec3(0, 2, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
	vertsToFillIn[8] = Vertex_PCU(Vec3(2, 1, 0.f), Rgba8(239, 141, 67), Vec2(0.f, 0.f));
}
