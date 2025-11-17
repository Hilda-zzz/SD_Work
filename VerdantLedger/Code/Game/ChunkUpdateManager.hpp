#pragma once
#include <queue>
#include <unordered_set>
#include "Engine/Core/GameTimer.hpp"
#include "TileChunk.hpp"

class TileLayer;
class TileChunk;

struct DelayedDirtyRequest 
{
	TileChunk* m_chunk;
	DirtyType m_dirtyType;
	IntVec2 m_dirtyGridPos;
	GameTimer m_timer;
};

class ChunkUpddateManger
{
public:
	ChunkUpddateManger() {}
	ChunkUpddateManger(TileLayer* layer);
	~ChunkUpddateManger() {}

	void MarkChunkDirty(TileChunk* dirtyChunk,DirtyType dirtyType,IntVec2 const& dirtyGridPos);
	void UpdateDirtyChunks();
	void CheckAndQueueNeighbors(TileChunk* centerChunk, DirtyType dirtyType, IntVec2 const& dirtyGridPos);

protected:
private:
	TileLayer* m_focusLayer = nullptr;
	std::queue<DelayedDirtyRequest> m_dirtyChunkQueue;
	std::unordered_set<uint64_t> m_queuedChunks; 
	int m_maxChunksPerFrame = 1;

	GameTimer m_dirtyObstacleDelayTimer;
	GameTimer m_dirtyFarmlandDelayTimer;
};