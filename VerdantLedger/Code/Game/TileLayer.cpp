#include "TileLayer.hpp"

TileLayer::TileLayer()
{
}

TileLayer::~TileLayer()
{
}

uint32_t TileLayer::GetGidFromGridPos(IntVec2 const& gridPos)
{
	for (TileChunk const& chunk : m_chunks) 
	{
		if (IsPositionInChunk(gridPos, chunk))
		{
			uint32_t gid = chunk.GetTile(gridPos);
			return gid;  
		}
	}
	return 0;
}

bool TileLayer::IsPositionInChunk(IntVec2 const& gridPos, const TileChunk& chunk) const
{
	IntVec2 localPos = gridPos - chunk.GetPosition();
	IntVec2 chunkSize = chunk.GetSize();

	return (localPos.x >= 0 && localPos.y <= 0 &&
		localPos.x < chunkSize.x && localPos.y > -chunkSize.y);
}
