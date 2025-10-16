#include "ChunkUpdateJob.hpp"
#include "Game/CellChunk.hpp"
#include "Game/SandboxMap.hpp"

ChunkUpdateJob::ChunkUpdateJob(CellChunk* chunk, SandboxMap* map)
	: m_chunk(chunk)
	, m_map(map)
{
}

void ChunkUpdateJob::Execute()
{
	if (m_chunk && m_chunk->IsDirty())
	{
		m_map->UpdateSingleChunk(m_chunk);
	}
}
