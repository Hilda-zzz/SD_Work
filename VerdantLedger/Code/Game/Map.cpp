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
#include "Game/Inventory.hpp"
#include "CropObject.hpp"
#include "Game/InventoryItemDef.hpp"
#include "CropDefinitions.hpp"

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
	UpdateCamFollow();

	IntVec2 m_playerFrontGridPos = GetTileCoordsFromPoint(m_player->m_position)+IntVec2(0,-1);
 	m_tileMap->UpdateTransparentObject(m_playerFrontGridPos);
 	m_tileMap->Update(deltaSeconds);

	//m_chunkUpdateManager.UpdateVisibleChunks(deltaSeconds,camCenter,camSize);
	UpdateVisibleChunk(deltaSeconds);
	m_chunkUpdateManager.UpdateDirtyChunks();
}

void Map::Render() const
{
	g_theRenderer->BeginCamera(m_gameplayCam);

	m_player->Render();

	// transparent things must be rendered at the end
	m_tileMap->Render(m_visibleChunk);

	g_theRenderer->EndCamera(m_gameplayCam);
}

void Map::UpdateCamFollow()
{
	Vec2 idealCamPos = m_player->m_position;
	float minCamX = g_halfGameCamDimensions.x;  // left bound
	float maxCamX = 110.f - g_halfGameCamDimensions.x;  // right bound
	float minCamY = -63.f + g_halfGameCamDimensions.y;  // bot bound
	float maxCamY = -g_halfGameCamDimensions.y;   // top bound

	Vec2 clampedCamPos;
	clampedCamPos.x = GetClamped(idealCamPos.x, minCamX, maxCamX);
	clampedCamPos.y = GetClamped(idealCamPos.y, minCamY, maxCamY);

	m_gameplayCam.SetOrthographicView(
		clampedCamPos - g_halfGameCamDimensions,
		clampedCamPos + g_halfGameCamDimensions,
		0.f,
		1000.f
	);
}

void Map::UpdateVisibleChunk(float deltaSeconds)
{
	m_visibleChunk.clear();

	if (!m_tileMap->m_markLayer) return;

	//Vec2 camSize = m_gameplayCam.GetOrthoTopRight() - m_gameplayCam.GetOrthoBottomLeft();
	Vec2 camSize = Vec2(20.f, 10.f);
	Vec2 camCenter = (m_gameplayCam.GetOrthoTopRight() + m_gameplayCam.GetOrthoBottomLeft()) * 0.5f;

	float halfWidth = camSize.x * 0.5f;
	float halfHeight = camSize.y * 0.5f;

	Vec2 cameraMin = Vec2(camCenter.x - halfWidth, camCenter.y - halfHeight);
	Vec2 cameraMax = Vec2(camCenter.x + halfWidth, camCenter.y + halfHeight);

	IntVec2 minChunkPos = IntVec2(
		(int)floor(cameraMin.x / 16.0f) * 16,
		(int)ceil(cameraMax.y / 16.0f) * 16
	);
	IntVec2 maxChunkPos = IntVec2(
		(int)ceil(cameraMax.x / 16.0f) * 16,
		(int)floor(cameraMin.y / 16.0f) * 16
	);

	for (int chunkY = minChunkPos.y; chunkY >= maxChunkPos.y; chunkY -= 16)
	{
		for (int chunkX = minChunkPos.x; chunkX <= maxChunkPos.x; chunkX += 16)
		{
			IntVec2 chunkStartPos(chunkX, chunkY);
			m_visibleChunk.push_back(chunkStartPos);
			uint64_t chunkKey = TileLayer::GetChunkKey(chunkStartPos);

			auto it = m_tileMap->m_markLayer->m_chunkIndexMap.find(chunkKey);
			if (it != m_tileMap->m_markLayer->m_chunkIndexMap.end())
			{
				size_t chunkIndex = it->second;
				TileChunk& chunk = m_tileMap->m_markLayer->m_chunks[chunkIndex];
				chunk.Update(deltaSeconds);
			}
		}
	}
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

void Map::UsingToolTowardsGridPos(IntVec2 const& aimGridPos, PlayerTools toolType, InventorySlotButton& curInventoryBtn)
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
			//it->second.m_farmState = FarmState::WATER;
			it->second.m_isWater = true;
			m_chunkUpdateManager.MarkChunkDirty(curChunk, DirtyType::DIRTY_FARMLAND, aimGridPos);
		}

		// Planting
		if ((it->second.m_farmState == FarmState::PLOWED)
			&& it->second.m_obstacleType == ObstacleType::NONE
			&& toolType == PlayerTools::SEEDS&&!it->second.m_isPlanted)
		{
			if (curInventoryBtn.GetItem()->m_quantity > 0)
			{
				// add a crop object
				Strings cropSeedName = SplitStringOnDelimiter(curInventoryBtn.GetItem()->m_itemDef->m_name, '_');
				std::string cropName = cropSeedName[0];
				CropDefinitions* curCropDef = CropDefinitions::s_cropDefinitions[cropName];
				CropObject* curCropObject = new CropObject(curCropDef, aimGridPos);
				curChunk->m_cropObjects.push_back(curCropObject);
				curChunk->m_gridPosToCropObject[tileKey] = curCropObject;

				// Consume from inventory
				m_player->m_inventory->RemoveItem(curInventoryBtn.GetItem()->m_itemDef, 1);

				// Mark the chunk tile type to planted
				it->second.m_isPlanted = true;

				// Update Panel
				m_game->UpdateToolBarFromInventory();
			}
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
