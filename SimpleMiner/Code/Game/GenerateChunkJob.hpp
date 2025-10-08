#pragma once
#include "Engine/JobSystem/Job.hpp"
#include <Engine/Math/IntVec2.hpp>

class World;
class Chunk;

class GenerateChunkJob: public Job
{
public:
	GenerateChunkJob(World* world, IntVec2 const& chunkCoords);
	virtual ~GenerateChunkJob() override;

	virtual void Execute() override;

	Chunk* GetChunk() { return m_generatedChunk; }
	IntVec2 GetChunkCoords() const { return m_chunkCoords; }
private:
	World* m_world=nullptr;
	IntVec2 m_chunkCoords;
	Chunk* m_generatedChunk=nullptr;
};