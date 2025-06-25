#include "DynamicContentGenerator.hpp"
#include "Game/TileMap.hpp"
#include "TileMapManager.hpp"
#include "GroundObstacle.hpp"
void DynamicContentGenerator::GenerateAllDynamicContentForTheMap(TileMap* curMap)
{
	curMap->m_dynamicTiles.clear();
// 	for (int i = 0; i < curMap->GetSize().x; i++)
// 	{
// 		for (int j = 0; j < curMap->GetSize().y; j++)
// 		{
// 			uint32_t gid = curMap->GetTileGidFromLayerID(curMap->m_markLayerIndex, IntVec2(i,j));
// 			uint32_t flag = TileMapManager::GetInstance().m_gidToTilePropertyFlag[gid];
// 			if (flag & static_cast<uint32_t>(TerrainType::FARMABLE))
// 			{
// 				// save dynamic data for each tile grid
// 				uint64_t tilePosKey = curMap->GetTileKey(IntVec2(i, j));
// 				DynamicTileData curDynamicTileData;
// 				float possiblity = curMap->m_rng.RollRandomFloatZeroToOne();
// 				if (possiblity > 0.4f)
// 				{
// 				 	curDynamicTileData.m_obstacleType = ObstacleType::ROCK;
// 					curDynamicTileData.m_curObstacleDurability = 4;
// 					GroundObstacle* newRock =new GroundObstacle(ObstacleType::ROCK, IntVec2(i, j));
// 					curMap->m_obstacles.push_back(newRock);
// 				}
// 				curMap->m_dynamicTiles[tilePosKey] = curDynamicTileData;
// 			}
// 		}
// 	}
	curMap->ForEachTileInLayer(
		curMap->m_markLayerIndex,
		[this, curMap](IntVec2 gridPos, uint32_t gid) 
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
				GenerateObstacleAtPosition(curMap, gridPos);
			}
		}
	);
}

void DynamicContentGenerator::GenerateObstacleAtPosition(TileMap* curMap, IntVec2 const& gridPos)
{
// 	uint32_t gid = curMap->GetTileGidFromLayerID(curMap->m_markLayerIndex, gridPos);
// 	uint32_t flag = TileMapManager::GetInstance().m_gidToTilePropertyFlag[gid];
// 	if (flag & static_cast<uint32_t>(TerrainType::FARMABLE))
// 	{
	 	// save dynamic data for each tile grid
	 	uint64_t tilePosKey = curMap->GetTileKey(gridPos);
	 	DynamicTileData curDynamicTileData;
	 	float possiblity = curMap->m_rng.RollRandomFloatZeroToOne();
	 	if (possiblity > 0.8f)
	 	{
	 		curDynamicTileData.m_obstacleType = ObstacleType::ROCK;
	 		curDynamicTileData.m_curObstacleDurability = 4;
	 		GroundObstacle* newRock =new GroundObstacle(ObstacleType::ROCK, gridPos);
	 		curMap->m_obstacles.push_back(newRock);
	 	}
	 	curMap->m_dynamicTiles[tilePosKey] = curDynamicTileData;
	//}
}
