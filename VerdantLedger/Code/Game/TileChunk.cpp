#include "TileChunk.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Tileset.hpp"
#include "TileMapManager.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "ObstacleDefinitions.hpp"

extern Renderer* g_theRenderer;

TileChunk::TileChunk()
{
}

TileChunk::~TileChunk()
{
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

			if (i % 2 == 0)
			{
				AddVertsForAABB2D(m_terrianVerts, tileBox, Rgba8::WHITE,curTileUV.m_mins,curTileUV.m_maxs);
			}
			else
			{
				AddVertsForAABB2D(m_terrianVerts, tileBox, Rgba8::WHITE, curTileUV.m_mins, curTileUV.m_maxs);
			}
			
		}
	}

}

void TileChunk::RenderDynamicContent() const
{
	g_theRenderer->BindTexture(ObstacleDefinition::s_obstacleTexture);;
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(m_dynamicVerts);
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
