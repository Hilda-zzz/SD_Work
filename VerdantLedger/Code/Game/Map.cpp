#include "Game/Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Player.hpp"
#include "GameCommon.hpp"
#include "Game/TileMap.hpp"
#include "Game/TileMapManager.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/TileTypesInGame.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/TileChunk.hpp"

extern Renderer* g_theRenderer;
extern Window* g_theWindow;

Map::Map(Game* game, TileMap* tileMap, Player* player):
	m_game(game),m_tileMap(tileMap),m_player(player)
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_gameplayCam.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_gameplayCam.SetOrthographicView(m_player->m_position - g_halfGameCamDimensions, m_player->m_position + g_halfGameCamDimensions,0.f,1000.f);

// 	m_dirtyObstacleDelayTimer = Timer(0.5f);
// 	m_dirtyFarmlandDelayTimer = Timer(0.5f);

	m_chunkUpdateManager = ChunkUpddateManger(m_tileMap->m_markLayer);
}

Map::~Map()
{
}

void Map::Update(float deltaSeconds)
{	
	m_playerPrevPos = m_player->m_position;
	m_player->Update(deltaSeconds);
	CheckPlayerCollWithSolidTiles();
	m_gameplayCam.SetOrthographicView(m_player->m_position - g_halfGameCamDimensions, m_player->m_position + g_halfGameCamDimensions, 0.f, 1000.f);

	IntVec2 m_playerFrontGridPos = GetTileCoordsFromPoint(m_player->m_position)+IntVec2(0,-1);
 	m_tileMap->UpdateTransparentObject(m_playerFrontGridPos);
 	m_tileMap->Update(deltaSeconds);

	m_chunkUpdateManager.UpdateDirtyChunks();
}

void Map::Render() const
{
	g_theRenderer->BeginCamera(m_gameplayCam);

	m_player->Render();

	// transparent things must be rendered at the end
	m_tileMap->Render();

	g_theRenderer->EndCamera(m_gameplayCam);
}

void Map::UpdateCamFollow(float deltaSeconds)
{
	Vec2 aimCamCenter = m_player->m_position;
	Vec2 cameraSmoothPos = Interpolate(m_playerPrevPos, aimCamCenter, 2.f * deltaSeconds);

	m_gameplayCam.SetOrthographicView(cameraSmoothPos - g_halfGameCamDimensions, cameraSmoothPos + g_halfGameCamDimensions, 0.f, 1000.f);
}


void Map::CheckPlayerCollWithSolidTiles()
{
	if (m_player)
	{
 		IntVec2 entityTileCoordsPos = GetTileCoordsFromPoint(m_player->m_position);
 		//N
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
  		//S                                                                                                
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
  		//W                                                                                                
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y), m_player->m_position, m_player->m_physicsRadius);
  		//E                                                                                               
  		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y), m_player->m_position, m_player->m_physicsRadius);
  		//NE
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
		//NW
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
		//SE
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y + 1), m_player->m_position, m_player->m_physicsRadius);
		//SW
		PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y - 1), m_player->m_position, m_player->m_physicsRadius);
	}
}

IntVec2 Map::GetTileCoordsFromPoint(Vec2 const& point)
{
    return IntVec2(static_cast<int>(floorf(point.x)), static_cast<int>(floorf(point.y)));
}

void Map::UsingToolTowardsGridPos(IntVec2 const& aimGridPos, PlayerTools toolType)
{
	TileChunk* curChunk=m_tileMap->m_markLayer->GetChunkContaining(aimGridPos);
	uint64_t tileKey = m_tileMap->GetTileKey(aimGridPos);
	auto it =curChunk->m_keyToDynamicTileData.find(tileKey);
	if (it != curChunk->m_keyToDynamicTileData.end())
	{
		if ((it->second.m_obstacleType == ObstacleType::ROCK&& toolType==PlayerTools::PICKAXE)||
			(it->second.m_obstacleType == ObstacleType::WEED&& toolType == PlayerTools::SICKLE))
		{
			it->second.m_curObstacleDurability--;
			if (it->second.m_curObstacleDurability <= 0)
			{
				it->second.m_obstacleType = ObstacleType::NONE;
				m_chunkUpdateManager.MarkChunkDirty(curChunk, DirtyType::DIRTY_STATIC_OBS, aimGridPos);
			}
		}

		if (it->second.m_farmState == FarmState::UNPLOWED
			&& it->second.m_obstacleType == ObstacleType::NONE
			&& toolType == PlayerTools::SHOVEL)
		{
			it->second.m_farmState = FarmState::PLOWED;
			m_chunkUpdateManager.MarkChunkDirty(curChunk, DirtyType::DIRTY_FARMLAND, aimGridPos);
		}

		if (it->second.m_farmState == FarmState::PLOWED
			&& it->second.m_obstacleType == ObstacleType::NONE
			&& toolType == PlayerTools::WATER)
		{
			it->second.m_farmState = FarmState::WATER;
			m_chunkUpdateManager.MarkChunkDirty(curChunk, DirtyType::DIRTY_FARMLAND, aimGridPos);
		}
	}
}

void Map::PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius)
{
    uint32_t gid= m_tileMap->GetTileGidFromLayerID(m_tileMap->m_markLayerId, tileCoords);
	uint32_t flag =m_game->g_tileManager->m_gidToTilePropertyFlag[gid];

	bool hasTerrainCollision = false; 
	bool hasObstacleCollision = false;

	hasTerrainCollision = flag & static_cast<uint32_t>(TerrainType::SOLID);

	TileChunk* curChunk= m_tileMap->m_markLayer->GetChunkContaining(tileCoords);
	uint64_t tileKey=m_tileMap->GetTileKey(tileCoords);
	if (curChunk && curChunk->m_keyToDynamicTileData.count(tileKey) > 0)
	{
		const DynamicTileData& tileData = curChunk->m_keyToDynamicTileData.at(tileKey);
		hasObstacleCollision = ((tileData.m_obstacleType != ObstacleType::NONE) && (tileData.m_obstacleType != ObstacleType::WEED));
	}

	if (hasObstacleCollision||hasTerrainCollision)
	{
		AABB2 thisTileBox = AABB2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y),
			static_cast<float>(tileCoords.x + 1.f), static_cast<float>(tileCoords.y + 1));
		PushDiscOutOfAABB2D(entityPos, entityPhyRadius, thisTileBox);
	}
}

void Map::TileMapRender() const
{
}
