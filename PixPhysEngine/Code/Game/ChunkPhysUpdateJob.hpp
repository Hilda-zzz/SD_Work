#pragma once
#include "Engine/JobSystem/Job.hpp"

class CellChunk;
class GameMap;

class ChunkPhysUpdateJob :public Job
{
public:
	ChunkPhysUpdateJob(CellChunk* chunk, GameMap* map);

	virtual void Execute() override;

private:
	CellChunk* m_chunk;
	GameMap* m_map;
};