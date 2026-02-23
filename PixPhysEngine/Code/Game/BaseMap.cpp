#include "BaseMap.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cmath>

Rgba8 BaseMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
	return Rgba8();
}

BaseMap::BaseMap(const IntVec2& mapSize, int chunkSize)
	: m_mapSize(mapSize)
	, m_chunkSize(chunkSize)
	, m_mapBounds(0.f, 0.f, (float)mapSize.x, (float)mapSize.y)
{
	// 计算需要多少个 Chunk
	m_chunkGridSize.x = (m_mapSize.x + m_chunkSize - 1) / m_chunkSize;  // 向上取整
	m_chunkGridSize.y = (m_mapSize.y + m_chunkSize - 1) / m_chunkSize;
}

BaseMap::~BaseMap() 
{
	//DestroyChunkGrid();
}

// ========================================
// Chunk 管理
// ========================================

//void BaseMap::CreateChunkGrid() {
//	m_chunks.resize(m_chunkGridSize.y);
//
//	for (int chunkY = 0; chunkY < m_chunkGridSize.y; ++chunkY) {
//		m_chunks[chunkY].resize(m_chunkGridSize.x);
//
//		for (int chunkX = 0; chunkX < m_chunkGridSize.x; ++chunkX) {
//			IntVec2 chunkCoords(chunkX, chunkY);
//			m_chunks[chunkY][chunkX] = new CellChunk(chunkCoords, this);
//		}
//	}
//}
//
//void BaseMap::DestroyChunkGrid() {
//	for (auto& row : m_chunks) {
//		for (auto& chunk : row) {
//			delete chunk;
//			chunk = nullptr;
//		}
//	}
//	m_chunks.clear();
//}

// ========================================
// Cell 访问
// ========================================

Cell& BaseMap::GetCell(int worldX, int worldY) {
	CellChunk* chunk = GetChunkByWorldPos(worldX, worldY);
	GUARANTEE_OR_DIE(chunk != nullptr, "Invalid world position in GetCell");

	IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);
	return chunk->GetLocalCell(localCoords.x, localCoords.y);
}

const Cell& BaseMap::GetCell(int worldX, int worldY) const {
	CellChunk* chunk = const_cast<BaseMap*>(this)->GetChunkByWorldPos(worldX, worldY);
	GUARANTEE_OR_DIE(chunk != nullptr, "Invalid world position in GetCell");

	IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);
	return chunk->GetLocalCell(localCoords.x, localCoords.y);
}

// ========================================
// Chunk 访问
// ========================================

CellChunk* BaseMap::GetChunk(int chunkX, int chunkY) {
	//if (!IsChunkIndexValid(chunkX, chunkY)) {
	//	return nullptr;
	//}
	//return m_chunks[chunkY][chunkX];
	return nullptr;
}

const CellChunk* BaseMap::GetChunk(int chunkX, int chunkY) const {
	//if (!IsChunkIndexValid(chunkX, chunkY)) {
	//	return nullptr;
	//}
	//return m_chunks[chunkY][chunkX];
	return nullptr;
}

CellChunk* BaseMap::GetChunkByWorldPos(int worldX, int worldY) {
	IntVec2 chunkCoords = CellChunk::WorldToChunkIndex(worldX, worldY);
	return GetChunk(chunkCoords.x, chunkCoords.y);
}

const CellChunk* BaseMap::GetChunkByWorldPos(int worldX, int worldY) const {
	IntVec2 chunkCoords = CellChunk::WorldToChunkIndex(worldX, worldY);
	return GetChunk(chunkCoords.x, chunkCoords.y);
}

// ========================================
// 物理查询接口
// ========================================

bool BaseMap::MSCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const {
	if (!IsInBounds(x, y)) {
		return false;
	}

	const Cell& targetCell = GetCell(x, y);
	const CellMatDef& targetMatDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
	float targetDensity = targetMatDef.m_density;

	// MoveSolid 可以移动到的条件：
	// 1. 目标是空的
	// 2. 目标是液体 且 当前密度大于目标密度（浮力）
	return targetCell.IsEmpty() ||
		((curDensity > targetDensity) &&
			targetMatDef.m_physicsType == PhyType::PHY_LIQUID);
}

bool BaseMap::LiquidCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const {
	if (!IsInBounds(x, y)) {
		return false;
	}

	const Cell& targetCell = GetCell(x, y);
	float targetDensity = CellMatManager::GetMaterialDef(targetCell.m_type.load()).m_density;

	// Liquid 可以移动到密度更小的位置（包括空气，密度=0）
	return curDensity > targetDensity;
}

bool BaseMap::CanMoveHorizontally(int x, int y, const Cell& cell) const {
	// 需要足够的速度 且 有支撑才能水平移动
	return (std::abs(cell.m_velocityX) > 1.0f || std::abs(cell.m_velocityY) > 80.0f) &&
		HasSupport(x, y);
}

bool BaseMap::HasSupport(int x, int y) const {
	// 在底边界就算有支撑
	if (y == 0) {
		return true;
	}

	// 检查下方是否有非空格子
	//if (!IsInBounds(x, y - 1)) {
	//	return false;  // 边界外视为没有支撑
	//}

	return !GetCell(x, y - 1).IsEmpty();
}

