#include "Game/Turret.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Turret::Turret(Game* game, float x, float y) : Entity(game, x, y)
{
	m_orientationDegrees = 0;
	m_physicsRadius = TURRET_PHYSICS_RADIUS;
	m_cosmeticRadius = TURRET_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_health = 30;
	m_oriHealth = 30;
	m_oriPosition = Vec2(x, y);

	Rgba8 darkRed = Rgba8(117, 51, 62);
	Rgba8 lightRed = Rgba8(142, 62, 75);
	vertices_base[0] = Vertex_PCU(Vec3(-5.f, -1.5f, 0.f), lightRed, Vec2(0.f, 0.f));
	vertices_base[1] = Vertex_PCU(Vec3(5.f, -1.5f, 0.f), lightRed, Vec2(0.f, 0.f));
	vertices_base[2] = Vertex_PCU(Vec3(5.f, 1.5f, 0.f), lightRed, Vec2(0.f, 0.f)); 
	
	vertices_base[3] = Vertex_PCU(Vec3(-5.f, -1.5f, 0.f), lightRed, Vec2(0.f, 0.f));
	vertices_base[4] = Vertex_PCU(Vec3(5.f, 1.5f, 0.f), lightRed, Vec2(0.f, 0.f));
	vertices_base[5] = Vertex_PCU(Vec3(-5.f,1.5f, 0.f), lightRed, Vec2(0.f, 0.f));

	vertices_base[6] = Vertex_PCU(Vec3(-5.5f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[7] = Vertex_PCU(Vec3(-5.f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[8] = Vertex_PCU(Vec3(-5.f, 1.8f, 0.f), darkRed, Vec2(0.f, 0.f));

	vertices_base[9] = Vertex_PCU(Vec3(-5.5f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[10] = Vertex_PCU(Vec3(-5.f, 1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[11] = Vertex_PCU(Vec3(-5.5f, 1.8f, 0.f), darkRed, Vec2(0.f, 0.f)); 

	vertices_base[12] = Vertex_PCU(Vec3(5.f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[13] = Vertex_PCU(Vec3(5.5f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[14] = Vertex_PCU(Vec3(5.5f,1.8f, 0.f), darkRed, Vec2(0.f, 0.f));

	vertices_base[15] = Vertex_PCU(Vec3(5.f, -1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[16] = Vertex_PCU(Vec3(5.5f, 1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
	vertices_base[17] = Vertex_PCU(Vec3(5.f, 1.8f, 0.f), darkRed, Vec2(0.f, 0.f));
}

void Turret::Update(float deltaTime)
{
	m_position.y = m_oriPosition.y+ CosDegrees((float)GetCurrentTimeSeconds() * 300.f) * TURRET_FLOAT_SPEED * deltaTime;
	
	if (m_health <= 0)
	{
		Die();
	}

	if (m_isStaticState == true)
	{
		float curTime = (float)GetCurrentTimeSeconds();
		if (curTime - m_startRotateTime >= m_staticTime)
		{
			m_isStaticState = false;
			m_game->TurretShootBullet(*this);
		}
	}
	if (m_isStaticState == false)
	{
		m_isRotate = true;
		m_startRotateTime = GetCurrentTimeSeconds();
		m_isStaticState = true;
	}
	if (GetCurrentTimeSeconds() - m_startRotateTime < 1.f)
	{
		m_orientationDegrees += (45.f * deltaTime) / 1.f;
	}

}

void Turret::Render() const
{
	
	Vertex_PCU temp_vertices[NUM_TURRET_VERTS];
	Vertex_PCU temp_vertices2[NUM_TURRET_VERTS];
	for (int i = 0; i < NUM_TURRET_VERTS; i++)
	{
		temp_vertices[i] =vertices_base[i];
		temp_vertices2[i] = vertices_base[i];
		//temp_vertices[i].m_color = m_asteroidCurColor;
	}
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);

	TransformVertexArrayXY3D(NUM_TURRET_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_TURRET_VERTS, temp_vertices);

	TransformVertexArrayXY3D(NUM_TURRET_VERTS, temp_vertices2, 1.f, m_orientationDegrees+ 90.f, m_position);
	g_theRenderer->DrawVertexArray(NUM_TURRET_VERTS, temp_vertices2);

	DebugDrawHighCircle(TURRET_PHYSICS_RADIUS, m_position, Rgba8(117, 51, 62));
	DebugDrawHighCircle(TURRET_PHYSICS_RADIUS - 2.f, m_position, Rgba8(222, 130,145));


}


void Turret::Die()
{
	m_game->ShakeScreen();
	m_game->GenerateDebris(m_position, Rgba8(99, 43, 52, 130),
		15, 15.f, 35.f, 1.5f, 2.0f, m_velocity);
	m_isGarbage = true;
}

void Turret::GetHurt(float hurtPoint)
{
	m_health -= hurtPoint;
	g_theAudio->StartSound(m_game->enemyHurtSound);
}
