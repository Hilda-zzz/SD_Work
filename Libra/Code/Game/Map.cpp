#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Leo.hpp"
#include "Game/Aries.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Entity.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Explosion.hpp"
#include "Game/Rubble.hpp"
#include "Game/Tree.hpp"

float  g_lastFindOutSoundTime;
Map::Map(MapDefinition const& mapDef) :m_dimensions(mapDef.m_dimensions), m_startCoords(mapDef.m_startCoords), m_exitCoords(mapDef.m_exitCoords),m_mapDef(mapDef)
{
	g_lastFindOutSoundTime = 0.f;
	m_viewTileNumX = g_gameConfigBlackboard.GetValue("viewTileNumX", 16);
	m_viewTileNumY = g_gameConfigBlackboard.GetValue("viewTileNumY", 8);
	m_tileSizeX = g_gameConfigBlackboard.GetValue("tileSizeX", 1.f);
	m_tileSizeY = g_gameConfigBlackboard.GetValue("tileSizeY", 1.f);
	m_viewLen = Vec2(m_tileSizeX * m_viewTileNumX, m_tileSizeY * m_viewTileNumY);
	m_heatMapDistance = new TileHeatMap(m_dimensions);
	m_heatMapSolidForAmph = new TileHeatMap(m_dimensions);
	m_heatMapSolidForLand = new TileHeatMap(m_dimensions);
	if (m_mapDef.m_mapImageName != "")
	{
		InitializeTileMapFromImage(m_mapDef.m_mapImageName);
	}
	else
	{
		InitializeTileMapFromDef(mapDef);
	}
	InitializeAgentsFromDef(mapDef);

	m_heatmapVerts.reserve(m_dimensions.x * m_dimensions.y * 6);

	g_theEventSystem->SubscribeEventCallbackFuction("FindOutEvent", GenerateFindOutEffect);
}


void Map::InitializeTileMapFromDef(MapDefinition const& mapDef)
{
	m_tiles.reserve(m_dimensions.x * m_dimensions.y);
	int generateTimes = 0;
	while (generateTimes<100)
	{
		generateTimes++;
		for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
		{
			for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
			{
				std::string thisTileType;
				thisTileType = mapDef.m_fillTileType;
				//boundary
				if (columnIndex == 0 || rowIndex == 0 || columnIndex == m_dimensions.x - 1 || rowIndex == m_dimensions.y - 1)
				{
					thisTileType = mapDef.m_edgeTileType;
				}
				//L & square at bl corner
				if ((columnIndex > 0 && columnIndex < 6) && (rowIndex > 0 && rowIndex < 6))
				{
					thisTileType = mapDef.m_endFloorTileType;
				}

				if ((columnIndex == 2 && rowIndex == 4) ||
					(columnIndex == 3 && rowIndex == 4) ||
					(columnIndex == 4 && rowIndex == 4) ||
					(columnIndex == 4 && rowIndex == 3) ||
					(columnIndex == 4 && rowIndex == 2))
				{
					thisTileType = mapDef.m_startBunkerTileType;
				}
				//L & square at tr corner
				if ((columnIndex >= m_dimensions.x - 6 && columnIndex <= m_dimensions.x - 2) && (rowIndex >= m_dimensions.y - 6 && rowIndex <= m_dimensions.y - 2))
				{
					thisTileType = mapDef.m_endFloorTileType;
				}
				if ((columnIndex == m_dimensions.x - 5 && rowIndex == m_dimensions.y - 5) ||
					(columnIndex == m_dimensions.x - 4 && rowIndex == m_dimensions.y - 5) ||
					(columnIndex == m_dimensions.x - 3 && rowIndex == m_dimensions.y - 5) ||
					(columnIndex == m_dimensions.x - 5 && rowIndex == m_dimensions.y - 4) ||
					(columnIndex == m_dimensions.x - 5 && rowIndex == m_dimensions.y - 3))
				{
					thisTileType = mapDef.m_startBunkerTileType;
				}
				// start & end
				if (columnIndex == 1 && rowIndex == 1)
				{
					thisTileType = mapDef.m_startTileType;
				}
				else if (columnIndex == m_dimensions.x - 2 && rowIndex == m_dimensions.y - 2)
				{
					thisTileType = mapDef.m_endTileType;
				}
				GenerateEachTile(columnIndex, rowIndex, thisTileType);
			}
		}
		InitializeWorms(mapDef.m_worm1Count, mapDef.m_worm1MaxLen, mapDef.m_worm1TileType);
		InitializeWorms(m_mapDef.m_worm2Count, m_mapDef.m_worm2MaxLen, m_mapDef.m_worm2TileType);
		InitializeWorms(m_mapDef.m_wormWaterCount, m_mapDef.m_wormWaterMaxLen, m_mapDef.m_waterTileType);
		int maxSteps = PopulateDistanceField(*m_heatMapDistance, m_startCoords, 999999.f,true,false);
		ChangeUnreachTileToWall();
		if (m_heatMapDistance->GetTileHeatValue(m_exitCoords) < 999999.f)  //reachable
		{
			AddVertsForAllTiles();
			m_heatMapDistance->AddVertsForDebugDraw(m_heatmapVerts, 
				AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY), 
				FloatRange(0.f, (float)maxSteps));
			break;
		}
	}
	if (generateTimes >= 100)
	{
		ERROR_AND_DIE("cannot generate worms");
	}
}

void Map::InitializeTileMapFromImage(std::string const& imagePath)
{
	Image* testLevel = g_theRenderer->CreateImageFromFile(imagePath.c_str());
	m_dimensions = testLevel->GetDimensions();
	m_tiles.reserve(m_dimensions.x * m_dimensions.y);
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Rgba8 pixelColor=testLevel->GetTexelColor(IntVec2(columnIndex, rowIndex));
			std::string thisTileType=GetTileTypeFromColor(pixelColor);
			GenerateEachTile(columnIndex, rowIndex, thisTileType);
		}
	}
	delete testLevel;
	testLevel = nullptr;
	AddVertsForAllTiles();
	int maxSteps = PopulateDistanceField(*m_heatMapDistance, m_startCoords, 999999.f, true, false);
	m_heatMapDistance->AddVertsForDebugDraw(m_heatmapVerts,
		AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY),
		FloatRange(0.f, (float)maxSteps));
}

std::string const& Map::GetTileTypeFromColor(Rgba8 const& pixelColor)
{
	for (int i=0;i<(int)TileDefinition::s_tileDefinitions.size();i++)
	{
		if (pixelColor == TileDefinition::s_tileDefinitions[i].m_mapImageColor)
		{
			return TileDefinition::s_tileDefinitions[i].m_name;
		}
	}
	ERROR_AND_DIE("image's color is not appear in tile defs");
}

void Map::GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType)
{
	Tile newTile = Tile(columnIndex, rowIndex, GetTileDefIndex(thisTileType));
	m_tiles.push_back(newTile);
}

void Map::InitializeWorms(int wormsCount, int wormsLen,std::string wormsType)
{
	for (int i = 0; i < wormsCount; i++)
	{
		int startPointX;
		int startPointY;
		while (true)
		{
			startPointX = g_theGame->m_rng.RollRandomIntInRange(0, m_dimensions.x - 1);
			startPointY = g_theGame->m_rng.RollRandomIntInRange(0, m_dimensions.y - 1);
			if (GetTileType(startPointX, startPointY) == m_mapDef.m_fillTileType)
			{
				m_tiles[GetTileIndex(startPointX, startPointY)].m_tileDefIndex = GetTileDefIndex(wormsType);
				break;
			}
		}
		IntVec2 curPoint = IntVec2(startPointX, startPointY);
		IntVec2 nextPoint = IntVec2(startPointX, startPointY);
		for (int j = 0; j < wormsLen; j++)
		{
			int direction=g_theGame->m_rng.RollRandomIntInRange(1, 4);
			switch (direction)
			{
			case 1:
				nextPoint = curPoint + IntVec2(0, 1);
				break;
			case 2:
				nextPoint = curPoint + IntVec2(0, -1);
				break;
			case 3:
				nextPoint = curPoint + IntVec2(1, 0);
				break;
			case 4:
				nextPoint = curPoint + IntVec2(-1, 0);
				break;
			}
			if (GetTileType(nextPoint.x, nextPoint.y) == m_mapDef.m_fillTileType)
			{
				m_tiles[GetTileIndex(nextPoint.x, nextPoint.y)].m_tileDefIndex = GetTileDefIndex(wormsType);
				curPoint = nextPoint;
			}
		}
	}
}

void Map::ChangeUnreachTileToWall()
{
	for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++)
	{
		if (!TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex].m_isSolid
			&& !TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex].m_isWater
			&& m_heatMapDistance->m_values[i] == 999999.f)
		{
			m_tiles[i].m_tileDefIndex = GetTileDefIndex(m_mapDef.m_edgeTileType);
		}
	}
}

void Map::AddVertsForAllTiles()
{
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Vec2 worldBL_ofThisTile = Vec2((float)columnIndex * m_tileSizeX, (float)rowIndex * m_tileSizeY);
			AABB2 aabb_thisTile = AABB2(worldBL_ofThisTile, worldBL_ofThisTile + Vec2(m_tileSizeX, m_tileSizeY));

			TileDefinition curDef = TileDefinition::s_tileDefinitions[m_tiles[GetTileIndex(columnIndex, rowIndex)].m_tileDefIndex];
			AddVertsForAABB2D(m_tilemapVerts, aabb_thisTile,
				curDef.m_tintColor,
				curDef.m_UVs.m_mins,
				curDef.m_UVs.m_maxs);
		}
	}
}


void Map::InitializeAgentsFromDef(MapDefinition const& mapDef)
{
	UNUSED(mapDef);
	for (int i = 0; i < m_mapDef.m_rubbleCount; i++)
	{
		InitializeEachAgent(ENTITY_TYPE_NEU_RUBBLE, FACTION_NEUTRAL);
	}
	for (int i = 0; i <m_mapDef.m_scorpioCount; i++)
	{
		InitializeEachAgent(ENTITY_TYPE_EVIL_SCORPIO, FACTION_EVIL);
	}
	for (int i = 0; i < m_mapDef.m_leoCount; i++)
	{
		Entity* leo=InitializeEachAgent(ENTITY_TYPE_EVIL_LEO, FACTION_EVIL);
		leo->m_heatMapToPlayer = new TileHeatMap(m_dimensions);
		leo->m_maxSteps = PopulateDistanceField(*leo->m_heatMapToPlayer,
			GetTileCoordsFromPoint(leo->m_position), 999999.f, !leo->m_canSwim, true);
	}
	for (int i = 0; i < m_mapDef.m_ariesCount; i++)
	{
		Entity* aries=InitializeEachAgent(ENTITY_TYPE_EVIL_ARIES, FACTION_EVIL);
		aries->m_heatMapToPlayer = new TileHeatMap(m_dimensions);
		aries->m_maxSteps = PopulateDistanceField(*aries->m_heatMapToPlayer,
			GetTileCoordsFromPoint(aries->m_position), 999999.f, !aries->m_canSwim, true);
	}
	for (int i = 0; i < m_mapDef.m_capricornCount; i++)
	{
		Entity* capricorn = InitializeEachAgent(ENTITY_TYPE_EVIL_CAPRICORN, FACTION_EVIL);
		capricorn->m_heatMapToPlayer = new TileHeatMap(m_dimensions);
		capricorn->m_maxSteps = PopulateDistanceField(*capricorn->m_heatMapToPlayer,
			GetTileCoordsFromPoint(capricorn->m_position), 999999.f, !capricorn->m_canSwim, true);
	}
	for (int i = 0; i < m_mapDef.m_treeCount; i++)
	{
		 InitializeEachAgent(ENTITY_TYPE_NEU_TREE, FACTION_NEUTRAL);
	}

}

Entity* Map::InitializeEachAgent(EntityType entityType,EntityFaction entityFaction)
{
	while (true)
	{
		int x = g_theGame->m_rng.RollRandomIntInRange(1, m_dimensions.x - 2);
		int y = g_theGame->m_rng.RollRandomIntInRange(1, m_dimensions.y - 2);
		std::string const& type = GetTileType(x, y);
		if (type == m_mapDef.m_fillTileType )
		{
			float orientation = g_theGame->m_rng.RollRandomFloatInRange(0.f, 360.f);
			return SpawnNewEntity(entityType, entityFaction, Vec2((float)x + 0.5f, (float)y + 0.5f), orientation);
			//break;
		}
	}
}

Map::~Map()
{
	for (Entity* entity : m_allEntities) 
	{
		delete entity;
	}
	delete m_heatMapDistance;
	delete m_heatMapSolidForAmph;
	delete m_heatMapSolidForLand;
}

void Map::Update(float deltaSeconds)
{
	for (int i = 0; i < static_cast<int>(m_allEntities.size()); i++)
	{
		if (m_allEntities[i] != nullptr)
		{
			m_allEntities[i]->Update(deltaSeconds);
		}
	}
	CheckEntityCollWithEntity();
	CheckEntityCollWithSolidTiles();
	CheckBulletWithSolidTiles();
	CheckBulletWithAriesSheild();
	CheckBulletWithEntities();
	CameraFollowPlayer();
	if(!g_theGame->m_isChangeMap)
		CheckPlayerToExitPoint();
	DeleteGarbageEntity();

	if (m_isMapVertexDirty)
	{
		m_isMapVertexDirty = false;
		UpdateMapArray();
	}
	PopulateSolidField(*m_heatMapSolidForAmph, 999999.f, false, true);
	PopulateSolidField(*m_heatMapSolidForLand, 999999.f, true, true);
}

void Map::UpdateMapArray()
{
	AddVertsForAllTiles();
	int maxSteps = PopulateDistanceField(*m_heatMapDistance, m_startCoords, 999999.f, true, false);
	m_heatMapDistance->AddVertsForDebugDraw(m_heatmapVerts,
		AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY),
		FloatRange(0.f, (float)maxSteps));
}

void Map::Render() const
{
	Texture* tileTexture_8x8 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	g_theRenderer->BindTexture(tileTexture_8x8);
	g_theRenderer->DrawVertexArray(m_tilemapVerts);

	if (g_theGame->m_isHeatMapCount>0)
	{
		RenderHeatMap();
	}

	for (int i = 0; i < NUM_ENTITY_TYPES; i++)
	{
		if (i == ENTITY_TYPE_NEU_EXPLOSION|| i == ENTITY_TYPE_GOOD_FLAMEBULLET) g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		else g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		EntityList list = m_entityListsByType[i];
		for (int j = 0; j < static_cast<int>(m_entityListsByType[i].size()); j++)
		{
			if (list[j] != nullptr)
			{
				list[j]->Render();
			}
		}
	}
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	RenderEntityUI();
}

void Map::RenderDebugDraw()
{
	for (int i = 0; i < static_cast<int>(m_allEntities.size()); i++)
	{
		if (m_allEntities[i] != nullptr)
		{
			m_allEntities[i]->DrawDebugMode();
		}
	}

}

void Map::RenderHeatMap() const
{
	if (g_theGame->m_isHeatMapCount == 1)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(m_heatmapVerts);
	}
	else if (g_theGame->m_isHeatMapCount == 2)
	{
		std::vector<Vertex_PCU> heatmapSolidAmph;
		heatmapSolidAmph.reserve(m_dimensions.x * m_dimensions.y * 6);
		m_heatMapSolidForAmph->AddVertsForDebugDraw(heatmapSolidAmph,
			AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY),
			FloatRange(0.f, 0.f), Rgba8::BLACK, Rgba8::BLACK, 999999.f, Rgba8::WHITE);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(heatmapSolidAmph);
	}
	else if (g_theGame->m_isHeatMapCount == 3)
	{
		std::vector<Vertex_PCU> heatmapSolidLand;
		heatmapSolidLand.reserve(m_dimensions.x * m_dimensions.y * 6);
		m_heatMapSolidForLand->AddVertsForDebugDraw(heatmapSolidLand,
			AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY),
			FloatRange(0.f, 0.f), Rgba8::BLACK, Rgba8::BLACK, 999999.f, Rgba8::WHITE);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(heatmapSolidLand);
	}
	else
	{
		std::vector<Vertex_PCU> heatmapVertsLeo;
		heatmapVertsLeo.reserve(m_dimensions.x * m_dimensions.y * 6);
		m_entityListsByType[ENTITY_TYPE_EVIL_CAPRICORN][0]->m_heatMapToPlayer->AddVertsForDebugDraw(heatmapVertsLeo,
			AABB2(0.f, 0.f, (float)m_dimensions.x * m_tileSizeX, (float)m_dimensions.y * m_tileSizeY),
			FloatRange(0.f, (float)m_entityListsByType[ENTITY_TYPE_EVIL_CAPRICORN][0]->m_maxSteps));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(heatmapVertsLeo);

		std::vector<Vertex_PCU> arrowVerts;
		Vec2 tailPos = m_entityListsByType[ENTITY_TYPE_EVIL_CAPRICORN][0]->m_position + Vec2::MakeFromPolarDegrees(60.f) * 300.f;
		AddVertsForArrow2D(arrowVerts, tailPos, m_entityListsByType[ENTITY_TYPE_EVIL_CAPRICORN][0]->m_position+Vec2(0.7f,0.7f), 0.5f, 0.1f, Rgba8::MAGNETA);
		g_theRenderer->DrawVertexArray(arrowVerts);
	}
}

void Map::RenderEntityUI() const
{
	for (EntityList const& list : m_agentListByFaction)
	{
		for (Entity* agent : list)
		{
			if(agent!=nullptr)
				agent->RenderUI();
		}
	}
}

std::string const& Map::GetTileType(int coor_x, int coor_y)
{
	int index = coor_x + coor_y * m_dimensions.x;
	return TileDefinition::s_tileDefinitions[ m_tiles[index].m_tileDefIndex].m_name;
}

Tile* Map::GetTile(int coor_x, int coor_y)
{
	int index = coor_x + coor_y * m_dimensions.x;
	return &m_tiles[index];
}

int Map::GetTileIndex(int coor_x, int coor_y)
{
	return coor_x + coor_y * m_dimensions.x;
}

Entity* Map::SpawnNewEntity(EntityType type,EntityFaction faction, Vec2 const& position, float orientationDegrees)
{
	Entity* newEntity = CreateNewEntityOfType(type,faction); //add faction ,no pos and ori ,change pos and ori to add to map func
	AddEntityToMap(*newEntity,position,orientationDegrees); //there know this map,pos,orientation
	return newEntity;
}

Entity* Map::CreateNewEntityOfType(EntityType type, EntityFaction faction)
{
	switch(type)
	{
	case ENTITY_TYPE_GOOD_PLAYER:
		return new Player(type,faction);  //only faction new PlayerTank(faction)
	case ENTITY_TYPE_NEU_RUBBLE:
		return new Rubble(type, faction);
	case ENTITY_TYPE_EVIL_SCORPIO:
		return new Scorpio(type,faction);
	case ENTITY_TYPE_EVIL_LEO:
		return new Leo(type,faction);
	case ENTITY_TYPE_EVIL_ARIES:
		return new Aries(type, faction);
	case ENTITY_TYPE_EVIL_CAPRICORN:
		return new Capricorn(type, faction);
	case ENTITY_TYPE_GOOD_BULLET:
		return new Bullet(type, faction);
	case ENTITY_TYPE_GOOD_FLAMEBULLET:
		return new Bullet(type, faction);
	case ENTITY_TYPE_EVIL_BULLET:
		return new Bullet(type, faction);
	case ENTITY_TYPE_GOOD_BOLT:
		return new Bullet(type, faction);
	case ENTITY_TYPE_EVIL_BOLT:
		return new Bullet(type, faction);
	case ENTITY_TYPE_EVIL_GDMISSILE:
		return new Bullet(type, faction);
	case ENTITY_TYPE_NEU_EXPLOSION:
		return new Explosion(type, faction);
	case ENTITY_TYPE_NEU_TREE:
		return new Tree(type, faction);
	default:
		return new Player(ENTITY_TYPE_GOOD_PLAYER, faction);
	}
}

void Map::AddEntityToMap(Entity& thisEntity,Vec2 position,float orientation)
{
	thisEntity.m_map = this;
	thisEntity.m_position = position;
	thisEntity.m_orientationDegrees = orientation;
	AddEntityToList(thisEntity, m_allEntities);
	AddEntityToList(thisEntity, m_entityListsByType[thisEntity.m_type]);
	AddEntityToList(thisEntity, m_entityListsByFaction[thisEntity.m_faction]);

	if (IsBullet(&thisEntity))
	{
		AddEntityToList(thisEntity, m_allBullets);
		AddEntityToList(thisEntity,m_bulletListByFaction[thisEntity.m_faction]);
	}
	else if (IsAgent(&thisEntity))
	{
		AddEntityToList(thisEntity, m_agentListByFaction[thisEntity.m_faction]);
	}
	else if (IsObstacle(&thisEntity))
	{
		AddEntityToList(thisEntity, m_allObstacles);
	}
}

void Map::RemoveEntityFromMap(Entity& thisEntity)
{
	RemoveEntityFromList(thisEntity, m_allEntities);
	RemoveEntityFromList(thisEntity, m_entityListsByType[thisEntity.m_type]);
	RemoveEntityFromList(thisEntity, m_entityListsByFaction[thisEntity.m_faction]);
	if (IsBullet(&thisEntity))
	{
		RemoveEntityFromList(thisEntity, m_allBullets);
		RemoveEntityFromList(thisEntity, m_bulletListByFaction[thisEntity.m_faction]);
	}
	else if (IsAgent(&thisEntity))
	{
		//RemoveEntityFromList(thisEntity, m_agentListByFaction[thisEntity.m_faction]);
		RemoveEntityFromList(thisEntity, m_agentListByFaction[thisEntity.m_faction]);
	}
	else if (IsObstacle(&thisEntity))
	{
		RemoveEntityFromList(thisEntity, m_allObstacles);
	}
}

void Map::AddEntityToList(Entity& thisEntity, EntityList& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i] == nullptr)
		{
			list[i] = &thisEntity;
			return;
		}
	}
	list.push_back(&thisEntity);
}

void Map::RemoveEntityFromList(Entity& thisEntity, EntityList& list)
{
	for (int i = 0; i < static_cast<int>(list.size()); i++)
	{
		if (list[i] == &thisEntity)
		{
			list[i] = nullptr;
			return;
		}
	}
}

void Map::DeleteGarbageEntity()
{
	for (Entity* entity : m_allEntities)
	{
		if (entity == nullptr) continue;
		if (IsGarbage(entity))
		{
			RemoveEntityFromMap(*entity);
			delete entity;
		}
	}
}

bool Map::IsBullet(Entity* entity)
{
	switch (entity->m_type)
	{
	case	ENTITY_TYPE_GOOD_BULLET:
	case ENTITY_TYPE_GOOD_FLAMEBULLET:
	case	ENTITY_TYPE_EVIL_BULLET:
	case	ENTITY_TYPE_GOOD_BOLT:
	case	ENTITY_TYPE_EVIL_BOLT:
	case ENTITY_TYPE_EVIL_GDMISSILE:
		return true;

	case ENTITY_TYPE_GOOD_PLAYER:
	case	ENTITY_TYPE_EVIL_SCORPIO:
	case	ENTITY_TYPE_EVIL_LEO:
	case	ENTITY_TYPE_EVIL_ARIES:
	case ENTITY_TYPE_EVIL_CAPRICORN:
	case ENTITY_TYPE_UNSURE:
		return false;
	default:
		return false;
	}
}

bool Map::IsAgent(Entity* entity)
{
	switch (entity->m_type)
	{
	case	ENTITY_TYPE_GOOD_BULLET:
	case ENTITY_TYPE_GOOD_FLAMEBULLET:
	case	ENTITY_TYPE_EVIL_BULLET:
	case	ENTITY_TYPE_GOOD_BOLT:
	case	ENTITY_TYPE_EVIL_BOLT:
	case ENTITY_TYPE_EVIL_GDMISSILE:
		return false;

	case	ENTITY_TYPE_GOOD_PLAYER:
	case	ENTITY_TYPE_EVIL_SCORPIO:
	case	ENTITY_TYPE_EVIL_LEO:
	case	ENTITY_TYPE_EVIL_ARIES:
	case ENTITY_TYPE_EVIL_CAPRICORN:
	case ENTITY_TYPE_UNSURE:
		return true;
	default:
		return false;
	}
}

bool Map::IsObstacle(Entity* entity)
{
	switch (entity->m_type)
	{
	case	ENTITY_TYPE_NEU_TREE:
		return true;
	default:
		return false;
	}
}

bool Map::IsGarbage(Entity* entity)
{
	return entity->IsGarbage();
}

IntVec2 Map::GetTileCoordsFromPoint(Vec2 const& point)
{
	return IntVec2(static_cast<int>(floorf(point.x)), static_cast<int>(floorf(point.y)));
}

bool Map::IsPointInSolid(Vec2 const& point)
{
	IntVec2 coords=GetTileCoordsFromPoint(point);
	bool isSolid= MatchTileDef(GetTileType(coords.x, coords.y)).m_isSolid;
	return isSolid;
}

bool Map::IsTileSolid(IntVec2 const& coordinate)
{
	return MatchTileDef(GetTileType(coordinate.x, coordinate.y)).m_isSolid;
}

bool Map::IsTileSolid(int x, int y)
{
	return MatchTileDef(GetTileType(x, y)).m_isSolid;
}

bool Map::IsTileWater(int x, int y)
{
	return MatchTileDef(GetTileType(x, y)).m_isWater;
}

bool Map::IsObstacleTile(int x, int y)
{
	for (Entity* scorpio : m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO])
	{
		if (scorpio&&!scorpio->m_isDead)
		{
			if (GetTileCoordsFromPoint(scorpio->m_position) == IntVec2(x, y))
			{
				return true;
			}
		}
	}
	for (Entity* obstacle : m_allObstacles)
	{
		if (obstacle && !obstacle->m_isDead)
		{
			if (GetTileCoordsFromPoint(obstacle->m_position) == IntVec2(x, y))
			{
				return true;
			}
		}
	}
	return false;
}

void Map::CheckEntityCollWithEntity()
{
	for (Entity* entityA : m_allEntities)
	{
		if (entityA == nullptr||entityA->m_isDead) continue;
		for (Entity* entityB : m_allEntities)
		{
			if (entityB == nullptr || entityB->m_isDead) continue;
			if (entityA == entityB)
			{
				continue;
			}
			bool canAPushB = entityA->m_doesPushEntities && entityB->m_isPushedByEntities;
			bool canBPushA= entityB->m_doesPushEntities && entityA->m_isPushedByEntities;
			if (!canAPushB && !canBPushA)
			{
				continue;
			}
			else if (canAPushB && canBPushA)
			{
				PushDiscsOutOfEachOther2D(entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius);
			}
			else if (!canAPushB && canBPushA)
			{
				PushDiscOutOfDisc2D(entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius);
			}
			else if (canAPushB && !canBPushA)
			{
				PushDiscOutOfDisc2D(entityB->m_position, entityB->m_physicsRadius, entityA->m_position, entityA->m_physicsRadius);
			}	
		}
	}
}


void Map::CheckEntityCollWithSolidTiles()
{
	for (Entity* entity : m_allEntities)
	{
		if (entity == nullptr) continue;
		if (entity == m_player && g_theGame->IsNoClip())
		{
			continue;
		}
		if (entity->m_isPushedByWalls)
		{
			IntVec2 entityTileCoordsPos = GetTileCoordsFromPoint(entity->m_position);
			//N
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y + 1), entity->m_position, entity->m_physicsRadius,entity->m_canSwim);
			//S
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y - 1), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//W
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//E
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//NE
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y + 1), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//NW
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y - 1), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//SE
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y + 1), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
			//SW
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y - 1), entity->m_position, entity->m_physicsRadius, entity->m_canSwim);
		}
	}	
}

void Map::PushOutOfEachTile(IntVec2 tileCoords,Vec2& entityPos,float entityPhyRadius,bool canSwim)
{
	if (IsTileSolid(tileCoords)||(IsTileWater(tileCoords.x,tileCoords.y)&&!canSwim))
	{
		AABB2 thisTileBox = AABB2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y),
			static_cast<float>(tileCoords.x + 1.f), static_cast<float>(tileCoords.y + 1));
		PushDiscOutOfAABB2D(entityPos, entityPhyRadius, thisTileBox);
	}
}

void Map::CheckBulletWithSolidTiles()
{
	for (Entity* bullet : m_allBullets)
	{
		if (bullet == nullptr) continue;
		IntVec2 entityTileCoordsPos = GetTileCoordsFromPoint(bullet->m_position);
		if (IsTileSolid(entityTileCoordsPos))
		{
			bullet->m_isGarbage = true;
			continue;
		}
		//N
		IntVec2 northTileCoords = IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y + 1);
		if(BulletVsEachTile(entityTileCoordsPos, northTileCoords, bullet))	continue;
		//S
		IntVec2 southTileCoords = IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y - 1);
		if (BulletVsEachTile(entityTileCoordsPos, southTileCoords, bullet))	continue;
		//W
		IntVec2 westTileCoords = IntVec2(entityTileCoordsPos.x-1, entityTileCoordsPos.y);
		if (BulletVsEachTile(entityTileCoordsPos, westTileCoords, bullet))	continue;
		//E
		IntVec2 eastTileCoords = IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y);
		if (BulletVsEachTile(entityTileCoordsPos, eastTileCoords, bullet))	continue;
	}
}

bool Map::BulletVsEachTile(IntVec2& entityTileCoordsPos, IntVec2& nextTileCoordsPos, Entity* bullet)
{
	if (IsTileSolid(nextTileCoordsPos))
	{
		Tile* thisTile = GetTile(nextTileCoordsPos.x, nextTileCoordsPos.y);
		AABB2 thisTileBox = AABB2(static_cast<float>(nextTileCoordsPos.x), static_cast<float>(nextTileCoordsPos.y),
			static_cast<float>(nextTileCoordsPos.x + 1.f), static_cast<float>(nextTileCoordsPos.y + 1));
		Vec2 nearPoint = GetNearestPointOnAABB2D(bullet->m_position, thisTileBox);
		if (GetDistanceSquared2D(nearPoint, bullet->m_position) <= (bullet->m_physicsRadius) * (bullet->m_physicsRadius))
		{
			PushOutOfEachTile(nextTileCoordsPos, bullet->m_position, bullet->m_physicsRadius,true);
			static_cast<Bullet*>(bullet)->HitWall(entityTileCoordsPos-nextTileCoordsPos);
			if (TileDefinition::s_tileDefinitions[thisTile->m_tileDefIndex].m_isDestruct)
			{
				thisTile->m_health-=1.f;
				if (thisTile->m_health <= 0.f)
				{
					m_isMapVertexDirty = true;
					thisTile->m_tileDefIndex = 0;
				}
			}
			return true;
		}
		return false;
	}
	return false;
}

void Map::CheckBulletWithAriesSheild()
{
	for (Entity* aries : m_entityListsByType[ENTITY_TYPE_EVIL_ARIES])
	{
		if (aries == nullptr || aries->m_isDead) continue;
		for (Entity* playerBullet : m_entityListsByType[ENTITY_TYPE_GOOD_BULLET])
		{
			if (playerBullet == nullptr) continue;
			if (DoDiscsOverlap(aries->m_position, aries->m_physicsRadius, playerBullet->m_position, playerBullet->m_physicsRadius))
			{
				float ariesToBulletOri = (playerBullet->m_position - aries->m_position).GetOrientationDegrees();
				if (abs(GetShortestAngularDispDegrees(aries->m_orientationDegrees, ariesToBulletOri)) < 45.f)
				{
					static_cast<Bullet*>(playerBullet)->HitWall((playerBullet->m_position - aries->m_position).GetNormalized());
					PushDiscOutOfDisc2D(playerBullet->m_position, playerBullet->m_physicsRadius, aries->m_position, aries->m_physicsRadius);
					g_theAudio->StartSound(g_theGame->m_bulletBounce);
				}
			}
		}
	}
}

void Map::CheckBulletWithEntities()
{
	for (Entity* agent : m_agentListByFaction[FACTION_GOOD])
	{
		if (agent == nullptr||!agent->m_isHitByBullets || agent->m_isDead) continue;
		for (Entity* bullet : m_bulletListByFaction[FACTION_EVIL])
		{
			if (bullet == nullptr) continue;
			if (DoDiscsOverlap(agent->m_position, agent->m_physicsRadius, bullet->m_position, bullet->m_physicsRadius))
			{
				static_cast<Bullet*>(bullet)->HitEntity();
				agent->GetHurt(static_cast<Bullet*>(bullet)->m_causeHurtPoint);
				if (agent->m_faction == FACTION_EVIL)
				{
					g_theAudio->StartSound(g_theGame->m_enemyHit);
				}
				else
				{
					g_theAudio->StartSound(g_theGame->m_playerHit);
				}
			}
		}
	}

	for (Entity* agent : m_agentListByFaction[FACTION_EVIL])
	{
		if (agent == nullptr || !agent->m_isHitByBullets || agent->m_isDead) continue;
		for (Entity* bullet : m_bulletListByFaction[FACTION_GOOD])
		{
			if (bullet == nullptr) continue;
			if (DoDiscsOverlap(agent->m_position, agent->m_physicsRadius, bullet->m_position, bullet->m_physicsRadius))
			{
				if (agent->m_type == ENTITY_TYPE_EVIL_ARIES)
				{
					float ariesToBulletOri = (bullet->m_position - agent->m_position).GetOrientationDegrees();
					if (abs(GetShortestAngularDispDegrees(agent->m_orientationDegrees, ariesToBulletOri)) < 45.f)
					{
						continue;
					}
				}
				static_cast<Bullet*>(bullet)->HitEntity();
				agent->GetHurt(static_cast<Bullet*>(bullet)->m_causeHurtPoint);
			}
		}
	}
	for (Entity* obstacle : m_allObstacles)
	{
		if (obstacle == nullptr || !obstacle->m_isHitByBullets || obstacle->m_isDead) continue;
		for (Entity* bullet : m_allBullets)
		{
			if (bullet == nullptr) continue;
			if (DoDiscsOverlap(obstacle->m_position, obstacle->m_physicsRadius, bullet->m_position, bullet->m_physicsRadius))
			{
				if (bullet->m_type == ENTITY_TYPE_GOOD_BULLET)
				{
					static_cast<Bullet*>(bullet)->HitWall((bullet->m_position - obstacle->m_position).GetNormalized());
					PushDiscOutOfDisc2D(bullet->m_position, bullet->m_physicsRadius, obstacle->m_position, obstacle->m_physicsRadius);
					g_theAudio->StartSound(g_theGame->m_bulletBounce);
				}
				else
				{
					static_cast<Bullet*>(bullet)->HitEntity();
				}
				obstacle->GetHurt(static_cast<Bullet*>(bullet)->m_causeHurtPoint);
			}
		}
	}
}

RaycastResult2D Map::RaycastVsTiles(Ray const& ray)
{
	RaycastResult2D result= RaycastResult2D(ray);
	constexpr int steps_per_unit = 100;
	int numSteps = (int)ray.m_rayMaxLength * steps_per_unit;
	float dist_per_step = 1.f / (float)steps_per_unit;
	IntVec2 curTileCoords;
	IntVec2 previousCoords= GetTileCoordsFromPoint(ray.m_rayStartPos);
	for (int step = 0; step < numSteps; step++)
	{
		float stepFwdDist = dist_per_step * static_cast<float>(step);
		Vec2 curStepRayEnd = ray.m_rayStartPos + (ray.m_rayFwdNormal * stepFwdDist);
		curTileCoords = GetTileCoordsFromPoint(curStepRayEnd);
		if (curTileCoords==previousCoords)
		{
			continue;
		}

		if (IsPointInSolid(curStepRayEnd))
		{
			result.m_didImpact = true;
			result.m_impactPos = curStepRayEnd;
			result.m_impactDist = stepFwdDist;

			IntVec2 impartNormal = previousCoords - curTileCoords;
			result.m_impactNormal = Vec2((float)impartNormal.x, (float)impartNormal.y);
			return result;
		}
		previousCoords = curTileCoords;
	}
	result.m_didImpact = false;
	return result;
}

RaycastResult2D Map::FastRaycastVsTiles(Ray const& ray,bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	RaycastResult2D result = RaycastResult2D(ray);
	IntVec2 startTileCoords=GetTileCoordsFromPoint(ray.m_rayStartPos);

	if (MatchTileDef(GetTileType(startTileCoords.x, startTileCoords.y)).m_isSolid
		|| (treatWaterAsSolid && IsTileWater(startTileCoords.x, startTileCoords.y)) 
			||(treatScorpioAsSolid && IsObstacleTile(startTileCoords.x, startTileCoords.y)))
	{
		result.m_didImpact = true;
		result.m_impactPos = ray.m_rayStartPos;
		result.m_impactDist =0.f;
		result.m_impactNormal = Vec2::ZERO;
		return result;
	}
	float fwdDistPerXCrossing =m_tileSizeX/ abs(ray.m_rayFwdNormal.x);
	int tileStepDirectionX = (ray.m_rayFwdNormal.x < 0) ? -1 : 1;
	float xAtFirstXCrossing = startTileCoords.x + (tileStepDirectionX + 1.f) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - ray.m_rayStartPos.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = m_tileSizeY / abs(ray.m_rayFwdNormal.y);
	int tileStepDirectionY = (ray.m_rayFwdNormal.y < 0) ? -1 : 1;
	float yAtFirstYCrossing = startTileCoords.y + (tileStepDirectionY + 1.f) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - ray.m_rayStartPos.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	while (true)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) //touch x sooner
		{
			if (fwdDistAtNextXCrossing > ray.m_rayMaxLength)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.x += tileStepDirectionX;
			if (IsTileSolid(startTileCoords) || (treatWaterAsSolid && IsTileWater(startTileCoords.x, startTileCoords.y))
				|| (treatScorpioAsSolid && IsObstacleTile(startTileCoords.x, startTileCoords.y)))
			{
				result.m_didImpact = true;
				result.m_impactPos = ray.m_rayStartPos+ray.m_rayFwdNormal*fwdDistAtNextXCrossing;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactNormal = Vec2((float)-(float)tileStepDirectionX, 0.f);
				return result;
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else
		{
			if (fwdDistAtNextYCrossing > ray.m_rayMaxLength)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.y += tileStepDirectionY;
			if (IsTileSolid(startTileCoords) || (treatWaterAsSolid && IsTileWater(startTileCoords.x, startTileCoords.y))
				|| (treatScorpioAsSolid && IsObstacleTile(startTileCoords.x, startTileCoords.y)))
			{
				result.m_didImpact = true;
				result.m_impactPos = ray.m_rayStartPos + ray.m_rayFwdNormal * fwdDistAtNextYCrossing;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactNormal = Vec2(0.f, (float)-tileStepDirectionY);
				return result;
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}

bool Map::HasLineOfSight(Ray const& ray, Player* entity)
{
	RaycastResult2D resultVsEntity = RaycastVsDisc2D(ray.m_rayStartPos,ray.m_rayFwdNormal,ray.m_rayMaxLength, entity->m_position, entity->m_physicsRadius);
	if (!resultVsEntity.m_didImpact)
	{
		return false;
	}
	else
	{
		RaycastResult2D resultVsSolidTiles = FastRaycastVsTiles(ray,false,false);
		if (!resultVsSolidTiles.m_didImpact)
		{
			return true;
		}
		else
		{
			if (resultVsSolidTiles.m_impactDist <= resultVsEntity.m_impactDist)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
}

void Map::CheckPlayerToExitPoint()
{
	Vec2 exitPoint = Vec2((float)m_exitCoords.x + 0.5f, (float)m_exitCoords.y + 0.5f);
	if (IsPointInsideDisc2D(exitPoint, m_player->m_position, m_player->m_physicsRadius-0.25f))
	{
		g_theGame->ChangeToNextMap();
	}
}

void Map::AgentShooting(EntityType bulletType,EntityFaction faction,Vec2 const& tipPoint,float orientation, float explosionSize)
{
	SpawnNewEntity(bulletType, faction, tipPoint, orientation);
	SpawnExplosion(tipPoint + Vec2::MakeFromPolarDegrees(orientation) * 0.2f, explosionSize,g_explosionAnimDef_Fast);
}

void Map::SpawnExplosion(Vec2 const& spawnPos, float explosionSize, SpriteAnimDefinition* animDef)
{
	float explosionOri = g_theGame->m_rng.RollRandomFloatInRange(0.f, 360.f);
	Explosion* newExplosion = static_cast<Explosion*>(SpawnNewEntity(ENTITY_TYPE_NEU_EXPLOSION, FACTION_NEUTRAL, spawnPos, explosionOri));
	newExplosion->m_size = explosionSize;
	newExplosion->m_spriteAnimDef = animDef;
}



void Map::CameraFollowPlayer()
{
	Vec2 m_followedAim = m_player->m_position;
	
	if (m_player->m_position.x <m_viewLen.x*0.5f)
	{
		m_followedAim.x = m_viewLen.x * 0.5f;
	}
	else if (m_player->m_position.x > m_dimensions.x*m_tileSizeX - m_viewLen.x * 0.5f)
	{
		m_followedAim.x = m_dimensions.x * m_tileSizeX - m_viewLen.x * 0.5f;
	}
	if (m_player->m_position.y < m_viewLen.y * 0.5f)
	{
		m_followedAim.y = m_viewLen.y * 0.5f;
	}
	else if (m_player->m_position.y > m_dimensions.y * m_tileSizeY - m_viewLen.y * 0.5f)
	{
		m_followedAim.y = m_dimensions.y * m_tileSizeY - m_viewLen.y * 0.5f;
	}

	Vec2 blTrans = Vec2(-m_viewLen.x * 0.5f, -m_viewLen.y * 0.5f) + m_followedAim;
	Vec2 trTrans = Vec2(m_viewLen.x * 0.5f, m_viewLen.y * 0.5f) + m_followedAim;
	g_theGame->m_worldCamera.SetOrthoView(blTrans, trTrans);
}

TileDefinition const& Map::MatchTileDef(std::string const& tileName)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (tileName == TileDefinition::s_tileDefinitions[i].m_name)
		{
			return TileDefinition::s_tileDefinitions[i];
		}
	}
	ERROR_AND_DIE("can not find the tile def");
}

int Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	out_distanceField.SetAllValues(maxCost);
	out_distanceField.SetTileHeatValue(startCoords, 0.f);
	int step = 0;

	while (true)
	{
		bool isChange = false;
		for (int coordX = 0; coordX < out_distanceField.m_dimensions.x; coordX++)
		{
			for (int coordY = 0; coordY < out_distanceField.m_dimensions.y; coordY++)
			{
				int curIndex = coordX + coordY * m_dimensions.x;
				if (out_distanceField.m_values[curIndex] == step)
				{
					//N
					if (IsCoordsLegal(coordX, coordY + 1))
					{
						if (!IsTileSolid(coordX, coordY + 1) && !(treatScorpioAsSolid&&IsObstacleTile(coordX, coordY + 1))
							&& !(treatWaterAsSolid && IsTileWater(coordX, coordY + 1))
							&& out_distanceField.GetTileHeatValue(coordX, coordY + 1) > step + 1)
						{
							isChange = true;
							out_distanceField.SetTileHeatValue(coordX, coordY + 1,(float)step+1.f);
						}
					}
					//S
					if (IsCoordsLegal(coordX, coordY -1))
					{
						if (!IsTileSolid(coordX, coordY - 1) && !(treatScorpioAsSolid && IsObstacleTile(coordX, coordY- 1))
							&& !(treatWaterAsSolid && IsTileWater(coordX, coordY - 1))
							&& out_distanceField.GetTileHeatValue(coordX, coordY - 1) > (float)step + 1.f)
						{
							isChange = true;
							out_distanceField.SetTileHeatValue(coordX, coordY-1, (float)step + 1.f);
						}
					}
					//E
					if (IsCoordsLegal(coordX + 1, coordY))
					{
						if (!IsTileSolid(coordX + 1, coordY) && !(treatScorpioAsSolid && IsObstacleTile(coordX+1, coordY))
							&& !(treatWaterAsSolid && IsTileWater(coordX+1, coordY))
							&& out_distanceField.GetTileHeatValue(coordX + 1, coordY) > (float)step + 1.f)
						{
							isChange = true;
							out_distanceField.SetTileHeatValue(coordX+1, coordY, (float)step + 1.f);
						}
					}
					//W
					if (IsCoordsLegal(coordX - 1, coordY))
					{
						if (!IsTileSolid(coordX - 1, coordY) && !(treatScorpioAsSolid && IsObstacleTile(coordX - 1, coordY))
							&& !(treatWaterAsSolid && IsTileWater(coordX - 1, coordY))
							&& out_distanceField.GetTileHeatValue(coordX - 1, coordY) > (float)step + 1.f)
						{
							isChange = true;
							out_distanceField.SetTileHeatValue(coordX - 1, coordY, (float)step + 1.f);
						}
					}
				}
			}
		}
		if (isChange == false)
		{
			break;
		}
		step++;
	}
	return step;
	
}

void Map::PopulateSolidField(TileHeatMap& out_distanceField, float maxCost, bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	out_distanceField.SetAllValues(maxCost);
	for (int coordX = 0; coordX < out_distanceField.m_dimensions.x; coordX++)
	{
		for (int coordY = 0; coordY < out_distanceField.m_dimensions.y; coordY++)
		{
			if (!IsTileSolid(coordX, coordY) && !(treatScorpioAsSolid && IsObstacleTile(coordX, coordY))
				&& !(treatWaterAsSolid && IsTileWater(coordX, coordY)))
			{
				out_distanceField.SetTileHeatValue(coordX, coordY, 0);
			}
		}
	}
}

void Map::GenerateEntityPathToGoal(TileHeatMap& refer_distanceField, std::vector<Vec2>& pathPoints, IntVec2 const& startCoords, IntVec2 const& goalCoords)
{
	pathPoints.clear();
	IntVec2 curCoords = startCoords;
	Vec2 curPos=Vec2((float)curCoords.x+0.5f, (float)curCoords.y + 0.5f);
	while (goalCoords!=curCoords)
	{
		curPos =ImprovedPickNextWayPointPosition(refer_distanceField, curPos);
		curCoords = GetTileCoordsFromPoint(curPos);
		pathPoints.push_back(curPos);
	}
	std::reverse(pathPoints.begin(), pathPoints.end());
}

bool Map::GenerateFindOutEffect(EventArgs& args)
{
	UNUSED(args);
	if ((float)GetCurrentTimeSeconds() - g_lastFindOutSoundTime > 0.1f)
	{
		g_lastFindOutSoundTime = (float)GetCurrentTimeSeconds();
		g_theAudio->StartSound(g_theGame->m_clickSound);	
	}
	return false;
}

Vec2 Map::ImprovedPickNextWayPointPosition(TileHeatMap& heatmap, Vec2& curPos)
{
	IntVec2 curCoords = GetTileCoordsFromPoint(curPos);
	//N
	IntVec2 northTileCoords = curCoords + IntVec2(0, 1);
	float northHeat = heatmap.m_values[GetTileIndex(northTileCoords.x, northTileCoords.y)];
	//S
	IntVec2 southTileCoords = curCoords + IntVec2(0, -1);
	float southHeat = heatmap.m_values[GetTileIndex(southTileCoords.x, southTileCoords.y)];
	//E
	IntVec2 eastTileCoords = curCoords + IntVec2(1, 0);
	float eastHeat = heatmap.m_values[GetTileIndex(eastTileCoords.x, eastTileCoords.y)];
	//W
	IntVec2 westTileCoords = curCoords + IntVec2(-1, 0);
	float westHeat = heatmap.m_values[GetTileIndex(westTileCoords.x, westTileCoords.y)];

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

bool Map::IsCoordsLegal(int coordsX, int coordsY)
{
	if (coordsX >= 0 && coordsX <= m_dimensions.x - 1 && coordsY >= 0 && coordsY <= m_dimensions.y - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int Map::GetTileDefIndex(std::string const& typeName)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (typeName == TileDefinition::s_tileDefinitions[i].m_name)
		{
			return i;
		}
	}
	return -1;
}
