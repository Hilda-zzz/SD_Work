#include "Game/Aries.hpp"
#include "Engine/Math/Ray.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Image.hpp"
Aries::Aries(EntityType type, EntityFaction faction):Entity(type,faction)
{
	m_cosmeticRadius = ARIES_COS_RADIUS;
	m_physicsRadius = ARIES_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("ariesTurnRate", 180.f);
	m_speed = g_gameConfigBlackboard.GetValue("ariesDriveSpeed", 0.5f);
	m_health = 5.f;
	m_totalHealth = m_health;
	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_verts_body.reserve(6);
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255));

	//Image ariesImg = Image("Data/Images/EnemyAries.png");
	m_texAriesBase = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyAries.png");
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	m_nextWayPointPosition = m_position;
}

Aries::~Aries()
{
}

void Aries::Update(float deltaTime)
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
	PathFindingToPlayer(m_isTargetVisible, m_map->m_player, m_pathState, m_lastTargetPos, m_nextWayPointPosition);
	//-------------------------------------------------------------------------------------------------------------------------
	Vec2 aimDirection = m_nextWayPointPosition - m_position;

	//movement & turn
	float curDeg = GetTurnedTowardDegrees(m_orientationDegrees, aimDirection.GetOrientationDegrees(), m_angularVelocity * deltaTime);
	m_orientationDegrees = curDeg;
	Vec2 curDirection = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_velocity = curDirection * m_speed*m_speedRatio;
	m_position += m_velocity * deltaTime;
}

void Aries::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texAriesBase);
	g_theRenderer->DrawVertexArray(temp_verts);
}

void Aries::Die()
{
	g_theAudio->StartSound(g_theGame->m_enemyDead);
	m_isDead = true;
	m_isGarbage = true;
	m_map->SpawnExplosion(m_position, 0.7f, g_explosionAnimDef_Slow);
}

void Aries::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
	DebugDrawLine(m_position, m_nextWayPointPosition, 0.05f, Rgba8(0, 255, 255, 200));
	DebugDrawCircle(0.1f, m_nextWayPointPosition, Rgba8(255, 255, 255, 200));
	DebugDrawLine(m_position, m_lastTargetPos, 0.05f, Rgba8(255, 0, 255, 200));
	DebugDrawCircle(0.1f, m_lastTargetPos, Rgba8(255, 0, 255, 255));
}
