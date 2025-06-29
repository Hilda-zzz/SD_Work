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

extern Renderer* g_theRenderer;

TileChunk::TileChunk()
{
}

TileChunk::~TileChunk()
{
}

void TileChunk::Update()
{
	for (GroundObstacle* curObstacle : m_obstacleWithAnimation)
	{
		curObstacle->Update();
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
			// get uv according to index
	
			// the index in whitch tsx?

			// get int vec pos for the index

			// get uv from sprite sheet

// 			if (i % 2 == 0)
// 			{
// 				AddVertsForAABB2D(m_terrianVerts, tileBox, Rgba8::WHITE,curTileUV.m_mins,curTileUV.m_maxs);
// 			}
// 			else
// 			{
// 				
// 			}
			AddVertsForAABB2D(m_terrianVerts, tileBox, Rgba8::WHITE, curTileUV.m_mins, curTileUV.m_maxs,900.f);
			
		}
	}

}

void TileChunk::UpdateObstacleVerts()
{
	m_dynamicVerts.clear();
	m_dynamicVertsTransparent.clear();

	for (const auto& [tilePosKey, tileData] : m_dynamicTiles)
	{
		if (tileData.m_obstacleType == ObstacleType::NONE) continue;

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
			if (tileData.m_isTransparent)
			{
				AddVertsForAABB2D(m_dynamicVertsTransparent,
					AABB2(spriteBottomLeft, spriteTopRight),
					Rgba8(255,255,255,tileData.m_alpha),
					spriteUV.m_mins, spriteUV.m_maxs,
					(float)gridPos.y + Z_OFFSET);
					continue;
			}

			AddVertsForAABB2D(m_dynamicVerts,
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
			if (tileData.m_isTransparent)
			{
				renderColor.a = static_cast<unsigned char>(tileData.m_alpha * 255.0f);
			}

			AddVertsForAABB2D(m_dynamicVerts,
				AABB2(gridBottomLeft, gridTopRight),
				renderColor);
		}
	}

	m_isDirty = false;
}

void TileChunk::RenderDynamicContent() const
{
	g_theRenderer->BindTexture(ObstacleDefinition::s_obstacleTexture);;
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(m_dynamicVerts);

	for (GroundObstacle* curObstacle : m_obstacleWithAnimation)
	{
		curObstacle->Render();
	}
//  g_theRenderer->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
// 	g_theRenderer->DrawVertexArray(m_dynamicVertsTransparent);
// 	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
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
