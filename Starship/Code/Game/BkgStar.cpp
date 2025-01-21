#include "Game/BkgStar.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"


Vertex_PCU crossStarVerts[18] = {
	Vertex_PCU(Vec3(0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(-0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),

	Vertex_PCU(Vec3(-0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(-0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	//t
	Vertex_PCU(Vec3(0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(-0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.f, 0.6f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	//r
	Vertex_PCU(Vec3(0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.6f, 0.f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	//l
	Vertex_PCU(Vec3(-0.1f, 0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(-0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(-0.6f, 0.f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	//b
	Vertex_PCU(Vec3(-0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.1f, -0.1f, 0.f), Rgba8{ 255,255,255,200 }, Vec2(0.f, 0.f)),
	Vertex_PCU(Vec3(0.f, -1.2f, 0.f), Rgba8{ 255,255,255,100 }, Vec2(0.f, 0.f))
};

BkgStar::BkgStar(Game* game, Vec2 pos,int layer):m_game(game),m_position(pos),m_layer(layer),m_movingRatio(0.f)
{
	Rgba8 starColor{ 255,255,255,255 };
	if (m_layer == 1)
	{
		m_movingRatio = 0.1f;
		starColor = Rgba8{ 255,255,255,80 };
	}
	else if (m_layer == 2)
	{
		m_movingRatio = 0.2f;
		starColor= Rgba8{ 255,255,255,125 };
	}
	else if (m_layer == 3)
	{
		m_movingRatio = 0.3f;
		starColor = Rgba8{ 255,255,255,160 };
	}
	else if (m_layer == 4)
	{
		m_movingRatio = 0.3f;
	}
	int j = 0;
	for (int i = 0; i < 48; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(22.5f * j, 0.2f) + m_position;
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(22.5f * (j + 1), 0.2f) + m_position;
		m_vertices[i] = Vertex_PCU(Vec3(m_position.x, m_position.y, 0.f), starColor, Vec2(0.f, 0.f));
		m_vertices[i + 1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), starColor, Vec2(0.f, 0.f));
		m_vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), starColor, Vec2(0.f, 0.f));
		i += 3;
		j += 1;
	}
}

BkgStar::~BkgStar()
{
}

void BkgStar::Update(float deltaTime)
{
	m_position.x += -m_game->m_playerShip->m_velocity.x * deltaTime * m_movingRatio;
	m_position.y += -m_game->m_playerShip->m_velocity.y * deltaTime * m_movingRatio;
}

void BkgStar::Render() const
{
	if (m_layer != 4)
	{
		Vertex_PCU temp_vertices[48];
		for (int i = 0; i < 48; i++)
		{
			temp_vertices[i] = m_vertices[i];
		}
		TransformVertexArrayXY3D(48, temp_vertices, 1.f, 0.f, m_position);
		g_theRenderer->DrawVertexArray(48, temp_vertices);
	}
	else
	{
		Vertex_PCU temp_vertices[18];
		for (int i = 0; i < 18; i++)
		{
			temp_vertices[i] = crossStarVerts[i];
		}
		TransformVertexArrayXY3D(18, temp_vertices, 1.f, 0.f, m_position);
		g_theRenderer->DrawVertexArray(18, temp_vertices);
	}
}
