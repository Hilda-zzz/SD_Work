#include "SaveChunkJob.hpp"
#include "Game/Chunk.hpp"
#include "ChunkFileIO.hpp"

SaveChunkJob::SaveChunkJob(World* world, Chunk* chunk, std::string filename)
	: m_world(world)
	, m_chunk(chunk)
	, m_chunkCoords(chunk->m_chunkCoords)
	, m_filename(filename)
	, m_saveSuccess(false)
{
}

SaveChunkJob::~SaveChunkJob()
{
}

void SaveChunkJob::Execute()
{
	m_chunk->m_state = ChunkState::DEACTIVATING_SAVING;

	m_saveSuccess = ChunkFileIO::SaveChunk("Saves/",m_filename, m_chunk);

	if (m_saveSuccess)
	{
		m_chunk->m_state = ChunkState::DEACTIVATING_SAVE_COMPLETE;
		m_chunk->m_needsSaving = false;

		//g_theDevConsole->AddLine(DevConsole::INFO_MINOR,
		//	Stringf("Saved chunk (%d,%d) to disk",
		//		m_chunkCoords.x, m_chunkCoords.y));
	}
	else
	{
		// 保存失败
		m_chunk->m_state = ChunkState::DEACTIVATING_SAVE_COMPLETE;  // 仍然标记完成

		//g_theDevConsole->AddLine(DevConsole::ERROR_SEVERE,
		//	Stringf("Failed to save chunk (%d,%d)",
		//		m_chunkCoords.x, m_chunkCoords.y));
	}
}
