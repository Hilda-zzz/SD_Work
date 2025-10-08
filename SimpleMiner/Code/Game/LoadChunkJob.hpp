#pragma once
#include "Engine/JobSystem/Job.hpp"
#include <string>
#include <Engine/Math/IntVec2.hpp>

class World;
class Chunk;

class LoadChunkJob : public Job
{
public:
	LoadChunkJob(World* world, IntVec2 chunkCoords, std::string filename);

	virtual void Execute() override;

	IntVec2 GetChunkCoords() const { return m_chunkCoords; }
	Chunk* GetChunk() { return m_chunk; }

private:
	World* m_world;
	IntVec2 m_chunkCoords;
	std::string m_filename;
	Chunk* m_chunk;
};