#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include "CellChunk.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
class GameMap;
class StructuredBuffer;
class ConstantBuffer;
class IndirectArgsBuffer;
class Shader;
class ComputeShader;
class VertexBuffer;
class IndexBuffer;
class Texture;

constexpr int CHUNKS_PER_SUPER_CHUNK = 8;  // Super chunk = 8×8 chunks
constexpr int CELLS_PER_SUPER_CHUNK = CHUNKS_PER_SUPER_CHUNK* CHUNK_SIZE;

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
    CellChunk* GetChunkByLocalCoords(IntVec2 const& localChunkCoords);

    // === Coordinate Conversion ===
    IntVec2 LocalChunkToGlobal(int localChunkX, int localChunkY) const;
    static IntVec2 GlobalChunkToLocal(IntVec2 const& globalChunkCoords);
    static IntVec2 GlobalChunkToSuperChunk(IntVec2 const& globalChunkCoords);

    // === Rendering ===
    void RenderSolid() const;
    void RenderSolid_GPU() const;

    void RenderLiquid() const;
    void RebuildAllVertices();
    void ClearRenderData();

    // === Generation ===
    void GenerateAllCells();

    // === Update ===
    std::vector<CellChunk*> GetChunksOfPhase(int phaseIndex);

	// === Chunk Update Control ===
	void UpdateAllChunks();
	void UpdateChunksOfPhase(int phaseIndex, GameMap* map);

	// === Dirty Tracking ===
	bool HasDirtyChunks() const;
	void ResetAllUpdateFlags();

	// === Statistics ===
	int GetDirtyChunkCount() const;
	int GetActiveCellCount() const;

    bool GetIsVisible() const { return m_isVisible; }
    void SetIsVisible(bool visible) 
    { 
        m_isVisible = visible; 
    }

	// ==================== NEW: Rigid Body Management ====================
    // === Rigid Body Activation ===
	void ActivateAllRigidBodies();      // 激活所有chunk的刚体
	void DeactivateAllRigidBodies();    // 失活所有chunk的刚体

	// === Rigid Body Update ===
	void UpdateAllRigidBodies(float deltaTime);  // 更新所有chunk的刚体
	// ====================================================================

private:
    void InitializeChunks();
    void CalculateWorldBounds();
    bool IsLocalChunkValid(int localChunkX, int localChunkY) const;

    void UpdateSingleChunkInternal(CellChunk* chunk, GameMap* map);

	// === 新增：GPU渲染相关函数 ===
	void InitializeGPUBuffers();
	void ShutdownGPUBuffers();
	void UpdateCellDataBuffer() const;

public:

private:
    GameMap* m_map;
    IntVec2 m_superChunkCoords;          // Super chunk position in super chunk grid
    AABB2 m_worldBounds;                  // World space bounds (in cells)
    bool m_isActive;

    // 8×8 chunk array [localY][localX]
    CellChunk* m_chunks[CHUNKS_PER_SUPER_CHUNK][CHUNKS_PER_SUPER_CHUNK];
    mutable std::vector<CellChunk*> m_visibleChunks;

    // frustum culling
    bool m_isVisible = false;

    //mutable VertexBuffer* m_solidVBO = nullptr;
	mutable std::vector<Vertex_PCU> m_batchedSolidVertices;
	mutable bool m_solidBatchDirty = true;

	// === GPU渲染相关（新增）===
	mutable StructuredBuffer* m_cellDataBuffer = nullptr;      // Cell数据（12 bytes/cell）
	mutable VertexBuffer* m_cellVertexBuffer = nullptr;    // GPU生成的顶点（临时用StructuredBuffer）
    mutable IndexBuffer* m_cellIndexBuffer = nullptr;
	mutable IndirectArgsBuffer* m_cellIndirectArgs = nullptr;  // DrawIndirect参数
	mutable ConstantBuffer* m_cellPopulateCBO = nullptr;       // Populate Shader常量

	mutable ComputeShader* m_cellPopulateShader = nullptr;     // 生成顶点的CS
	mutable Shader* m_cellMRTShader = nullptr;                 // MRT渲染Shader

	int m_totalCellsInSuperChunk = 0;                          // 262,144
    mutable uint32_t m_lastUploadedCellCount = 0;
	bool m_gpuBuffersInitialized = false;

    Texture* m_mainColorTexture = nullptr;
    Texture* m_collisionTexture = nullptr;
    Texture* m_bloomTexture = nullptr;
};
