#include "CellChunk.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
//#include "Game/SandboxMap.hpp"
#include "Game/BaseMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

CellChunk::CellChunk(IntVec2 const& chunkCoords, BaseMap* map)
	: m_chunkCoords(chunkCoords)
	, m_updatePhaseIndex(0)
	, m_isDirty(true)  // Start dirty to ensure first update
	,m_map(map)
{
	// Initialize 64x64 cell grid
	m_cells.resize(CHUNK_SIZE, std::vector<Cell>(CHUNK_SIZE));

	// Calculate world bounds
	CalculateWorldBounds();

	// Calculate chunk color (0-3)
	CalculateUpdatePhaseIndex();

	// Pre-allocate incoming cells buffer
	m_incomingCells.reserve(256);  // Reasonable estimate  // #TODO: what it's for???
}

CellChunk::~CellChunk()
{
}

void CellChunk::RebuildVertexWithNewColor()
{
	m_vertex.clear();
	for (int localY = 0; localY < CHUNK_SIZE; ++localY) {
		for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
			const Cell& cell = GetLocalCell(localX, localY);
			if (cell.m_type != CellMatType::MAT_EMPTY) {
				IntVec2 worldPos = LocalToWorld(localX, localY);

				// Get color based on debug mode
				Rgba8 cellColor = m_map->GetCellDebugColor(cell,worldPos);

				AddVertsForAABB2D(m_vertex,
					AABB2(Vec2((float)worldPos.x, (float)worldPos.y),
						Vec2((float)worldPos.x + 1.f, (float)worldPos.y + 1.f)),
					cellColor);
			}
		}
	}
}

void CellChunk::RebuildVertexUseSelfColor()
{
	m_vertex.clear();
	for (int localY = 0; localY < CHUNK_SIZE; ++localY) 
	{
		for (int localX = 0; localX < CHUNK_SIZE; ++localX) 
		{
			const Cell& cell = GetLocalCell(localX, localY);
			if (cell.m_type != CellMatType::MAT_EMPTY) {
				IntVec2 worldPos = LocalToWorld(localX, localY);
				AddVertsForAABB2D(m_vertex,
					AABB2(Vec2((float)worldPos.x, (float)worldPos.y),
						Vec2((float)worldPos.x + 1.f, (float)worldPos.y + 1.f)),
					cell.m_color);
			}
		}
	}
}


void CellChunk::RenderChunk() const
{
	g_theRenderer->DrawVertexArray(m_vertex);
}

// ==================== Core Data Access ====================

Cell& CellChunk::GetLocalCell(int localX, int localY)
{
	GUARANTEE_OR_DIE(IsLocalPosValid(localX, localY),
		Stringf("Invalid local position (%d, %d)", localX, localY));
	return m_cells[localY][localX];
}

Cell const& CellChunk::GetLocalCell(int localX, int localY) const
{
	GUARANTEE_OR_DIE(IsLocalPosValid(localX, localY),
		Stringf("Invalid local position (%d, %d)", localX, localY));
	return m_cells[localY][localX];
}


// ==================== Cell Operations ====================

void CellChunk::SwapLocalCells(int x1, int y1, int x2, int y2)           // #TODO: cell behavior system should use it ??
{
	GUARANTEE_OR_DIE(IsLocalPosValid(x1, y1) && IsLocalPosValid(x2, y2),
		"Invalid swap positions");

	std::swap(m_cells[y1][x1], m_cells[y2][x2]);
	m_isDirty = true;
}

bool CellChunk::IsLocalPosValid(int localX, int localY) const
{
	return localX >= 0 && localX < CHUNK_SIZE &&
		localY >= 0 && localY < CHUNK_SIZE;
}

// ==================== Cross-Chunk Movement ====================

//void CellChunk::QueueIncomingCell(int localX, int localY, const Cell& cell, IntVec2 sourceChunk)
//{
//	GUARANTEE_OR_DIE(IsLocalPosValid(localX, localY),
//		Stringf("Invalid incoming position (%d, %d)", localX, localY));
//
//	// Thread-safe: lock only the incoming queue
//	std::lock_guard<std::mutex> lock(m_incomingMutex);
//
//	PendingCellMove move;
//	move.m_localX = localX;
//	move.m_localY = localY;
//	move.m_cell = cell;
//	move.m_sourceChunkIndex = sourceChunk;
//
//	m_incomingCells.push_back(move);
//}

//void CellChunk::ApplyIncomingCells()
//{
//	// No lock needed - called at sync point (single-threaded)
//
//	for (const auto& move : m_incomingCells) {
//		Cell& targetCell = m_cells[move.m_localY][move.m_localX];
//
//		if (targetCell.IsEmpty()) {
//			// Normal case: empty slot
//			targetCell = move.m_cell;
//			// targetCell.m_updatedThisFrame = true;  // #TODO: added by myself
//		}
//		//else {
//		//	// Conflict! This should never happen with 32-cell limit
//		//	ERROR_AND_DIE(Stringf(
//		//		"Chunk cell collision at chunk(%d,%d) local(%d,%d) from chunk(%d,%d)",
//		//		m_chunkCoords.x, m_chunkCoords.y,
//		//		move.m_localX, move.m_localY,
//		//		move.m_sourceChunkIndex.x, move.m_sourceChunkIndex.y
//		//	));
//		//}
//	}
//
//	if (!m_incomingCells.empty()) {
//		m_isDirty = true;
//		m_incomingCells.clear();
//	}
//}

// ==================== Coordinate Conversion ====================

IntVec2 CellChunk::LocalToWorld(int localX, int localY) const
{
	int worldX = m_chunkCoords.x * CHUNK_SIZE + localX;
	int worldY = m_chunkCoords.y * CHUNK_SIZE + localY;
	return IntVec2(worldX, worldY);
}

IntVec2 CellChunk::WorldToLocal(int worldX, int worldY)
{
	int localX = worldX % CHUNK_SIZE;
	int localY = worldY % CHUNK_SIZE;

	// Handle negative coordinates
	if (localX < 0) localX += CHUNK_SIZE;
	if (localY < 0) localY += CHUNK_SIZE;

	return IntVec2(localX, localY);
}

IntVec2 CellChunk::WorldToChunkIndex(int worldX, int worldY)
{
	int chunkX = worldX / CHUNK_SIZE;
	int chunkY = worldY / CHUNK_SIZE;

	// Handle negative coordinates
	if (worldX < 0 && worldX % CHUNK_SIZE != 0) chunkX--;
	if (worldY < 0 && worldY % CHUNK_SIZE != 0) chunkY--;

	return IntVec2(chunkX, chunkY);
}

// ==================== Update Flags ====================

void CellChunk::ResetUpdateFlags()
{
	for (int y = 0; y < CHUNK_SIZE; ++y) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			m_cells[y][x].m_updatedThisFrame = false;
		}
	}
}

// ==================== Statistics ====================

int CellChunk::GetActiveCellCount() const
{
	int count = 0;
	for (int y = 0; y < CHUNK_SIZE; ++y) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			if (!m_cells[y][x].IsEmpty()) {
				count++;
			}
		}
	}
	return count;
}

bool CellChunk::IsEmpty() const
{
	for (int y = 0; y < CHUNK_SIZE; ++y) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			if (!m_cells[y][x].IsEmpty()) {
				return false;
			}
		}
	}
	return true;
}

// ==================== Private Helpers ====================

void CellChunk::CalculateWorldBounds()
{
	float minX = (float)(m_chunkCoords.x * CHUNK_SIZE);
	float minY = (float)(m_chunkCoords.y * CHUNK_SIZE);
	float maxX = minX + (float)CHUNK_SIZE;
	float maxY = minY + (float)CHUNK_SIZE;

	m_boundsInWorld = AABB2(Vec2(minX, minY), Vec2(maxX, maxY));
}

void CellChunk::CalculateUpdatePhaseIndex()
{
	// 4-color checkerboard pattern
	// Pattern:
	//	   x=0
	//		2 3 2 3
	//		0 1 0 1
	//		2 3 2 3
	//y=0	0 1 0 1
	m_updatePhaseIndex = (m_chunkCoords.x % 2) * 1 + (m_chunkCoords.y % 2) * 2;
}