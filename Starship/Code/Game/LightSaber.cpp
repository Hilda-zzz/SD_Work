#include "LightSaber.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Game/Game.hpp"
#include <Engine/Renderer/Renderer.hpp>
extern Renderer* g_theRenderer;
LightSaber::LightSaber(Game* game, float x, float y,int level):Entity(game,x,y)
{
	m_level = level;
	//0 deg
	vertices_0_deg[0] = Vertex_PCU(Vec3(1.f, LIGHTSABER_WIDTH*0.5f, 0.f), Rgba8(242, 170, 132), Vec2(0.f, 0.f));
	vertices_0_deg[1] = Vertex_PCU(Vec3(1.f, -LIGHTSABER_WIDTH * 0.5f, 0.f), Rgba8(242, 170, 132), Vec2(0.f, 0.f));
	vertices_0_deg[2] = Vertex_PCU(Vec3(1.f+LIGHTSABER_LEN, LIGHTSABER_WIDTH * 0.5f, 0.f), Rgba8(242, 170, 132,100), Vec2(0.f, 0.f));
	vertices_0_deg[3] = Vertex_PCU(Vec3(1.f + LIGHTSABER_LEN, LIGHTSABER_WIDTH * 0.5f, 0.f), Rgba8(242, 170, 132, 100), Vec2(0.f, 0.f));
	vertices_0_deg[4] = Vertex_PCU(Vec3(1.f + LIGHTSABER_LEN, -LIGHTSABER_WIDTH * 0.5f, 0.f), Rgba8(242, 170, 132, 100), Vec2(0.f, 0.f));
	vertices_0_deg[5] = Vertex_PCU(Vec3(1.f, -LIGHTSABER_WIDTH * 0.5f, 0.f), Rgba8(242, 170, 132), Vec2(0.f, 0.f));
	for (int i = 0; i < 6; i++)
	{
		TransformPositionXY3D(vertices_0_deg[i].m_position, Vec2(CosDegrees(LIGHTSABER_L1_SHIFT), SinDegrees(LIGHTSABER_L1_SHIFT)),
			Vec2(CosDegrees(LIGHTSABER_L1_SHIFT+90.f), SinDegrees(LIGHTSABER_L1_SHIFT+90.f)), Vec2(0, 0));

		vertices_120_deg[i] = vertices_0_deg[i];
		TransformPositionXY3D(vertices_120_deg[i].m_position, Vec2(CosDegrees(LIGHTSABER_L1_SHIFT*2.f),SinDegrees(LIGHTSABER_L1_SHIFT * 2.f)),
			Vec2(CosDegrees(LIGHTSABER_L1_SHIFT * 2.f+90.f), SinDegrees(LIGHTSABER_L1_SHIFT * 2.f + 90.f)),Vec2(0, 0));

		vertices_240_deg[i] = vertices_0_deg[i];
		TransformPositionXY3D(vertices_240_deg[i].m_position, Vec2(CosDegrees(LIGHTSABER_L1_SHIFT * 4.f), SinDegrees(LIGHTSABER_L1_SHIFT * 4.f)),
			Vec2(CosDegrees(LIGHTSABER_L1_SHIFT * 4.f+90.f), SinDegrees(LIGHTSABER_L1_SHIFT * 4.f + 90.f)), Vec2(0, 0));

		vertices_90_deg[i] = vertices_0_deg[i];
		TransformPositionXY3D(vertices_90_deg[i].m_position, Vec2(CosDegrees(90.f), SinDegrees(90.f)),
			Vec2(CosDegrees(180.f), SinDegrees(180.f)), Vec2(0, 0));

		vertices_180_deg[i] = vertices_0_deg[i];
		TransformPositionXY3D(vertices_180_deg[i].m_position, Vec2(CosDegrees(180.f), SinDegrees(180.f)),
			Vec2(CosDegrees(270.f), SinDegrees(270.f)), Vec2(0, 0));

		vertices_270_deg[i] = vertices_0_deg[i];
		TransformPositionXY3D(vertices_180_deg[i].m_position, Vec2(CosDegrees(270.f), SinDegrees(270.f)),
			Vec2(CosDegrees(360.f), SinDegrees(360.f)), Vec2(0, 0));
	}
	//m_aabb_0 = AABB2(vertices_0_deg[1].m_position.x, vertices_0_deg[1].m_position.y, 
	//	vertices_0_deg[3].m_position.x, vertices_0_deg[3].m_position.y);
	//m_aabb_120 = AABB2(vertices_120_deg[1].m_position.x, vertices_120_deg[1].m_position.y,
	//	vertices_120_deg[3].m_position.x, vertices_120_deg[3].m_position.y);
	//m_aabb_240 = AABB2(vertices_240_deg[1].m_position.x, vertices_240_deg[1].m_position.y,
	//	vertices_240_deg[3].m_position.x, vertices_240_deg[3].m_position.y);
	//m_aabb_90 = AABB2(vertices_90_deg[1].m_position.x, vertices_90_deg[1].m_position.y,
	//	vertices_90_deg[3].m_position.x, vertices_90_deg[3].m_position.y);
	//m_aabb_180 = AABB2(vertices_180_deg[1].m_position.x, vertices_180_deg[1].m_position.y,
	//	vertices_180_deg[3].m_position.x, vertices_180_deg[3].m_position.y);
	//m_aabb_270 = AABB2(vertices_270_deg[1].m_position.x, vertices_270_deg[1].m_position.y,
	//	vertices_270_deg[3].m_position.x, vertices_270_deg[3].m_position.y);

}

void LightSaber::Update(float deltaTime)
{
	m_orientationDegrees = m_game->m_playerShip->m_orientationDegrees;
	m_position = m_game->m_playerShip->m_position;
}

void LightSaber::Render() const
{
	if (m_level == 1)
	{
		Vertex_PCU temp_vertices_0deg[6];
		Vertex_PCU temp_vertices_120deg[6];
		Vertex_PCU temp_vertices_240deg[6];
		for (int i = 0; i < 6; i++)
		{
			temp_vertices_0deg[i] = vertices_0_deg[i];
			temp_vertices_120deg[i]= vertices_120_deg[i];
			temp_vertices_240deg[i] = vertices_240_deg[i];
		}
		TransformVertexArrayXY3D(6, temp_vertices_0deg, 1.f, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(6, temp_vertices_120deg, 1.f, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(6, temp_vertices_240deg, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(6, temp_vertices_0deg);
		g_theRenderer->DrawVertexArray(6, temp_vertices_120deg);
		g_theRenderer->DrawVertexArray(6, temp_vertices_240deg);
	}
	else if (m_level == 2)
	{
		Vertex_PCU temp_vertices_0deg[6];
		Vertex_PCU temp_vertices_90deg[6];
		Vertex_PCU temp_vertices_180deg[6];
		Vertex_PCU temp_vertices_270deg[6];
		for (int i = 0; i < 6; i++)
		{
			temp_vertices_0deg[i] = vertices_0_deg[i];
			temp_vertices_90deg[i] = vertices_90_deg[i];
			temp_vertices_180deg[i] = vertices_180_deg[i];
			temp_vertices_270deg[i] = vertices_270_deg[i];
		}
		TransformVertexArrayXY3D(6, temp_vertices_0deg, 1.f, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(6, temp_vertices_90deg, 1.f, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(6, temp_vertices_180deg, 1.f, m_orientationDegrees, m_position);
		TransformVertexArrayXY3D(6, temp_vertices_270deg, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(6, temp_vertices_0deg);
		g_theRenderer->DrawVertexArray(6, temp_vertices_90deg);
		g_theRenderer->DrawVertexArray(6, temp_vertices_180deg);
		g_theRenderer->DrawVertexArray(6, temp_vertices_270deg);
	}
}

void LightSaber::Die()
{

}

