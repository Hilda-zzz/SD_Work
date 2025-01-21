#include "Game/Leo.hpp"
#include "Engine/Math/Ray.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
Leo::Leo(EntityType type, EntityFaction faction) :Entity(type, faction)
{
	m_cosmeticRadius = LEO_COS_RADIUS;
	m_physicsRadius = LEO_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("leoTurnRate", 150.f);
	m_speed = g_gameConfigBlackboard.GetValue("leoDriveSpeed", 0.8f);

	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_verts_body.reserve(6);
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255));
	m_texLeoBase = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");

	m_health = 5.f;
	m_totalHealth = m_health;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_nextWayPointPosition = m_position;
	
}

Leo::~Leo()
{
}

void Leo::Update(float deltaTime)
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
	//PathFindingToPlayer(m_isTargetVisible,m_map->m_player,m_pathState,m_lastTargetPos,m_nextWayPointPosition);
	ImprovedPathFindingToPlayer(m_isTargetVisible, m_map->m_player, m_pathState, m_lastTargetPos, m_nextWayPointPosition);
	//-------------------------------------------------------------------------------------------------------------------------
	Vec2 aimDirection = m_nextWayPointPosition - m_position;

	//shooting logic
	Vec2 toTargetDirection = m_map->m_player->m_position - m_position;
	if (m_isTargetVisible && !m_map->m_player->m_isDead)
	{
		if (abs(GetShortestAngularDispDegrees(toTargetDirection.GetOrientationDegrees(), m_orientationDegrees))< 5.f)
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
			m_map->AgentShooting(ENTITY_TYPE_EVIL_BULLET,m_faction, tip, m_orientationDegrees,0.3f);
			g_theAudio->StartSound(g_theGame->m_enemyShoot);
		}
	}
	if (GetCurrentTimeSeconds() - m_lastShootTime > m_gapShootTime)
	{
		m_isShootCoolDown = false;
	}
}

void Leo::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texLeoBase);
	g_theRenderer->DrawVertexArray(temp_verts);
}

void Leo::Die()
{
	g_theAudio->StartSound(g_theGame->m_enemyDead);
	m_isDead = true;
	m_isGarbage = true;
	m_map->SpawnExplosion(m_position, 0.7f, g_explosionAnimDef_Slow);
}

void Leo::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
	DebugDrawLine(m_position, m_nextWayPointPosition, 0.05f, Rgba8(0,255, 255, 200));
	DebugDrawCircle(0.1f, m_nextWayPointPosition, Rgba8(255, 255,255, 200));
	DebugDrawLine(m_position,m_lastTargetPos, 0.05f, Rgba8(255, 0,255, 200));
	DebugDrawCircle(0.1f, m_lastTargetPos, Rgba8(255, 0, 255, 255));
}

//Vec2 Leo::PickNextWaypointPosition()
//{
//	IntVec2 curCoords = m_map->GetTileCoordsFromPoint(m_position);
//	//N
//	IntVec2 northTileCoords = curCoords + IntVec2(0, 1);
//	float northHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(northTileCoords.x, northTileCoords.y)];
//	//S
//	IntVec2 southTileCoords = curCoords + IntVec2(0, -1);
//	float southHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(southTileCoords.x, southTileCoords.y)];
//	//E
//	IntVec2 eastTileCoords = curCoords + IntVec2(1, 0);
//	float eastHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(eastTileCoords.x, eastTileCoords.y)];
//	//W
//	IntVec2 westTileCoords = curCoords + IntVec2(-1, 0);
//	float westHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(westTileCoords.x, westTileCoords.y)];
//
//	float minHeat = std::min({ northHeat, southHeat, eastHeat, westHeat });
//
//	if (minHeat == northHeat) 
//	{
//		return Vec2((float)northTileCoords.x+0.5f, (float)northTileCoords.y + 0.5f);
//	}
//	else if (minHeat == southHeat) 
//	{
//		return Vec2((float)southTileCoords.x + 0.5f, (float)southTileCoords.y + 0.5f);
//	}
//	else if (minHeat == eastHeat) 
//	{
//		return Vec2((float)eastTileCoords.x + 0.5f, (float)eastTileCoords.y + 0.5f);
//	}
//	else 
//	{
//		return Vec2((float)westTileCoords.x + 0.5f, (float)westTileCoords.y + 0.5f);
//	}
//}

//void Leo::PathFindingToPlayer()
//{
//	IntVec2 curCoords = m_map->GetTileCoordsFromPoint(m_position);
//	IntVec2 curPlayerCoords = m_map->GetTileCoordsFromPoint(m_map->m_player->m_position);
//	//Ray sightRay = Ray((m_map->m_player->m_position - m_position).GetNormalized(), m_position, 10.f);
//	//m_isTargetVisible = m_map->HasLineOfSight(sightRay, m_map->m_player);
//	if (m_pathState == 0) //wandering
//	{
//		if (m_heatMapToPlayer->m_values[m_map->GetTileIndex(curCoords.x, curCoords.y)] == 0.f)
//		{
//			int randomAimPointX;
//			int randomAimPointY;
//			while (true)
//			{
//				randomAimPointX = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.x - 1);
//				randomAimPointY = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.y - 1);
//				if (m_map->GetTileType(randomAimPointX, randomAimPointY) == m_map->m_mapDef.m_fillTileType)
//				{
//					break;
//				}
//			}
//			m_lastTargetPos = Vec2((float)randomAimPointX + 0.5f, (float)randomAimPointY + 0.5f);
//			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, IntVec2(randomAimPointX, randomAimPointY), 999999.f, true, true);
//			m_nextWayPointPosition = PickNextWaypointPosition();
//		}
//		if (IsPointInsideDisc2D(m_nextWayPointPosition, m_position, m_physicsRadius))
//		{
//			m_nextWayPointPosition = PickNextWaypointPosition();
//		}
//		//sight & find target
//		if (m_isTargetVisible && !m_map->m_player->m_isDead)
//		{
//			m_pathState = 1;
//		}
//	}
//	else if (m_pathState == 1) //go to player
//	{
//		if (curCoords - curPlayerCoords == IntVec2(1, 0) ||
//			curCoords - curPlayerCoords == IntVec2(-1, 0) ||
//			curCoords - curPlayerCoords == IntVec2(0, 1) ||
//			curCoords - curPlayerCoords == IntVec2(0, -1))
//		{
//			//near player
//			m_lastTargetPos = m_map->m_player->m_position;
//			m_nextWayPointPosition = m_map->m_player->m_position;
//			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, curPlayerCoords, 999999.f, true, true);
//		}
//		else
//		{
//			m_lastTargetPos = m_map->m_player->m_position;
//			IntVec2 lastCoords = m_map->GetTileCoordsFromPoint(m_lastTargetPos);
//			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, lastCoords, 999999.f, true, true);
//
//			if (IsPointInsideDisc2D(m_nextWayPointPosition, m_position, m_physicsRadius))
//			{
//				m_nextWayPointPosition = PickNextWaypointPosition();
//			}
//		}
//		//sight & find target
//		if (!m_isTargetVisible || m_map->m_player->m_isDead)
//		{
//			m_pathState = 2;
//		}
//	}
//	else if (m_pathState == 2) //go to last seen
//	{
//		if (m_heatMapToPlayer->m_values[m_map->GetTileIndex(curCoords.x, curCoords.y)] == 0.f)
//		{
//			m_pathState = 0;
//		}
//		if (IsPointInsideDisc2D(m_nextWayPointPosition, m_position, m_physicsRadius))
//		{
//			m_nextWayPointPosition = PickNextWaypointPosition();
//		}
//		//sight & find target
//		if (m_isTargetVisible && !m_map->m_player->m_isDead)
//		{
//			m_pathState = 1;
//		}
//	}
//}
