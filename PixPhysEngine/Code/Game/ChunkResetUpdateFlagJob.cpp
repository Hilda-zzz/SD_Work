#include "ChunkResetUpdateFlagJob.hpp"
#include "CellChunk.hpp"
ChunkResetUpdateFlagJob::ChunkResetUpdateFlagJob(CellChunk* chunk, GameMap* map)
	: m_chunk(chunk)
{
}

void ChunkResetUpdateFlagJob::Execute()
{
	if (m_chunk)
		m_chunk->ResetUpdateFlags();
}
