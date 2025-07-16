#include "ChunkUpdateManager.hpp"
#include "TileLayer.hpp"

ChunkUpddateManger::ChunkUpddateManger(TileLayer* layer) : m_focusLayer(layer)
{
	m_dirtyObstacleDelayTimer = Timer(0.5f);
	m_dirtyFarmlandDelayTimer = Timer(0.5f);
}

void ChunkUpddateManger::MarkChunkDirty(TileChunk* dirtyChunk, DirtyType dirtyType, IntVec2 const& dirtyGridPos)
{
	DelayedDirtyRequest dirtyRequest;
	dirtyRequest.m_chunk = dirtyChunk;
	dirtyRequest.m_dirtyGridPos = dirtyGridPos;
	dirtyRequest.m_dirtyType = dirtyType;

	dirtyRequest.m_timer = Timer(0.5f);
	dirtyRequest.m_timer.Start();

	m_dirtyChunkQueue.push(dirtyRequest);
	uint64_t chunkKey = TileLayer::GetChunkKey(dirtyChunk->m_startPosition);
	m_queuedChunks.insert(chunkKey);

	CheckAndQueueNeighbors(dirtyChunk, dirtyType, dirtyGridPos);
}

void ChunkUpddateManger::UpdateDirtyChunks()
{
	if (m_dirtyChunkQueue.empty()) return;

	int updatedCount = 0;

	while (updatedCount < m_maxChunksPerFrame) 
	{
		DelayedDirtyRequest curRequest = m_dirtyChunkQueue.front();
		if (curRequest.m_timer.GetElapsedFraction() > 1.f)
		{
			TileChunk* curChunk = curRequest.m_chunk;
			m_dirtyChunkQueue.pop();

			uint64_t chunkKey = TileLayer::GetChunkKey(curChunk->m_startPosition);
			m_queuedChunks.erase(chunkKey);

			switch (curRequest.m_dirtyType)
			{
			case NONE:
				break;
			case DIRTY_STATIC_OBS:
				curChunk->UpdateObstacleVerts();
				break;
			case DIRTY_FARMLAND:
				curChunk->UpdateFarmlandVerts();
				break;
			}
			updatedCount++;
		}
		else
		{
			break;
		}
	}
}

void ChunkUpddateManger::CheckAndQueueNeighbors(TileChunk* centerChunk, DirtyType dirtyType, IntVec2 const& dirtyGridPos)
{
	IntVec2 chunkStartPos = centerChunk->m_startPosition;
	IntVec2 localPos = dirtyGridPos - chunkStartPos;

	std::vector<IntVec2> neighborChunkPositions;

	if (localPos.x == 0) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x - 16, chunkStartPos.y));
	}

	if (localPos.x == 15) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x + 16, chunkStartPos.y));
	}

	if (localPos.y == 0) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x, chunkStartPos.y + 16));
	}

	if (localPos.y == -15) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x, chunkStartPos.y - 16));
	}

	if (localPos.x == 0 && localPos.y == 0) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x - 16, chunkStartPos.y + 16));
	}

	if (localPos.x == 15 && localPos.y == 0) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x + 16, chunkStartPos.y + 16));
	}

	if (localPos.x == 0 && localPos.y == -15) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x - 16, chunkStartPos.y - 16));
	}

	if (localPos.x == 15 && localPos.y == -15) {
		neighborChunkPositions.push_back(IntVec2(chunkStartPos.x + 16, chunkStartPos.y - 16));
	}


	for (const IntVec2& neighborPos : neighborChunkPositions) 
	{
		uint64_t chunkKey = TileLayer::GetChunkKey(neighborPos);

		auto it = m_focusLayer->m_chunkIndexMap.find(chunkKey);
		if (it != m_focusLayer->m_chunkIndexMap.end()) 
		{
			size_t chunkIndex = it->second;
			TileChunk* neighborChunk = &m_focusLayer->m_chunks[chunkIndex];

			if (m_queuedChunks.find(chunkKey) == m_queuedChunks.end()) 
			{
				DelayedDirtyRequest dirtyRequest;
				dirtyRequest.m_chunk = neighborChunk;
				dirtyRequest.m_dirtyGridPos = neighborChunk->m_startPosition;
				dirtyRequest.m_dirtyType = dirtyType;

				dirtyRequest.m_timer = Timer(0.5f);
				dirtyRequest.m_timer.Start();

				m_dirtyChunkQueue.push(dirtyRequest);
				uint64_t curChunkKey = TileLayer::GetChunkKey(neighborChunk->m_startPosition);
				m_queuedChunks.insert(curChunkKey);
			}
		}
	}
}