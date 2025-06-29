#include "DynamicContentGenerator.hpp"
#include "Game/TileMap.hpp"
#include "TileMapManager.hpp"
#include "Game/TileChunk.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "ObstacleDefinitions.hpp"
#include "Game.hpp"
#include "GroundObstacle.hpp"

extern RandomNumberGenerator* g_randomNumGenerator;

void DynamicContentGenerator::GenerateAllDynamicContentForTheMap(TileMap* curMap)
{
// 	curMap->m_dynamicTiles.clear();
// 	curMap->ForEachTileInLayer(
// 		curMap->m_markLayerIndex,
// 		[this, curMap](IntVec2 gridPos, uint32_t gid) 
// 		{
// 			if (gid == 0) 
// 				return;
// 
// 			auto it = TileMapManager::GetInstance().m_gidToTilePropertyFlag.find(gid);
// 			if (it == TileMapManager::GetInstance().m_gidToTilePropertyFlag.end()) 
// 			{
// 				return;
// 			}
// 
// 			uint32_t flag = it->second;
// 			if (flag & static_cast<uint32_t>(TerrainType::FARMABLE)) 
// 			{
// 				GenerateObstacleAtPosition(curMap, gridPos);
// 			}
// 		}
// 	);
	TileLayer* markLayer = curMap->FindLayerById(curMap->m_markLayerId);
	for (int i=0;i<markLayer->m_chunks.size();i++)
	{
		GenerateDynamicContentForEachChunk(&markLayer->m_chunks[i]);
	}
}

void DynamicContentGenerator::GenerateDynamicContentForEachChunk(TileChunk* curChunk)
{
	curChunk->m_obstacleWithAnimation.reserve(200);
	curChunk->m_dynamicTiles.clear();
	curChunk->ForEachTileInChunk(
		[this, curChunk](IntVec2 gridPos, uint32_t gid)
		{
			if (gid == 0)
				return;

			auto it = TileMapManager::GetInstance().m_gidToTilePropertyFlag.find(gid);
			if (it == TileMapManager::GetInstance().m_gidToTilePropertyFlag.end())
			{
				return;
			}

			uint32_t flag = it->second;
			if (flag & static_cast<uint32_t>(TerrainType::FARMABLE))
			{
				GenerateObstacleAtPosition(curChunk, gridPos);
			}
		}
	);
}

// void DynamicContentGenerator::GenerateObstacleAtPosition(TileMap* curMap, IntVec2 const& gridPos)
// {
// // 	uint32_t gid = curMap->GetTileGidFromLayerID(curMap->m_markLayerIndex, gridPos);
// // 	uint32_t flag = TileMapManager::GetInstance().m_gidToTilePropertyFlag[gid];
// // 	if (flag & static_cast<uint32_t>(TerrainType::FARMABLE))
// // 	{
// 	 	// save dynamic data for each tile grid
// 	 	uint64_t tilePosKey = curMap->GetTileKey(gridPos);
// 	 	DynamicTileData curDynamicTileData;
// 	 	float possiblity = curMap->m_rng.RollRandomFloatZeroToOne();
// 	 	if (possiblity > 0.8f)
// 	 	{
// 	 		curDynamicTileData.m_obstacleType = ObstacleType::ROCK;
// 	 		curDynamicTileData.m_curObstacleDurability = 4;
// 	 		//GroundObstacle* newRock =new GroundObstacle(ObstacleType::ROCK, gridPos);
// 	 		curMap->m_obstacles.push_back(newRock);
// 	 	}
// 	 	curMap->m_dynamicTiles[tilePosKey] = curDynamicTileData;
// 	//}
// }

void DynamicContentGenerator::GenerateObstacleAtPosition(TileChunk* curChunk, IntVec2 const& gridPos)
{
	uint64_t tilePosKey = TileMap::GetTileKey(gridPos);
	DynamicTileData curDynamicTileData;
	float possibility =g_randomNumGenerator->RollRandomFloatZeroToOne();

	if (possibility <= 0.7f)
	{
		curDynamicTileData.m_obstacleType = ObstacleType::NONE;
	}
 	else if (possibility <= 0.8f)
 	{
 		if (gridPos.y == 0)
 		{
 			int i = 0;
 		}
 		GenerateSpecificObstacle(curChunk, gridPos, tilePosKey, ObstacleType::TREE);
 	}
	else if (possibility <= 0.9f)
	{
		if (gridPos.y == 0)
		{
			int i = 0;
		}
		GenerateSpecificObstacle(curChunk, gridPos, tilePosKey, ObstacleType::ROCK);
	}
 	else if (possibility <= 0.95f)
 	{
		if (gridPos.y == 0)
		{
			int i = 0;
		}
		GenerateSpecificObstacle(curChunk, gridPos, tilePosKey, ObstacleType::WEED);
 	}
}

void DynamicContentGenerator::GenerateSpecificObstacle(TileChunk* curChunk, IntVec2 const& gridPos, uint64_t tilePosKey, ObstacleType obstacleType)
{
	DynamicTileData curDynamicTileData;
	curDynamicTileData.m_obstacleType = obstacleType;
	ObstacleDefinition* curDef = ObstacleDefinition::s_obstacleDefinitions.at(obstacleType);
	if (curDef)
	{
		curDynamicTileData.m_curObstacleDurability = curDef->m_maxDurability;

		int spriteVariantIndex = g_randomNumGenerator->RollRandomIntInRange(0, curDef->m_spriteGridPos.size() - 1);
		IntVec2 spriteGridPos = curDef->m_spriteGridPos[spriteVariantIndex];
		int spriteIndex = curDef->m_spriteSheet->GetSpriteIndexFromGridPos(spriteGridPos);
		curDynamicTileData.m_spriteIndex = spriteIndex;
		AABB2 spriteUV;
		if (spriteIndex > -1)
		{
			spriteUV = curDef->m_spriteSheet->GetSpriteUVs(spriteIndex);
		}

		curChunk->m_dynamicTiles[tilePosKey] = curDynamicTileData;

		if (curDef->m_isObject)
		{
			GroundObstacle* groundObstacle = new GroundObstacle(obstacleType,curDef, gridPos,spriteIndex);
			curChunk->m_obstacleWithAnimation.push_back(groundObstacle);
			curChunk->m_gridPosToGroundObstacle[tilePosKey] = groundObstacle;
		}
		else
		{
			Vec2 spriteSize = curDef->m_spriteSheet->GetEachSpriteWidthHeight();
			Vec2 spriteSizeInWorld = spriteSize / (float)RESOLUTION;

			Vec2 gridBottomCenter = Vec2((float)gridPos.x + 0.5f, (float)gridPos.y);
			Vec2 spriteBottomLeft = gridBottomCenter - Vec2(spriteSizeInWorld.x * 0.5f, 0.f);
			Vec2 spriteTopRight = gridBottomCenter + Vec2(spriteSizeInWorld.x * 0.5f, spriteSizeInWorld.y);

			AddVertsForAABB2D(curChunk->m_dynamicVerts,
				AABB2(spriteBottomLeft, spriteTopRight),
				Rgba8(255, 255, 255, 255), spriteUV.m_mins, spriteUV.m_maxs, (float)gridPos.y + Z_OFFSET);
		}
	}
	else
	{
		curDynamicTileData.m_curObstacleDurability = 1;
		curChunk->m_dynamicTiles[tilePosKey] = curDynamicTileData;

		AddVertsForAABB2D(curChunk->m_dynamicVerts,
			AABB2(Vec2((float)gridPos.x, (float)gridPos.y), Vec2((float)gridPos.x, (float)gridPos.y) + Vec2(1.f, 1.f)),
			Rgba8::WHITE);
	}
}
