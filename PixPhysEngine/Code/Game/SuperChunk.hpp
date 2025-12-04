#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class CellChunk;
class GameMap;

constexpr int CHUNKS_PER_SUPER_CHUNK = 8;  // Super chunk = 8×8 chunks

class SuperChunk {
public:
    SuperChunk(IntVec2 const& superChunkCoords, GameMap* map);
    ~SuperChunk();

    // === Core Info ===
    IntVec2 GetSuperChunkCoords() const { return m_superChunkCoords; }
    AABB2 GetWorldBounds() const { return m_worldBounds; }
    bool IsActive() const { return m_isActive; }

    // === Activation ===
    void Activate();
    void Deactivate();

    // === Chunk Access ===
    CellChunk* GetChunk(int localChunkX, int localChunkY);
    const CellChunk* GetChunk(int localChunkX, int localChunkY) const;
    CellChunk* GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords);

    // === Coordinate Conversion ===
    IntVec2 LocalChunkToGlobal(int localChunkX, int localChunkY) const;
    static IntVec2 GlobalChunkToLocal(IntVec2 const& globalChunkCoords);
    static IntVec2 GlobalChunkToSuperChunk(IntVec2 const& globalChunkCoords);

    // === Rendering ===
    void Render() const;
    void RebuildAllVertices();
    void ClearRenderData();

    // === Generation ===
    void GenerateAllCells();

    // === Update ===
    std::vector<CellChunk*> GetChunksOfPhase(int phaseIndex);

    bool GetIsVisible() const { return m_isVisible; }
    void SetIsVisible(bool visible) 
    { 
        m_isVisible = visible; }
 
private:
    void InitializeChunks();
    void CalculateWorldBounds();
    bool IsLocalChunkValid(int localChunkX, int localChunkY) const;

private:
    GameMap* m_map;
    IntVec2 m_superChunkCoords;          // Super chunk position in super chunk grid
    AABB2 m_worldBounds;                  // World space bounds (in cells)
    bool m_isActive;

    // 8×8 chunk array [localY][localX]
    CellChunk* m_chunks[CHUNKS_PER_SUPER_CHUNK][CHUNKS_PER_SUPER_CHUNK];

    // frustum culling
    bool m_isVisible = false;
};
