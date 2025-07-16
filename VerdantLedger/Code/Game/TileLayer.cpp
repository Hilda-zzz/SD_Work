#include "TileLayer.hpp"
#include "Game.hpp"

TileLayer::TileLayer()
{
}

TileLayer::~TileLayer()
{
}

void TileLayer::AddChunk(TileChunk const& chunk)
{
	m_chunks.push_back(chunk);
	uint64_t chunkKey = GetChunkKey(chunk.m_startPosition);
	m_chunkIndexMap[chunkKey] = m_chunks.size()-1;
}

TileChunk* TileLayer::GetChunkContaining(IntVec2 tileCoords)
{
// 	IntVec2 expectedChunkPos;
// 	expectedChunkPos.x = (tileCoords.x / CHUNK_SIZE) * CHUNK_SIZE;
// 	expectedChunkPos.y = ((tileCoords.y / CHUNK_SIZE) + 1) * CHUNK_SIZE;
// 	if (tileCoords.x < 0) expectedChunkPos.x -= CHUNK_SIZE;
// 	if (tileCoords.y < 0) expectedChunkPos.y -= CHUNK_SIZE;
// 	uint64_t chunkKey = GetChunkKey(expectedChunkPos);
// 	
// 	auto it = m_chunkIndexMap.find(chunkKey);
// 	return (it != m_chunkIndexMap.end()) ? &m_chunks[it->second] : nullptr;
	IntVec2 expectedChunkPos;

	expectedChunkPos.x = (tileCoords.x >= 0) ?
		(tileCoords.x / CHUNK_SIZE) * CHUNK_SIZE :
		((tileCoords.x - CHUNK_SIZE + 1) / CHUNK_SIZE) * CHUNK_SIZE;

	if (tileCoords.y == 0)
	{
		expectedChunkPos.y = 0;
	}
	else if (tileCoords.y > 0)
	{
		expectedChunkPos.y = ((tileCoords.y / CHUNK_SIZE) + 1) * CHUNK_SIZE;
	}
	else if (tileCoords.y <  0)
	{
		expectedChunkPos.y = (tileCoords.y / CHUNK_SIZE)* CHUNK_SIZE;
	}
// 	expectedChunkPos.y = (tileCoords.y > 0) ?
// 		((tileCoords.y / CHUNK_SIZE) + 1) * CHUNK_SIZE :
// 		((tileCoords.y + 1) / CHUNK_SIZE) * CHUNK_SIZE;

	uint64_t chunkKey = GetChunkKey(expectedChunkPos);
	auto it = m_chunkIndexMap.find(chunkKey);
	return (it != m_chunkIndexMap.end()) ? &m_chunks[it->second] : nullptr;
}

TileChunk* TileLayer::GetChunk(int chunkX, int chunkY)
{
	uint64_t key = GetChunkKey(IntVec2(chunkX, chunkY));
	auto it = m_chunkIndexMap.find(key);
	if (it != m_chunkIndexMap.end())
	{
		int index = (int)it->second;
		return &m_chunks[index];
	}
	return nullptr;
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

uint64_t TileLayer::GetChunkKey(IntVec2 chunkPos)
{
	uint32_t x_bits = *reinterpret_cast<const uint32_t*>(&chunkPos.x);
	uint32_t y_bits = *reinterpret_cast<const uint32_t*>(&chunkPos.y);
	return (static_cast<uint64_t>(x_bits) << 32) | y_bits;
}
