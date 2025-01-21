#include "Game/Scorpio.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"
Scorpio::Scorpio(EntityType type,EntityFaction faction):Entity(type,faction)
{
	m_cosmeticRadius = SCORPIO_COS_RADIUS;
	m_physicsRadius = SCORPIO_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_angularVelocity =0.f;
	m_speed = 0.f;
	m_turretAngularVelocity = g_gameConfigBlackboard.GetValue("scorpioTurnRate",30.f);
	//m_turret_orientation = m_orientationDegrees;
	m_health = 5.f;
	m_totalHealth = m_health;
	Vec2 turret_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	Vec2 body_direction = Vec2::MakeFromPolarDegrees(0.f);
	m_verts_body.reserve(6);
	m_verts_turret.reserve(6);
	m_verts_ray.reserve(6);
	m_turretBox = OBB2(Vec2(0.f, 0.f), turret_direction, Vec2(0.6f, 0.6f));
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255));
	AddVertsForOBB2D(m_verts_turret, m_turretBox, Rgba8(255, 255, 255, 255));

	m_texScorpioBase = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	m_texScorpioTurret = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");


	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = false;
	m_isHitByBullets = true;

	m_ray = Ray(turret_direction.GetNormalized(), m_position,10.f);

}

Scorpio::~Scorpio()
{
}

void Scorpio::Update(float deltaTime)
{
	Ray sightRay = Ray((m_map->m_player-> m_position - m_position).GetNormalized(), m_position, 10.f);
	m_isTargetVisible = m_map->HasLineOfSight(sightRay, m_map->m_player);
	if (m_isTargetVisible && !m_map->m_player->m_isDead)
	{
		Vec2 aimDirection = m_map->m_player->m_position - m_position;
		float curDeg = GetTurnedTowardDegrees(m_orientationDegrees, aimDirection.GetOrientationDegrees(), g_gameConfigBlackboard.GetValue("scorpioTurnRate", 30.f) * deltaTime);
		m_orientationDegrees = curDeg;
		//Vec2 toTargetDirection = m_map->m_player->m_position - m_position;
		float aimori = aimDirection.GetOrientationDegrees();
		if (abs(GetShortestAngularDispDegrees(aimori, m_orientationDegrees))<5.f)
		{
			m_isShooting = true;
		}
		else
		{
			m_isShooting = false;
		}
	}
	else
	{
		m_orientationDegrees += g_gameConfigBlackboard.GetValue("scorpioTurnRate", 30.f) * deltaTime;
		m_isShooting = false;
	}

	Vec2 tip = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees) * 0.5f;
	if (m_isShooting)
	{
		if (!m_isShootCoolDown)
		{
			m_isShootCoolDown = true;
			m_lastShootTime = GetCurrentTimeSeconds();
			m_map->AgentShooting(ENTITY_TYPE_EVIL_BULLET, m_faction, tip, m_orientationDegrees,0.4f);
			g_theAudio->StartSound(g_theGame->m_enemyShoot);
		}
	}
	if (GetCurrentTimeSeconds() - m_lastShootTime > m_gapShootTime)
	{
		m_isShootCoolDown = false;
	}

	//add verts for red ray
	m_ray.m_rayStartPos = m_position;
	m_ray.m_rayFwdNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees);

	m_raycastResult= m_map->FastRaycastVsTiles(m_ray,false,false);
	m_verts_ray.clear();
	if (m_raycastResult.m_didImpact)
	{
		AddVertsForLinSegment2D(m_verts_ray, tip, m_raycastResult.m_impactPos, 0.05f, Rgba8::RED);
	}
	else
	{
		AddVertsForLinSegment2D(m_verts_ray, tip, m_position+m_ray.m_rayFwdNormal*m_ray.m_rayMaxLength, 0.05f, Rgba8::RED);
	}
}

void Scorpio::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, 0.f, m_position);
	g_theRenderer->BindTexture(m_texScorpioBase);
	g_theRenderer->DrawVertexArray(temp_verts);

	std::vector<Vertex_PCU> temp_verts_turret;
	for (int i = 0; i < static_cast<int>(m_verts_turret.size()); i++)
	{
		temp_verts_turret.push_back(m_verts_turret[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_turret.size()), temp_verts_turret, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texScorpioTurret);
	g_theRenderer->DrawVertexArray(temp_verts_turret);

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_verts_ray);
}

void Scorpio::Die()
{
	g_theAudio->StartSound(g_theGame->m_enemyDead);
	m_isDead = true;
	m_isGarbage = true;
	m_map->SpawnExplosion(m_position, 0.7f, g_explosionAnimDef_Slow);
}

void Scorpio::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
	if (m_isTargetVisible)
	{
		DebugDrawLine(m_position, m_map->m_player->m_position, 0.05f, Rgba8(0,0, 0, 200));
		DebugDrawCircle(0.1f, m_map->m_player->m_position, Rgba8(0, 0, 0, 200));
	}
}
