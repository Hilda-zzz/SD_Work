#include "Asteroid.hpp"
#include <Engine/Math/RandomNumberGenerator.hpp>
#include "Game.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include <Engine/Math/MathUtils.hpp>
#include <Engine/Core/Time.hpp>
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
Asteroid::Asteroid(Game* game, float x, float y) : Entity(game, x, y)
{
	m_asteroidCurColor = m_asteroidOriColor;
	m_orientationDegrees = game->m_rng->RollRandomFloatInRange(0.f,360.f);
	m_angularVelocity = game->m_rng->RollRandomFloatInRange(-200.f, 200.f);
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_health =30;
	float velocity_angle= game->m_rng->RollRandomFloatInRange(0.f, 360.f);
	m_velocity = Vec2::MakeFromPolarDegrees(velocity_angle, ASTEROID_SPEED);

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
		vertices[i] = Vertex_PCU(Vec3(0, 0, 0.f), m_asteroidCurColor, Vec2(0.f, 0.f));
		vertices[i+1] = Vertex_PCU(Vec3(curPosFirst.x, curPosFirst.y, 0.f), m_asteroidCurColor, Vec2(0.f, 0.f));
		vertices[i + 2] = Vertex_PCU(Vec3(curPosSecond.x, curPosSecond.y, 0.f), m_asteroidCurColor, Vec2(0.f, 0.f));
		i += 3;
		j+=1;
	}
}

Asteroid::~Asteroid()
{

}

void Asteroid::Update(float deltaTime)
{
	m_orientationDegrees += m_angularVelocity * deltaTime;
	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;

	if (m_isInvincibleState)
	{
		double curTime = GetCurrentTimeSeconds();
		if (curTime - m_invincibleStartTime > 0.8f)
		{
			m_isInvincibleState = false;
			if (m_health < 30.f && m_health >= 20.f)
			{
				m_asteroidCurColor = Rgba8(119,81,103,255);
			}
			else if (m_health < 20.f && m_health >= 10.f)
			{
				m_asteroidCurColor = Rgba8(110, 58, 77, 255);
			}
			else if (m_health < 10.f)
			{
				m_asteroidCurColor = Rgba8(99, 43, 52, 255);
			}
			return;
		}
		float temp = RangeMap(SinDegrees((float)curTime * 500.f), -1.f, 1.f, 0.4f, 1.f);
		m_asteroidCurColor = Rgba8(200, 200, 200, static_cast<unsigned char>(255.f * temp));
	}

	if (IsOffscreen())
	{
		WrapAround();
	}
	if (m_health <= 0)
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
		temp_vertices[i].m_color = m_asteroidCurColor;
	}

	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTS, temp_vertices); //NUM_SHIP_VERTS

	/*g_theRenderer->BeginCamera(*m_game->m_screenCamera);
	float asteroidMapPosX = RangeMapClamped(m_position.x, -200.f, 400.f, 1350.f, 1550.f);
	float asteroidMapPosY = RangeMapClamped(m_position.y, -100.f, 200.f, 660.f, 760.f);
	DebugDrawCircle(1.5f, Vec2(asteroidMapPosX, asteroidMapPosY), Rgba8(116, 116, 116, 255));
	g_theRenderer->BeginCamera(*m_game->m_worldCamera);*/
}

void Asteroid::Die()
{
	m_game->GenerateDebris(m_position, Rgba8(99, 43, 52, 130), 
		8, 10.f, 25.f, 1.5f, 2.0f, m_velocity);
	m_isGarbage = true;

	g_theAudio->StartSound(m_game->asteroidExpSound, false, 5.f);
}

void Asteroid::GetHurt(float hurtPoint)
{
	m_health -= hurtPoint;
	m_isInvincibleState = true;
	m_invincibleStartTime = GetCurrentTimeSeconds();

	g_theAudio->StartSound(m_game->asteroidHurtSound,false);
}

void Asteroid::WrapAround()
{
	if (m_position.x < -m_cosmeticRadius+WORLDSPACE_SIZE_BL_X)
	{
		m_position.x = WORLDSPACE_SIZE_TR_X + m_cosmeticRadius;
	}
	else if (m_position.x > WORLDSPACE_SIZE_TR_X + m_cosmeticRadius)
	{
		m_position.x = -m_cosmeticRadius + WORLDSPACE_SIZE_BL_X;
	}
	else if (m_position.y < -m_cosmeticRadius+WORLDSPACE_SIZE_BL_Y)
	{
		m_position.y = WORLDSPACE_SIZE_TR_Y + m_cosmeticRadius;
	}
	else if (m_position.y > WORLDSPACE_SIZE_TR_Y + m_cosmeticRadius)
	{
		m_position.y = -m_cosmeticRadius+WORLDSPACE_SIZE_BL_Y;
	}
}

