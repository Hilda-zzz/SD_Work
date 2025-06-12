#include "TileChunk.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Tileset.hpp"
#include "TileMapManager.hpp"

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
		for (int i = 0; i < m_data.size(); i++)
		{
			IntVec2 tileGridPos = IntVec2(i % m_size.x, -(i / m_size.y)) + m_startPosition;
			AABB2 tileBox = AABB2(Vec2((float)tileGridPos.x, (float)tileGridPos.y), Vec2((float)tileGridPos.x, (float)tileGridPos.y) + Vec2::ONE);

			Tileset const* curSet=TileMapManager::GetInstance().FindTilesetByGid(m_data[i]);
			AABB2 curTileUV=AABB2::ZERO_TO_ONE;
			if (curSet)
			{
				int innerSetIndex = m_data[i] - curSet->GetFirstGid();
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
				AddVertsForAABB2D(m_verts, tileBox, Rgba8::WHITE,curTileUV.m_mins,curTileUV.m_maxs);
			}
			else
			{
				AddVertsForAABB2D(m_verts, tileBox, Rgba8::WHITE, curTileUV.m_mins, curTileUV.m_maxs);
			}
			
		}
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
	return (index >= 0 && index < m_data.size()) ? m_data[index] : 0;
}
