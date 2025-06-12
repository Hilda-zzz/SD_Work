#include "TileLayer.hpp"

TileLayer::TileLayer()
{
}

TileLayer::~TileLayer()
{
}

uint32_t TileLayer::GetGidFromGridPos(IntVec2 const& gridPos)
{
	for (const auto& chunk : m_chunks) 
	{
		for (const auto& chunk : m_chunks)
		{
			// 使用你已经正确实现的GetTile方法
			
			if (IsPositionInChunk(gridPos, chunk))
			{
				uint32_t gid = chunk.GetTile(gridPos);
				return gid;  // 找到了对应的chunk
			}
		}
	}
	return 0;
}

bool TileLayer::IsPositionInChunk(IntVec2 const& gridPos, const TileChunk& chunk) const
{
	IntVec2 localPos = gridPos - chunk.GetPosition();
	IntVec2 chunkSize = chunk.GetSize();

	return (localPos.x >= 0 && localPos.y <= 0 &&  // 注意：y <= 0
		localPos.x < chunkSize.x && localPos.y > -chunkSize.y);
}
