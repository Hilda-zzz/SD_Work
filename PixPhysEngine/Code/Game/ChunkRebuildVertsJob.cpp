#include "ChunkRebuildVertsJob.hpp"
#include "CellChunk.hpp"

ChunkRebuildVertsJob::ChunkRebuildVertsJob(CellChunk* chunk)
	: m_chunk(chunk)
{
}

void ChunkRebuildVertsJob::Execute()
{
	if (m_chunk)
	{
		m_chunk->RebuildVertexWithNewColor();
	}
}
