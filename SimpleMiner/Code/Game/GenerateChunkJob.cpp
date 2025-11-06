#include "GenerateChunkJob.hpp"
#include "Game/Chunk.hpp"
#include "TerrainGenerator.hpp"

GenerateChunkJob::GenerateChunkJob(World* world, IntVec2 const& chunkCoords)
	: m_world(world)
	, m_chunkCoords(chunkCoords)
	, m_generatedChunk(nullptr)
{
}

GenerateChunkJob::~GenerateChunkJob()
{
}

void GenerateChunkJob::Execute()
{
	m_generatedChunk = new Chunk(m_chunkCoords);
	TerrainGenerator::GenerateBlocksForChunk_New(m_generatedChunk);
	m_generatedChunk->m_needsSaving = false;
}
