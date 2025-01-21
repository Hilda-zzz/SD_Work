#include "Game/Capricorn.hpp"
#include <Engine/Math/Ray.hpp>
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include <Engine/Core/Time.hpp>
#include "Game/Game.hpp"
Capricorn::Capricorn(EntityType type, EntityFaction faction):Entity(type, faction)
{
	m_cosmeticRadius = LEO_COS_RADIUS;
	m_physicsRadius = LEO_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("capricornTurnRate", 150.f);
	m_speed = g_gameConfigBlackboard.GetValue("capricornDriveSpeed", 0.8f);
	m_canSwim = true;

	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_verts_body.reserve(6);
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255));
	m_texCapricornBase = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank3.png");

	m_health = 5.f;
	m_totalHealth = m_health;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_nextWayPointPosition = m_position;
}

Capricorn::~Capricorn()
{
}

void Capricorn::Update(float deltaTime)
{
	if (CheckOverlapVsRubble())
	{
		m_speedRatio = 0.5f;
	}
	else
	{
		m_speedRatio = 1.f;
	}
	Ray sightRay = Ray((m_map->m_player->m_position - m_position).GetNormalized(), m_position, 10.f);
	m_isTargetVisible = m_map->HasLineOfSight(sightRay, m_map->m_player);
	//---------------path----------------------------------------------------------------------
	//PathFindingToPlayer(m_isTargetVisible, m_map->m_player, m_pathState, m_lastTargetPos, m_nextWayPointPosition);
	ImprovedPathFindingToPlayer(m_isTargetVisible, m_map->m_player, m_pathState, m_lastTargetPos, m_nextWayPointPosition);
	//-------------------------------------------------------------------------------------------------------------------------
	Vec2 aimDirection = m_nextWayPointPosition - m_position;

	//shooting logic
	Vec2 toTargetDirection = m_map->m_player->m_position - m_position;
	if (m_isTargetVisible && !m_map->m_player->m_isDead)
	{
		if (abs(GetShortestAngularDispDegrees(toTargetDirection.GetOrientationDegrees(), m_orientationDegrees)) < 5.f)
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
		m_isShooting = false;
	}

	//movement & turn
	float curDeg = GetTurnedTowardDegrees(m_orientationDegrees, aimDirection.GetOrientationDegrees(), m_angularVelocity * deltaTime);
	m_orientationDegrees = curDeg;
	Vec2 curDirection = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_velocity = curDirection * m_speed*m_speedRatio;
	m_position += m_velocity * deltaTime;

	//generate bullet logic
	if (m_isShooting)
	{
		if (!m_isShootCoolDown)
		{
			Vec2 tip = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees) * 0.5f;
			m_isShootCoolDown = true;
			m_lastShootTime = GetCurrentTimeSeconds();
			m_map->AgentShooting(ENTITY_TYPE_EVIL_GDMISSILE, m_faction, tip, m_orientationDegrees,0.5f);
			g_theAudio->StartSound(g_theGame->m_enemyShoot);
		}
	}
	if (GetCurrentTimeSeconds() - m_lastShootTime > m_gapShootTime)
	{
		m_isShootCoolDown = false;
	}
}

void Capricorn::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texCapricornBase);
	g_theRenderer->DrawVertexArray(temp_verts);
	g_theRenderer->BindTexture(nullptr);
	/*if (m_lRayResult.m_didImpact)
	{
		DebugDrawLine(m_lRayResult.m_rayStartPos, m_lRayResult.m_impactPos, 0.05f, Rgba8::HILDA);
		DebugDrawCircle(0.1f, m_lRayResult.m_impactPos, Rgba8::BLUE);
	}
	else
	{
		DebugDrawLine(m_lRayResult.m_rayStartPos, m_lRayResult.m_rayStartPos+m_lRayResult.m_rayFwdNormal* m_lRayResult.m_rayMaxLength, 0.1f, Rgba8::HILDA);
	}
	
	if (m_rRayResult.m_didImpact)
	{
		DebugDrawLine(m_rRayResult.m_rayStartPos, m_rRayResult.m_impactPos, 0.05f, Rgba8::HILDA);
		DebugDrawCircle(0.1f, m_rRayResult.m_impactPos, Rgba8::RED);
	}
	else
	{
		DebugDrawLine(m_rRayResult.m_rayStartPos, m_rRayResult.m_rayStartPos + m_rRayResult.m_rayFwdNormal * m_rRayResult.m_rayMaxLength, 0.1f, Rgba8::HILDA);
	}*/
}

void Capricorn::Die()
{
	g_theAudio->StartSound(g_theGame->m_enemyDead);
	m_isDead = true;
	m_isGarbage = true;
	m_map->SpawnExplosion(m_position, 0.7f, g_explosionAnimDef_Slow);
}

void Capricorn::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
	DebugDrawLine(m_position, m_nextWayPointPosition, 0.05f, Rgba8(0, 255, 255, 200));
	DebugDrawCircle(0.1f, m_nextWayPointPosition, Rgba8(255, 255, 255, 200));
	DebugDrawLine(m_position, m_lastTargetPos, 0.05f, Rgba8(255, 0, 255, 200));
	DebugDrawCircle(0.1f, m_lastTargetPos, Rgba8(255, 0, 255, 255));
}
