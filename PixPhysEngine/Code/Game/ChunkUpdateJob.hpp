#pragma once
#include "Engine/JobSystem/Job.hpp"

class CellChunk;
class SandboxMap;

class ChunkUpdateJob :public Job
{
public:
	ChunkUpdateJob(CellChunk* chunk, SandboxMap* map);

	virtual void Execute() override;

private:
	CellChunk* m_chunk;
	SandboxMap* m_map;
};