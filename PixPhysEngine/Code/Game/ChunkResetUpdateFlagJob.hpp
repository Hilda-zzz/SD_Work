
#pragma once
#include "Engine/JobSystem/Job.hpp"

class CellChunk;
class GameMap;

class ChunkResetUpdateFlagJob :public Job
{
public:
	ChunkResetUpdateFlagJob(CellChunk* chunk, GameMap* map);

	virtual void Execute() override;

private:
	CellChunk* m_chunk;
};