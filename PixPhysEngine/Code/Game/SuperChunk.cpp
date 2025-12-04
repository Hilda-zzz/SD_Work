#include "SuperChunk.hpp"
#include "CellChunk.hpp"
#include "GameMap.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GamePlayer.hpp"

SuperChunk::SuperChunk(IntVec2 const& superChunkCoords, GameMap* map)
    : m_superChunkCoords(superChunkCoords)
    , m_map(map)
    , m_isActive(false)
{
    // Initialize chunk pointers to nullptr
    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
            m_chunks[y][x] = nullptr;
        }
    }

    CalculateWorldBounds();
    InitializeChunks();
}

SuperChunk::~SuperChunk()
{
    // Delete all chunks
    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
            if (m_chunks[y][x]) {
                delete m_chunks[y][x];
                m_chunks[y][x] = nullptr;
            }
        }
    }
}

void SuperChunk::InitializeChunks()
{
    for (int localY = 0; localY < CHUNKS_PER_SUPER_CHUNK; ++localY) {
        for (int localX = 0; localX < CHUNKS_PER_SUPER_CHUNK; ++localX) {
            IntVec2 globalChunkCoords = LocalChunkToGlobal(localX, localY);
            m_chunks[localY][localX] = new CellChunk(globalChunkCoords, m_map);
        }
    }
}

void SuperChunk::CalculateWorldBounds()
{
    // Super chunk size in chunks
    constexpr int chunksPerSC = CHUNKS_PER_SUPER_CHUNK;
    
    // Each chunk is CHUNK_SIZE cells
    constexpr int cellsPerSC = CHUNK_SIZE * chunksPerSC;

    // Bottom-left world position (in cells)
    int worldMinX = m_superChunkCoords.x * cellsPerSC;
    int worldMinY = m_superChunkCoords.y * cellsPerSC;

    // Top-right world position (in cells)
    int worldMaxX = worldMinX + cellsPerSC;
    int worldMaxY = worldMinY + cellsPerSC;

    m_worldBounds = AABB2(
        static_cast<float>(worldMinX),
        static_cast<float>(worldMinY),
        static_cast<float>(worldMaxX),
        static_cast<float>(worldMaxY)
    );
}

void SuperChunk::Activate()
{
    if (m_isActive) return;

    m_isActive = true;
    
    // Generation and rendering will be handled by GameMap
}

void SuperChunk::Deactivate()
{
    if (!m_isActive) return;

    m_isActive = false;
    ClearRenderData();
}

CellChunk* SuperChunk::GetChunk(int localChunkX, int localChunkY)
{
    GUARANTEE_OR_DIE(IsLocalChunkValid(localChunkX, localChunkY),
        "Invalid local chunk coordinates in SuperChunk");
    return m_chunks[localChunkY][localChunkX];
}

const CellChunk* SuperChunk::GetChunk(int localChunkX, int localChunkY) const
{
    GUARANTEE_OR_DIE(IsLocalChunkValid(localChunkX, localChunkY),
        "Invalid local chunk coordinates in SuperChunk");
    return m_chunks[localChunkY][localChunkX];
}

CellChunk* SuperChunk::GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords)
{
    IntVec2 localCoords = GlobalChunkToLocal(globalChunkCoords);
    
    if (!IsLocalChunkValid(localCoords.x, localCoords.y)) {
        return nullptr;
    }

    return m_chunks[localCoords.y][localCoords.x];
}

IntVec2 SuperChunk::LocalChunkToGlobal(int localChunkX, int localChunkY) const
{
    return IntVec2(
        m_superChunkCoords.x * CHUNKS_PER_SUPER_CHUNK + localChunkX,
        m_superChunkCoords.y * CHUNKS_PER_SUPER_CHUNK + localChunkY
    );
}

IntVec2 SuperChunk::GlobalChunkToLocal(IntVec2 const& globalChunkCoords)
{
    return IntVec2(
        globalChunkCoords.x % CHUNKS_PER_SUPER_CHUNK,
        globalChunkCoords.y % CHUNKS_PER_SUPER_CHUNK
    );
}

IntVec2 SuperChunk::GlobalChunkToSuperChunk(IntVec2 const& globalChunkCoords)
{
    return IntVec2(
        globalChunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
        globalChunkCoords.y / CHUNKS_PER_SUPER_CHUNK
    );
}

void SuperChunk::Render() const
{
    if (!m_isActive) return;

    AABB2 viewBounds = m_map->GetPlayer()->GetBaseCamBound();

    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) 
    {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) 
        {
            if (m_chunks[y][x]) 
            {
				if (!m_map->IsChunkVisible(m_chunks[y][x], viewBounds)) {
                    m_chunks[y][x]->SetIsVisible(false);
				}
                else
                {
					m_chunks[y][x]->SetIsVisible(true);
					m_chunks[y][x]->RenderChunk();
                }
            }
        }
    }
}

void SuperChunk::RebuildAllVertices()
{
    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
            if (m_chunks[y][x]) {
                m_chunks[y][x]->RebuildVertexWithNewColor();
            }
        }
    }
}

void SuperChunk::ClearRenderData()
{
    // Clear vertex buffers to save memory when inactive
    // Note: This would require adding a ClearVertex() method to CellChunk
    // For now, we'll just mark as not dirty to skip unnecessary rebuilds
    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
            if (m_chunks[y][x]) {
                m_chunks[y][x]->ClearDirty();
            }
        }
    }
}

void SuperChunk::GenerateAllCells()
{
    // Will be implemented in Phase 3
    // This will call GameMap's generation methods for each chunk
}

std::vector<CellChunk*> SuperChunk::GetChunksOfPhase(int phaseIndex)
{
    std::vector<CellChunk*> result;
    result.reserve(16); // Typical case: ~16 chunks per phase in 8×8 grid

    for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
        for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
            CellChunk* chunk = m_chunks[y][x];
            if (chunk && chunk->GetPhaseIndex() == phaseIndex) {
                result.push_back(chunk);
            }
        }
    }

    return result;
}

bool SuperChunk::IsLocalChunkValid(int localChunkX, int localChunkY) const
{
    return (localChunkX >= 0 && localChunkX < CHUNKS_PER_SUPER_CHUNK &&
            localChunkY >= 0 && localChunkY < CHUNKS_PER_SUPER_CHUNK);
}
