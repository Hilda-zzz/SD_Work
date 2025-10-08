#include "LoadChunkJob.hpp"
#include "Game/Chunk.hpp"
#include "ChunkFileIO.hpp"

LoadChunkJob::LoadChunkJob(World* world, IntVec2 chunkCoords, std::string filename)
	: m_world(world)
	, m_chunkCoords(chunkCoords)
	, m_filename(filename)
	, m_chunk(nullptr)
{
}

void LoadChunkJob::Execute()
{
	m_chunk = new Chunk(m_chunkCoords);
	m_chunk->m_state = ChunkState::ACTIVATING_LOADING;

	ChunkFileIO::LoadChunk("Saves/", m_filename, m_chunk);

	m_chunk->m_state = ChunkState::ACTIVATING_LOAD_COMPLETE;
}
