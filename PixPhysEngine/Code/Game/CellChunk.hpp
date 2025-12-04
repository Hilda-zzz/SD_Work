#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <mutex>
#include "Engine/Core/Vertex_PCU.hpp"

class BaseMap;

constexpr int CHUNK_SIZE = 64;

struct PendingCellMove {
	int m_localX;
	int m_localY;
	Cell m_cell;

	// Optional: for debugging
	IntVec2 m_sourceChunkIndex;
};

class CellChunk {
public:
	CellChunk(IntVec2 const& chunkIndex, BaseMap* map);
	~CellChunk();

	void RebuildVertexWithNewColor();
	void RebuildVertexUseSelfColor();
	void RenderChunk() const;

	void RenderSolidCells() const;
	void RenderLiquidCells() const;

	// === Core Data Access ===
	Cell& GetLocalCell(int localX, int localY);
	Cell const& GetLocalCell(int localX, int localY) const;

	// === Chunk Info ===
	IntVec2 GetChunkIndex() const { return m_chunkCoords; }
	AABB2 GetWorldBounds() const { return m_boundsInWorld; }
	int GetPhaseIndex() const { return m_updatePhaseIndex; }
	bool IsDirty() const { return m_isDirty; }

	// === Chunk State ===
	void MarkDirty() { m_isDirty = true; }
	void ClearDirty() { m_isDirty = false; }

	// === Cell Operations (within chunk) ===
	void SwapLocalCells(int x1, int y1, int x2, int y2);
	bool IsLocalPosValid(int localX, int localY) const;

	// === Cross-Chunk Movement ===
	// Thread-safe: called by worker threads
	//void QueueIncomingCell(int localX, int localY, const Cell& cell, IntVec2 sourceChunk = IntVec2(-1, -1));
	// Single-threaded: called at sync point
	//void ApplyIncomingCells();

	// === Coordinate Conversion ===
	IntVec2 LocalToWorld(int localX, int localY) const;
	static IntVec2 WorldToLocal(int worldX, int worldY);
	static IntVec2 WorldToChunkIndex(int worldX, int worldY);

	// === Update Flags ===
	void ResetUpdateFlags();

	// === Statistics (optional) ===
	int GetActiveCellCount() const;
	bool IsEmpty() const;

	bool GetIsVisible() const { return m_isVisible; }
	void SetIsVisible(bool visible) { m_isVisible = visible; }

private:
	void CalculateWorldBounds();
	void CalculateUpdatePhaseIndex();

private:
	BaseMap* m_map = nullptr;
	// === Core Data ===
	std::vector<std::vector<Cell>> m_cells;           // [y][x] - 64x64 grid
	IntVec2 m_chunkCoords;                             // Chunk position in chunk grid
	AABB2 m_boundsInWorld;                              // World space bounds

	// === Chunk Properties ===
	int m_updatePhaseIndex;                                      // 0-3 for scheduling
	bool m_isDirty;                                   // Need update?

	// === Cross-Chunk Communication ===
	std::vector<PendingCellMove> m_incomingCells;     // Pending cells from other chunks
	std::mutex m_incomingMutex;                       // Protects m_incomingCells only

	// === Render ===
	std::vector<Vertex_PCU> m_solidVertex;
	std::vector<Vertex_PCU> m_liquidVertex;
	bool m_isVisible = false;
};