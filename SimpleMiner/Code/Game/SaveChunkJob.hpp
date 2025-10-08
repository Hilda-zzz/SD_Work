#pragma once
#include "Engine/JobSystem/Job.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <string>

class World;
class Chunk;

class SaveChunkJob : public Job
{
public:
	SaveChunkJob(World* world, Chunk* chunk, std::string filename);
	virtual ~SaveChunkJob();

	virtual void Execute() override;

	IntVec2 GetChunkCoords() const { return m_chunkCoords; }
	Chunk* GetChunk() { return m_chunk; }
	bool WasSaveSuccessful() const { return m_saveSuccess; }

private:
	World* m_world;
	Chunk* m_chunk;
	IntVec2 m_chunkCoords;
	std::string m_filename;
	bool m_saveSuccess;
};