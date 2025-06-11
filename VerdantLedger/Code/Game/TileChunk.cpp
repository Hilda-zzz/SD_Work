#include "TileChunk.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"

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

			if (i % 2 == 0)
			{
				AddVertsForAABB2D(m_verts, tileBox, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE);
			}
			else
			{
				AddVertsForAABB2D(m_verts, tileBox, Rgba8::BLACK, Vec2::ZERO, Vec2::ONE);
			}
			
		}
	}

}
