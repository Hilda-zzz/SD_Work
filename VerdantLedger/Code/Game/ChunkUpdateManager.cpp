#include "ChunkUpdateManager.hpp"
#include "TileLayer.hpp"

ChunkUpddateManger::ChunkUpddateManger(TileLayer* layer) : m_focusLayer(layer)
{
	m_dirtyObstacleDelayTimer = Timer(0.5f);
	m_dirtyFarmlandDelayTimer = Timer(0.5f);
}

void ChunkUpddateManger::UpdateVisibleChunks(float deltaSeconds, Vec2 const& cameraCenter, Vec2 const& cameraSize)
{
	if (!m_focusLayer) return;

	// 计算相机视野的边界（世界坐标）
	float halfWidth = cameraSize.x * 0.5f;
	float halfHeight = cameraSize.y * 0.5f;

	Vec2 cameraMin = Vec2(cameraCenter.x - halfWidth, cameraCenter.y - halfHeight);
	Vec2 cameraMax = Vec2(cameraCenter.x + halfWidth, cameraCenter.y + halfHeight);

	// 转换为chunk网格坐标范围
	IntVec2 minChunkPos = IntVec2(
		(int)floor(cameraMin.x / 16.0f) * 16,
		(int)ceil(cameraMax.y / 16.0f) * 16    // Y轴：上边界用ceil
	);
	IntVec2 maxChunkPos = IntVec2(
		(int)ceil(cameraMax.x / 16.0f) * 16,
		(int)floor(cameraMin.y / 16.0f) * 16   // Y轴：下边界用floor
	);

	// 注意：现在Y坐标是从大到小遍历
	for (int chunkY = minChunkPos.y; chunkY >= maxChunkPos.y; chunkY -= 16)
	{
		for (int chunkX = minChunkPos.x; chunkX <= maxChunkPos.x; chunkX += 16)
		{
			IntVec2 chunkStartPos(chunkX, chunkY);
			uint64_t chunkKey = TileLayer::GetChunkKey(chunkStartPos);

			auto it = m_focusLayer->m_chunkIndexMap.find(chunkKey);
			if (it != m_focusLayer->m_chunkIndexMap.end())
			{
				size_t chunkIndex = it->second;
				TileChunk& chunk = m_focusLayer->m_chunks[chunkIndex];
				chunk.Update(deltaSeconds);
			}
		}
	}

// 	for (int chunkY = minChunkPos.y; chunkY <= maxChunkPos.y; chunkY += 16)
// 	{
// 		for (int chunkX = minChunkPos.x; chunkX <= maxChunkPos.x; chunkX += 16)
// 		{
// 			IntVec2 chunkStartPos(chunkX, chunkY);
// 			uint64_t chunkKey = TileLayer::GetChunkKey(chunkStartPos);
// 
// 			auto it = m_focusLayer->m_chunkIndexMap.find(chunkKey);
// 			if (it != m_focusLayer->m_chunkIndexMap.end())
// 			{
// 				size_t chunkIndex = it->second;
// 				TileChunk& chunk = m_focusLayer->m_chunks[chunkIndex];
// 				chunk.Update(deltaSeconds);
// 			}
// 		}
// 	}
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
				uint64_t chunkKey = TileLayer::GetChunkKey(neighborChunk->m_startPosition);
				m_queuedChunks.insert(chunkKey);
			}
		}
	}
}

// void ChunkUpddateManger::UpdateDelayDirtyObstacleChunk()
// {
// 	if (m_curDirtyChunk && !m_dirtyObstacleDelayTimer.IsStopped() && m_dirtyObstacleDelayTimer.GetElapsedFraction() > 1.f)
// 	{
// 		m_dirtyObstacleDelayTimer.Stop();
// 		m_curDirtyChunk->m_isDirtyObstacle = true;
// 		m_curDirtyChunk = nullptr;
// 	}
// }

// void ChunkUpddateManger::UpdateDelayDirtyFarmlandChunk()
// {
// 	if (m_curDirtyChunk && !m_dirtyFarmlandDelayTimer.IsStopped() && m_dirtyFarmlandDelayTimer.GetElapsedFraction() > 1.f)
// 	{
// 		m_dirtyFarmlandDelayTimer.Stop();
// 		m_curDirtyChunk->m_isDirtyFarmland = true;
// 		m_curDirtyChunk = nullptr;
// 	}
// }
