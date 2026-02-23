#include "ChunkPhysUpdateJob.hpp"
#include "GameMap.hpp"

ChunkPhysUpdateJob::ChunkPhysUpdateJob(CellChunk* chunk, GameMap* map)
	: m_chunk(chunk)
	, m_map(map)
{
}

void ChunkPhysUpdateJob::Execute()
{
	if (m_chunk && m_chunk->IsDirty())
	{
		m_map->UpdateSingleThreadChunk(m_chunk);
	}
}
