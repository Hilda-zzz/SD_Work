#pragma once
#include "Engine/JobSystem/Job.hpp"

class CellChunk;
class GameMap;

class ChunkRebuildVertsJob :public Job
{
public:
	ChunkRebuildVertsJob(CellChunk* chunk);

	virtual void Execute() override;

private:
	CellChunk* m_chunk;
};