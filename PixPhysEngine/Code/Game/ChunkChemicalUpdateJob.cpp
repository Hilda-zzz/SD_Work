#include "ChunkChemicalUpdateJob.hpp"
#include "GameMap.hpp"
ChunkChemicalUpdateJob::ChunkChemicalUpdateJob(CellChunk* chunk, GameMap* map)
	: m_chunk(chunk)
	, m_map(map)
{
}

void ChunkChemicalUpdateJob::Execute()
{
	if (m_chunk)
	{
		m_map->UpdateSingleChunkChemical(m_chunk);
	}
}
