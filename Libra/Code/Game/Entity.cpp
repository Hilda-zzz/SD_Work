#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
Entity::Entity(float x, float y,Map* map):m_position{x,y}, m_map(map),m_type(ENTITY_TYPE_UNSURE),m_faction(FACTION_UNSURE)
{
	m_pathPoints.reserve(50);
}

Entity::Entity(EntityType type,EntityFaction faction): m_position{0.f,0.f },m_orientationDegrees(0.f),m_type(type),m_faction(faction)
{
}

Entity::~Entity()
{
	if (m_heatMapToPlayer != nullptr)
	{
		delete m_heatMapToPlayer;
	}
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1);
}

bool Entity::IsDead() const
{
	return m_isDead;
}

bool Entity::IsGarbage() const
{
	return m_isGarbage;
}

void Entity::GetHurt(float hurtPoint)
{
	m_health -= hurtPoint;
	if (m_health <= 0)
	{
		Die();
	}
}

void Entity::RenderHeatMap() const
{
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_heatmapVerts);
}

void Entity::PathFindingToPlayer(bool isTargetVisible,Entity*player,int& pathState,Vec2& lastTargetPos,Vec2& nextWayPointPosition)
{
	IntVec2 curCoords = m_map->GetTileCoordsFromPoint(m_position);
	IntVec2 curPlayerCoords = m_map->GetTileCoordsFromPoint(player->m_position);
	
	if (pathState == 0) //wandering
	{
		if (m_heatMapToPlayer->m_values[m_map->GetTileIndex(curCoords.x, curCoords.y)] == 0.f)
		{
			int randomAimPointX;
			int randomAimPointY;
			while (true)
			{
				randomAimPointX = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.x - 1);
				randomAimPointY = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.y - 1);
				if (m_map->GetTileType(randomAimPointX, randomAimPointY) == m_map->m_mapDef.m_fillTileType)
				{
					break;
				}
			}
			lastTargetPos = Vec2((float)randomAimPointX + 0.5f, (float)randomAimPointY + 0.5f);
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, IntVec2(randomAimPointX, randomAimPointY), 999999.f, !m_canSwim, true);
			nextWayPointPosition = PickNextWaypointPosition();
		}
		if (IsPointInsideDisc2D(nextWayPointPosition, m_position, m_physicsRadius))
		{
			nextWayPointPosition = PickNextWaypointPosition();
		}
		//sight & find target
		if (isTargetVisible && !player->m_isDead)
		{
			pathState = 1;
			g_theEventSystem->FireEvent("FindOutEvent");
		}
	}
	else if (pathState == 1) //go to player
	{
		if (curCoords - curPlayerCoords == IntVec2(1, 0) ||
			curCoords - curPlayerCoords == IntVec2(-1, 0) ||
			curCoords - curPlayerCoords == IntVec2(0, 1) ||
			curCoords - curPlayerCoords == IntVec2(0, -1))
		{
			//near player
			lastTargetPos = player->m_position;
			nextWayPointPosition = player->m_position;
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, curPlayerCoords, 999999.f, !m_canSwim, true);
		}
		else
		{
			lastTargetPos = player->m_position;
			IntVec2 lastCoords = m_map->GetTileCoordsFromPoint(lastTargetPos);
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, lastCoords, 999999.f, !m_canSwim, true);

			if (IsPointInsideDisc2D(nextWayPointPosition, m_position, m_physicsRadius))
			{
				nextWayPointPosition = PickNextWaypointPosition();
			}
		}
		//sight & find target
		if (!isTargetVisible || player->m_isDead)
		{
			pathState = 2;
		}
	}
	else if (pathState == 2) //go to last seen
	{
		if (m_heatMapToPlayer->m_values[m_map->GetTileIndex(curCoords.x, curCoords.y)] == 0.f)
		{
			pathState = 0;
		}
		if (IsPointInsideDisc2D(nextWayPointPosition, m_position, m_physicsRadius))
		{
			nextWayPointPosition = PickNextWaypointPosition();
		}
		//sight & find target
		if (isTargetVisible && !player->m_isDead)
		{
			pathState = 1;
			g_theEventSystem->FireEvent("FindOutEvent");
		}
	}
}

void Entity::ImprovedPathFindingToPlayer(bool isTargetVisible, Entity* player, int& pathState, Vec2& lastTargetPos, Vec2& nextWayPointPosition)
{
	IntVec2 curCoords = m_map->GetTileCoordsFromPoint(m_position);
	IntVec2 curPlayerCoords = m_map->GetTileCoordsFromPoint(player->m_position);

	if (pathState == 0) //wandering
	{
		/*if (m_pathPoints.empty())
		{
			int randomAimPointX;
			int randomAimPointY;
			while (true)
			{
				randomAimPointX = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.x - 1);
				randomAimPointY = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.y - 1);
				if (m_map->GetTileType(randomAimPointX, randomAimPointY) == m_map->m_mapDef.m_fillTileType)
				{
					break;
				}
			}
			lastTargetPos = Vec2((float)randomAimPointX + 0.5f, (float)randomAimPointY + 0.5f);
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, IntVec2(randomAimPointX, randomAimPointY), 999999.f, !m_canSwim, true);
			m_map->GenerateEntityPathToGoal(*m_heatMapToPlayer, m_pathPoints, curCoords, IntVec2(randomAimPointX, randomAimPointY));
		}*/
		//sight & find target
		if (isTargetVisible && !player->m_isDead)
		{
			pathState = 1;
			g_theEventSystem->FireEvent("FindOutEvent");
		}
	}
	else if (pathState == 1) //go to player
	{
		if (curCoords - curPlayerCoords == IntVec2(1, 0) ||
			curCoords - curPlayerCoords == IntVec2(-1, 0) ||
			curCoords - curPlayerCoords == IntVec2(0, 1) ||
			curCoords - curPlayerCoords == IntVec2(0, -1))
		{
			//near player
			lastTargetPos = player->m_position;
			//nextWayPointPosition = player->m_position;
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, curPlayerCoords, 999999.f, !m_canSwim, true);
			m_map->GenerateEntityPathToGoal(*m_heatMapToPlayer, m_pathPoints, curCoords, curPlayerCoords);
		}
		else
		{
			lastTargetPos = player->m_position;
			IntVec2 lastCoords = m_map->GetTileCoordsFromPoint(lastTargetPos);
			m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, lastCoords, 999999.f, !m_canSwim, true);
			m_map->GenerateEntityPathToGoal(*m_heatMapToPlayer, m_pathPoints, curCoords, lastCoords);
		}
		//sight & find target
		if (!isTargetVisible || player->m_isDead)
		{
			pathState = 0;
		}
	}
	//else if (pathState == 2) //go to last seen
	//{
	//	if (m_heatMapToPlayer->m_values[m_map->GetTileIndex(curCoords.x, curCoords.y)] == 0.f)
	//	{
	//		pathState = 0;
	//	}
	//	//sight & find target
	//	if (isTargetVisible && !player->m_isDead)
	//	{
	//		pathState = 1;
	//		g_theEventSystem->FireEvent("FindOutEvent");
	//	}
	//}
	
	while (m_pathPoints.size() >= 2)
	{
		if (IsPassToNextPos())
		{
			m_pathPoints.pop_back();
		}
		else
		{
			break;
		}
	}
	if (!m_pathPoints.empty()&&IsPointInsideDisc2D(m_pathPoints.back(), m_position, m_physicsRadius))
	{
		m_pathPoints.pop_back();
	}
	if (m_pathPoints.empty())
	{
		int randomAimPointX;
		int randomAimPointY;
		while (true)
		{
			randomAimPointX = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.x - 1);
			randomAimPointY = g_theGame->m_rng.RollRandomIntInRange(0, m_map->m_dimensions.y - 1);
			if (m_map->GetTileType(randomAimPointX, randomAimPointY) == m_map->m_mapDef.m_fillTileType)
			{
				break;
			}
		}
		lastTargetPos = Vec2((float)randomAimPointX + 0.5f, (float)randomAimPointY + 0.5f);
		m_maxSteps = m_map->PopulateDistanceField(*m_heatMapToPlayer, IntVec2(randomAimPointX, randomAimPointY), 999999.f, !m_canSwim, true);
		m_map->GenerateEntityPathToGoal(*m_heatMapToPlayer, m_pathPoints, curCoords, IntVec2(randomAimPointX, randomAimPointY));
	}

	nextWayPointPosition = m_pathPoints.back();
}

bool Entity::IsPassToNextPos()
{
	Vec2 direction = (m_pathPoints[m_pathPoints.size() - 2] - m_position).GetNormalized();
	Ray edgeLeftRay = Ray(direction, m_position+(m_physicsRadius-0.01f)*direction.GetRotated90Degrees(), 1000.f);
	Ray edgeRightRay = Ray(direction, m_position +( m_physicsRadius-0.01f) * direction.GetRotatedMinus90Degrees(), 1000.f);
	//RaycastResult2D resultLeftVsSolidTiles = m_map->FastRaycastVsTiles(edgeLeftRay);
	//RaycastResult2D resultRightVsSolidTiles = m_map->FastRaycastVsTiles(edgeLeftRay);
	m_lRayResult = m_map->FastRaycastVsTiles(edgeLeftRay,!m_canSwim,true);
	m_rRayResult = m_map->FastRaycastVsTiles(edgeRightRay, !m_canSwim, true);
	return IsEdgeRayPassThrough(m_lRayResult)&& IsEdgeRayPassThrough(m_rRayResult);
}

bool Entity::IsEdgeRayPassThrough(RaycastResult2D& resultVsSolidTiles)
{
	if (!resultVsSolidTiles.m_didImpact)
	{
		return true;
	}
	else
	{
		if (resultVsSolidTiles.m_impactDist > (m_pathPoints[m_pathPoints.size() - 2] - m_position).GetLength())
		{
			return true;
		}
		else return false;
	}
}

Vec2 Entity::PickNextWaypointPosition()
{
	IntVec2 curCoords = m_map->GetTileCoordsFromPoint(m_position);
	//N
	IntVec2 northTileCoords = curCoords + IntVec2(0, 1);
	float northHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(northTileCoords.x, northTileCoords.y)];
	//S
	IntVec2 southTileCoords = curCoords + IntVec2(0, -1);
	float southHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(southTileCoords.x, southTileCoords.y)];
	//E
	IntVec2 eastTileCoords = curCoords + IntVec2(1, 0);
	float eastHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(eastTileCoords.x, eastTileCoords.y)];
	//W
	IntVec2 westTileCoords = curCoords + IntVec2(-1, 0);
	float westHeat = m_heatMapToPlayer->m_values[m_map->GetTileIndex(westTileCoords.x, westTileCoords.y)];

	float minHeat = std::min({ northHeat, southHeat, eastHeat, westHeat });

	if (minHeat == northHeat)
	{
		return Vec2((float)northTileCoords.x + 0.5f, (float)northTileCoords.y + 0.5f);
	}
	else if (minHeat == southHeat)
	{
		return Vec2((float)southTileCoords.x + 0.5f, (float)southTileCoords.y + 0.5f);
	}
	else if (minHeat == eastHeat)
	{
		return Vec2((float)eastTileCoords.x + 0.5f, (float)eastTileCoords.y + 0.5f);
	}
	else
	{
		return Vec2((float)westTileCoords.x + 0.5f, (float)westTileCoords.y + 0.5f);
	}
}



void Entity::RenderUI()
{
	//hp bar
	AABB2 bkgBox = AABB2(m_position + Vec2(-0.4f, 0.5f), m_position + Vec2(0.4f, 0.6f));
	DebugDrawBox(bkgBox, Rgba8::RED);
	
	float ratio = m_health / m_totalHealth;
	float len = ratio * 0.8f;
	AABB2 bloodBox = AABB2(m_position + Vec2(-0.4f, 0.5f), m_position + Vec2(-0.4f+len, 0.6f));
	DebugDrawBox(bloodBox, Rgba8::GREEN);
}

bool Entity::CheckOverlapVsRubble()
{
	for (Entity* rubble : m_map->m_entityListsByType[ENTITY_TYPE_NEU_RUBBLE])
	{
		if (DoDiscsOverlap(m_position, m_physicsRadius, rubble->m_position, rubble->m_physicsRadius))
		{
			return true;
		}
	}
	return false;
}

void Entity::DrawDebugMode() const
{
	DebugDrawRing(0.02f, m_cosmeticRadius, Rgba8::MAGNETA, m_position);
	DebugDrawRing(0.02f, m_physicsRadius, Rgba8::CYAN, m_position);

	Vec2 end_ast =m_position + GetForwardNormal() *m_cosmeticRadius;
	Vec2 endL_ast = m_position + GetForwardNormal().GetRotated90Degrees() *m_cosmeticRadius;
	Vec2 endV_ast = m_position + m_velocity;
	DebugDrawLine(m_position, end_ast, 0.05f, Rgba8::RED);
	DebugDrawLine(m_position, endL_ast, 0.05f, Rgba8::GREEN);
	DebugDrawLine(m_position, endV_ast, 0.025f, Rgba8::YELLOW);
}
