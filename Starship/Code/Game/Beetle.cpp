#include "Game/Beetle.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"

constexpr int ORI_BEETLE_HP = 30;
Beetle::Beetle(Game* game, float x, float y) : Entity(game,x,y)
{
	//can use initialize list
	m_orientationDegrees = 0;
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_health = ORI_BEETLE_HP;
	m_oriHealth= ORI_BEETLE_HP;
	vertices[0] = Vertex_PCU(Vec3(-1.5f, 1.7f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));
	vertices[1] = Vertex_PCU(Vec3(-1.5f, -1.7f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));
	vertices[2] = Vertex_PCU(Vec3(1.5f, 1.f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));

	vertices[3] = Vertex_PCU(Vec3(-1.5f, -1.7f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));
	vertices[4] = Vertex_PCU(Vec3(1.5f, -1.f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));
	vertices[5] = Vertex_PCU(Vec3(1.5f, 1.f, 0.f), Rgba8(100, 160, 60, 255), Vec2(0.f, 0.f));
}

void Beetle::Update(float deltaTime)
{
	if (m_game->m_playerShip->IsAlive())
	{
		Vec2 direction = m_game->m_playerShip->m_position - m_position;
		m_orientationDegrees = direction.GetOrientationDegrees();
	}
	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, BEETLE_SPEED);

	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;

	if (m_health <= 0)
	{
		Die();
	}
}

void Beetle::Render() const
{
	Vertex_PCU temp_vertices[NUM_BEETLE_VERTS];
	for (int i = 0; i < NUM_BEETLE_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_BEETLE_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_BEETLE_VERTS, temp_vertices); //NUM_SHIP_VERTS
}

void Beetle::Die()
{
	m_game->GenerateDebris(m_position, Rgba8(100, 160, 60, 130), 8, 15.f, 35.f, 1.5f, 2.f,m_velocity);
	m_isGarbage = true;
}

void Beetle::GetHurt(float hurtPoint)
{
	m_health -= hurtPoint;
	//m_isInvincibleState = true;
	//m_invincibleStartTime = GetCurrentTimeSeconds();

	g_theAudio->StartSound(m_game->enemyHurtSound, false);
}


