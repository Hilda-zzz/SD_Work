#include "Debris.hpp"
#include <Engine/Math/RandomNumberGenerator.hpp>
#include "Game.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/Time.hpp>
extern Renderer* g_theRenderer;

Debris::Debris(Game* game, float x, float y,float minSpeed,float maxSpeed, 
	float minRadius, float maxRadius, Rgba8 color, Vec2 const& oriVelocity,int type) : Entity(game, x, y)
{
	m_orientationDegrees = game->m_rng->RollRandomFloatInRange(0.f, 360.f);
	m_angularVelocity = game->m_rng->RollRandomFloatInRange(-200.f, 200.f);
	m_physicsRadius = minRadius;
	m_cosmeticRadius = maxRadius;
	m_health = 0;
	m_type = type;
	float velocity_angle = game->m_rng->RollRandomFloatInRange(0.f, 360.f);
	float speed= game->m_rng->RollRandomFloatInRange(minSpeed, maxSpeed);
	m_velocity = Vec2::MakeFromPolarDegrees(velocity_angle, speed)+oriVelocity;

	int j = 0;
	for (int i = 0; i < NUM_DEBRIS_VERTS / 3; i++)
	{
		m_point_len[i] = game->m_rng->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	}
	m_point_len[(NUM_DEBRIS_VERTS / 3)] = m_point_len[0];
	for (int i = 0; i < NUM_DEBRIS_VERTS; )
	{
		Vec2 curPosFirst = Vec2::MakeFromPolarDegrees(22.5f * j, m_point_len[j]);
		Vec2 curPosSecond = Vec2::MakeFromPolarDegrees(22.5f * (j + 1), m_point_len[j + 1]);
		vertices[i] = Vertex_PCU(Vec3(0, 0, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), color, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), color, Vec2(0.f, 0.f));
		i += 3;
		j += 1;
	}
	m_startTime = GetCurrentTimeSeconds();
}

void Debris::Update(float deltaTime)
{
	m_orientationDegrees += m_angularVelocity * deltaTime;
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;
	if (m_type == 1)
	{
		m_currentTime = GetCurrentTimeSeconds();
		m_current_a = RangeMapClamped((float)m_currentTime, (float)m_startTime, (float)m_startTime + 2.f, 130.f, 0.f);
		if (IsOffscreen() || (float)m_currentTime - (float)m_startTime > 2.f)
		{
			Die();
		}
	}
	else if (m_type == 2)
	{
		m_current_a = 255;
		if (IsOffscreen())
		{
			Die();
		}
	}
	
}

void Debris::Render() const
{
	Vertex_PCU temp_vertices[NUM_DEBRIS_VERTS];
	for (int i = 0; i < NUM_DEBRIS_VERTS; i++)
	{
		temp_vertices[i].m_position = vertices[i].m_position;
		temp_vertices[i].m_uvTexCoords = vertices[i].m_uvTexCoords;
		unsigned char uc_a = static_cast<unsigned char>(m_current_a);
		Rgba8 cur_color{ vertices[i].m_color.r,vertices[i].m_color.g,vertices[i].m_color.b,uc_a };
		temp_vertices[i].m_color = cur_color;
	}

	TransformVertexArrayXY3D(NUM_DEBRIS_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_DEBRIS_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void Debris::Die()
{
	m_isGarbage = true;
}