#include "TileChunk.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Tileset.hpp"
#include "TileMapManager.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "ObstacleDefinitions.hpp"
#include "Game.hpp"
#include "TileMap.hpp"
#include "GroundObstacle.hpp"
#include "RuledTileset.hpp"
#include "CropObject.hpp"

extern Renderer* g_theRenderer;

TileChunk::TileChunk()
{
	auto plowed = TileMapManager::GetInstance().m_loadedRuledTilesetsByName.find("FARM_SOIL");
	if (plowed != TileMapManager::GetInstance().m_loadedRuledTilesetsByName.end())
	{
		m_plowedSoilRuleSet = plowed->second;
	}

	auto watered = TileMapManager::GetInstance().m_loadedRuledTilesetsByName.find("FARM_WATER");
	if (watered != TileMapManager::GetInstance().m_loadedRuledTilesetsByName.end())
	{
		m_wateredSoilRuleSet= watered->second;
	}
}

TileChunk::~TileChunk()
{
	for (GroundObstacle* curObstacle : m_obstacleWithAnimation)
	{
		delete curObstacle;
		curObstacle = nullptr;
	}

	for (CropObject* curCrop : m_cropObjects)
	{
		delete curCrop;
		curCrop = nullptr;
	}
}

void TileChunk::Update(float deltaSeconds)
{
 	for (GroundObstacle* curObstacle : m_obstacleWithAnimation)
 	{
 		curObstacle->Update(deltaSeconds);
 	}

	for (CropObject* curCrop : m_cropObjects)
	{
		curCrop->Update(deltaSeconds);
	}
}

void TileChunk::InitializeChunkVerts()
{
	if (m_size.x != 0 && m_size.y != 0)
	{
		for (int i = 0; i < (int)m_terrianData.size(); i++)
		{
			IntVec2 tileGridPos = IntVec2(i % m_size.x, -(i / m_size.y)) + m_startPosition;
			AABB2 tileBox = AABB2(Vec2((float)tileGridPos.x, (float)tileGridPos.y), Vec2((float)tileGridPos.x, (float)tileGridPos.y) + Vec2::ONE);

			Tileset const* curSet=TileMapManager::GetInstance().FindTilesetByGid(m_terrianData[i]);
			AABB2 curTileUV=AABB2::ZERO_TO_ONE;
			if (curSet)
			{
				int innerSetIndex = m_terrianData[i] - curSet->GetFirstGid();
				curTileUV = curSet->GetTileUVByInnerIndex(innerSetIndex);
			}
			else
			{
				continue;
			}
			AddVertsForAABB2D(m_terrianVerts, tileBox, Rgba8::WHITE, curTileUV.m_mins, curTileUV.m_maxs,900.f);
		}
		for (const auto& [tilePosKey, tileData] : m_keyToDynamicTileData)
		{
			if (tileData.m_farmState == FarmState::UNPLOWED) continue;

			IntVec2 tileGridPos = TileMap::GetGridPosByTileKey(tilePosKey);
			AABB2 tileBox = AABB2(Vec2((float)tileGridPos.x, (float)tileGridPos.y), 
				Vec2((float)tileGridPos.x, (float)tileGridPos.y) + Vec2::ONE);
			if (tileData.m_farmState == FarmState::PLOWED)
			{
				AABB2 curUv;
				if (m_plowedSoilRuleSet)
				{
					uint8_t neighborMask=GetPlowedNeighborMask(tileGridPos);
					curUv= m_plowedSoilRuleSet->GetUvFromMask(neighborMask);
				}
				AddVertsForAABB2D(m_plowedFarmlandVerts, tileBox, Rgba8::WHITE, curUv.m_mins, curUv.m_maxs, 900.f);
			}
			//if (tileData.m_farmState == FarmState::WATER)
			if (tileData.m_isWater)
			{
				AABB2 curUv;
				if (m_plowedSoilRuleSet)
				{
					uint8_t neighborMask = GetPlowedNeighborMask(tileGridPos);
					curUv = m_plowedSoilRuleSet->GetUvFromMask(neighborMask);
				}

				AddVertsForAABB2D(m_plowedFarmlandVerts, tileBox, Rgba8::WHITE, curUv.m_mins, curUv.m_maxs, 900.f);

				AddVertsForAABB2D(m_wateredFarmlandVerts, tileBox, Rgba8::WHITE, curUv.m_mins, curUv.m_maxs, 900.f);
			}
		}
	}
}

void TileChunk::UpdateObstacleVerts()
{
	m_staticObstacleVerts.clear();

	for (const auto& [tilePosKey, tileData] : m_keyToDynamicTileData)
	{
		if (tileData.m_obstacleType == ObstacleType::NONE) continue;
		if (ObstacleDefinition::s_obstacleDefinitions[tileData.m_obstacleType]->m_isObject) continue;

		IntVec2 gridPos = TileMap::GetGridPosByTileKey(tilePosKey);
		ObstacleDefinition* curDef = ObstacleDefinition::s_obstacleDefinitions.at(tileData.m_obstacleType);
		if (curDef)
		{
			AABB2 spriteUV;
			if (tileData.m_spriteIndex > -1)
			{
				spriteUV = curDef->m_spriteSheet->GetSpriteUVs(tileData.m_spriteIndex);
			}

			Vec2 spriteSize = curDef->m_spriteSheet->GetEachSpriteWidthHeight();
			Vec2 spriteSizeInWorld = spriteSize / (float)RESOLUTION;

			Vec2 gridBottomCenter = Vec2((float)gridPos.x + 0.5f, (float)gridPos.y);
			Vec2 spriteBottomLeft = gridBottomCenter - Vec2(spriteSizeInWorld.x * 0.5f, 0.f);
			Vec2 spriteTopRight = gridBottomCenter + Vec2(spriteSizeInWorld.x * 0.5f, spriteSizeInWorld.y);

			Rgba8 renderColor = Rgba8::WHITE;

			AddVertsForAABB2D(m_staticObstacleVerts,
				AABB2(spriteBottomLeft, spriteTopRight),
				renderColor,
				spriteUV.m_mins, spriteUV.m_maxs,
				(float)gridPos.y + Z_OFFSET);
		}
		else
		{
			Vec2 gridBottomLeft = Vec2((float)gridPos.x, (float)gridPos.y);
			Vec2 gridTopRight = gridBottomLeft + Vec2(1.f, 1.f);

			Rgba8 renderColor = Rgba8::WHITE;

			AddVertsForAABB2D(m_staticObstacleVerts,
				AABB2(gridBottomLeft, gridTopRight),
				renderColor);
		}
	}
}

void TileChunk::UpdateFarmlandVerts()
{
	m_plowedFarmlandVerts.clear();
	m_wateredFarmlandVerts.clear();

	for (const auto& [tilePosKey, tileData] : m_keyToDynamicTileData)
	{
		if (tileData.m_farmState == FarmState::UNPLOWED) continue;

		IntVec2 gridPos = TileMap::GetGridPosByTileKey(tilePosKey);
		AABB2 tileBox = AABB2(Vec2((float)gridPos.x, (float)gridPos.y),
			Vec2((float)gridPos.x, (float)gridPos.y) + Vec2::ONE);

		if (tileData.m_farmState == FarmState::PLOWED)
		{
			//get uv
			AABB2 curUv;
			if (m_plowedSoilRuleSet)
			{
				uint8_t neighborMask = GetPlowedNeighborMask(gridPos);
				curUv = m_plowedSoilRuleSet->GetUvFromMask(neighborMask);
			}
			AddVertsForAABB2D(m_plowedFarmlandVerts, tileBox, Rgba8::WHITE, curUv.m_mins, curUv.m_maxs, 900.f);
		}
		//if (tileData.m_farmState == FarmState::WATER)
		if (tileData.m_isWater)
		{
			AABB2 plowedUv;
			if (m_plowedSoilRuleSet)
			{
				uint8_t plowedNeighborMask = GetPlowedNeighborMask(gridPos);
				plowedUv = m_plowedSoilRuleSet->GetUvFromMask(plowedNeighborMask);
			}
			AddVertsForAABB2D(m_plowedFarmlandVerts, tileBox, Rgba8::WHITE, plowedUv.m_mins, plowedUv.m_maxs, 900.f);

			AABB2 wateredUv;
			if (m_wateredSoilRuleSet)
			{
				uint8_t wateredNeighborMask = GetWateredNeighborMask(gridPos);
				wateredUv = m_wateredSoilRuleSet->GetUvFromMask(wateredNeighborMask);
			}
			AddVertsForAABB2D(m_wateredFarmlandVerts, tileBox, Rgba8::WHITE, wateredUv.m_mins, wateredUv.m_maxs, 900.f);
		}
	}
}

void TileChunk::RenderDynamicContent() const
{
	g_theRenderer->BindTexture(m_plowedSoilRuleSet->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(m_plowedFarmlandVerts);
	g_theRenderer->BindTexture(m_wateredSoilRuleSet->GetTexture());
	g_theRenderer->DrawVertexArray(m_wateredFarmlandVerts);

	g_theRenderer->BindTexture(ObstacleDefinition::s_obstacleTexture);;
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(m_staticObstacleVerts);

	for (GroundObstacle* curObstacle : m_obstacleWithAnimation)
	{
		curObstacle->Render();
	}

	for (CropObject* curCrop : m_cropObjects)
	{
		curCrop->Render();
	}
}

uint32_t TileChunk::GetTile(IntVec2 const& gridPos) const
{
	IntVec2 localPos = gridPos - m_startPosition;

	if (localPos.x < 0 || localPos.y > 0 ||
		localPos.x >= m_size.x || localPos.y <= -m_size.y) 
	{
		return 0;
	}

	int index = -localPos.y * m_size.x + localPos.x;
	return (index >= 0 && index < (int)m_terrianData.size()) ? m_terrianData[index] : 0;
}

// concern about the y +? -?
IntVec2 TileChunk::GetGridPos(int tileIndex) const
{
	if (tileIndex < 0 || tileIndex >= m_size.x * m_size.y)
	{
		return IntVec2(0, 0);
	}
	int relativeX = tileIndex % m_size.x;
	int relativeY = tileIndex / m_size.y;
	return IntVec2(relativeX, relativeY);
}

bool TileChunk::RemoveTheGroundObstacle(GroundObstacle* curObstacle)
{
	if (!curObstacle) 
	{
		return false;
	}

	bool removed = false;

	auto animIt = std::find(m_obstacleWithAnimation.begin(), m_obstacleWithAnimation.end(), curObstacle);
	if (animIt != m_obstacleWithAnimation.end()) {
		m_obstacleWithAnimation.erase(animIt);
		removed = true;
	}

	for (auto it = m_gridPosToGroundObstacle.begin(); it != m_gridPosToGroundObstacle.end(); ++it) {
		if (it->second == curObstacle) {
			m_gridPosToGroundObstacle.erase(it);
			removed = true;
			break;
		}
	}

	return removed;
}

void TileChunk::UpdateStateForNewDayInChunk()
{
	// clear all water ground data
	for (auto& [key, tileData] : m_keyToDynamicTileData) 
	{
		tileData.m_isWater = false;  
	}
	// clear water verts
	m_wateredFarmlandVerts.clear();
	// settle crop new day
	for (auto* crop : m_cropObjects)
	{
		if (crop) 
		{
			crop->SettleDailyState();  
		}
	}
}

uint8_t TileChunk::GetPlowedNeighborMask(IntVec2 const& tileGridPos)
{
	uint8_t mask = 0;

	IntVec2 directions[4] = {
		IntVec2(0, 1),
		IntVec2(1, 0),
		IntVec2(0, -1),
		IntVec2(-1, 0)
	};

	for (int i = 0; i < 4; ++i) 
	{
		IntVec2 neighborPos = tileGridPos + directions[i];

		uint64_t tileKey = TileMap::GetTileKey(neighborPos);
		auto neighborData = m_keyToDynamicTileData.find(tileKey);
		if (neighborData!=m_keyToDynamicTileData.end())
		{
			DynamicTileData data = neighborData->second;
			if (data.m_obstacleType == ObstacleType::NONE
				&&data.m_farmState!=FarmState::UNPLOWED)
			{
				mask |= (1 << i);
			}
		}
 		else
 		{
 			TileChunk* neighborChunk= m_parentLayer->GetChunkContaining(neighborPos);
 			if (neighborChunk )//&& neighborChunk!=this
 			{
 				auto neighborTileData = neighborChunk->m_keyToDynamicTileData.find(tileKey);
 				if (neighborTileData != neighborChunk->m_keyToDynamicTileData.end())
 				{
 					DynamicTileData data = neighborTileData->second;
 					if (data.m_obstacleType == ObstacleType::NONE
 						&& data.m_farmState != FarmState::UNPLOWED)
 					{
 						mask |= (1 << i);
 					}
 				}
 			}
 		}
	}
	return mask;
}

uint8_t TileChunk::GetWateredNeighborMask(IntVec2 const& tileGridPos)
{
	uint8_t mask = 0;

	IntVec2 directions[4] = {
		IntVec2(0, 1),
		IntVec2(1, 0),
		IntVec2(0, -1),
		IntVec2(-1, 0)
	};

	for (int i = 0; i < 4; ++i)
	{
		IntVec2 neighborPos = tileGridPos + directions[i];

		uint64_t tileKey = TileMap::GetTileKey(neighborPos);
		auto neighborData = m_keyToDynamicTileData.find(tileKey);
		if (neighborData != m_keyToDynamicTileData.end())
		{
			DynamicTileData data = neighborData->second;
			if (data.m_obstacleType == ObstacleType::NONE
				&& data.m_isWater)
			{
				mask |= (1 << i);
			}
		}
		else
		{
			TileChunk* neighborChunk = m_parentLayer->GetChunkContaining(neighborPos);
			if (neighborChunk)
			{
				auto neighborTileData = neighborChunk->m_keyToDynamicTileData.find(tileKey);
				if (neighborTileData != neighborChunk->m_keyToDynamicTileData.end())
				{
					DynamicTileData data = neighborTileData->second;
					if (data.m_obstacleType == ObstacleType::NONE
						&& data.m_isWater)
					{
						mask |= (1 << i);
					}
				}
			}
		}
	}
	return mask;
}
