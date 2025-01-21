#include "Game/ShootWasp.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"


ShootWasp::ShootWasp(Game* game, float x, float y) : Entity(game, x, y)
{
	m_orientationDegrees = 0;
	m_physicsRadius = SHOOTWASP_PHYSICS_RADIUS;
	m_cosmeticRadius = SHOOTWASP_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_health = 3;
	m_oriHealth = 3;
	m_shootWaspCurColor = m_shootWaspOriColor;
	InitializedVerts(vertices, m_shootWaspCurColor);
}

void ShootWasp::Update(float deltaTime)
{
	if (m_game->m_playerShip->IsAlive())
	{
		Vec2 direction = m_game->m_playerShip->m_position - m_position;
		m_orientationDegrees = direction.GetOrientationDegrees();
	}
	m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, SHOOTWASP_SPEED);

	m_position.x += m_velocity.x * deltaTime;
	m_position.y += m_velocity.y * deltaTime;

	if (m_isAutoCoolDown == true)
	{
		float curTime = float(GetCurrentTimeSeconds());
		if (curTime - m_startCoolDownTime >= m_coolDownTime)
		{
			m_isAutoCoolDown = false;
		}
	}
	if (m_isAutoCoolDown == false)
	{
		//shoot
		m_startCoolDownTime = GetCurrentTimeSeconds();
		m_isAutoCoolDown = true;
		m_game->ShootWaspShootBullet(*this);
		
	}

	if (m_health <= 0)
	{
		Die();
	}
}

void ShootWasp::Render() const
{
	Vertex_PCU temp_vertices[NUM_SHOOTWASP_VERTS];
	for (int i = 0; i < NUM_SHOOTWASP_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
	}

	TransformVertexArrayXY3D(NUM_SHOOTWASP_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_SHOOTWASP_VERTS, temp_vertices);
}

void ShootWasp::Die()
{
	m_game->ShakeScreen();
	m_game->GenerateDebris(m_position, Rgba8(100, 160, 60, 130), 8, 15.f, 35.f, 1.5f, 2.f, m_velocity);
	m_isGarbage = true;
}

void ShootWasp::GetHurt(float hurtPoint)
{
	m_health -= hurtPoint;
	//m_isInvincibleState = true;
	//m_invincibleStartTime = GetCurrentTimeSeconds();

	g_theAudio->StartSound(m_game->enemyHurtSound, false);
}

void ShootWasp::InitializedVerts(Vertex_PCU* vertsToFillIn, Rgba8 const& color)
{
	vertsToFillIn[0] = Vertex_PCU(Vec3(-3.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(-2.f, 2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(-2.f, -2.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[3] = Vertex_PCU(Vec3(-2.f, 2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(-1.f, 2.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[6] = Vertex_PCU(Vec3(-2.f, -2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[7] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[8] = Vertex_PCU(Vec3(-1.f, -2.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[9] = Vertex_PCU(Vec3(-2.f, 2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[10] = Vertex_PCU(Vec3(2.f, 2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[11] = Vertex_PCU(Vec3(0.f,3.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[12] = Vertex_PCU(Vec3(-2.f, -2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[13] = Vertex_PCU(Vec3(2.f, -2.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[14] = Vertex_PCU(Vec3(0, -3.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[15] = Vertex_PCU(Vec3(-1.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[16] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[17] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[18] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[19] = Vertex_PCU(Vec3(0.f, 1.f, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[20] = Vertex_PCU(Vec3(0.f, -1.f, 0.f), color, Vec2(0.f, 0.f));
}