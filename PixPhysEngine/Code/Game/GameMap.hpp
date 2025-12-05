#pragma once
#include "BaseMap.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "RegionDefinition.hpp"

class SuperChunk;
class GamePlayer;
class HerringboneMapGenerator;
class GameMapDebugRenderer;
class RegionManager;              // ⭐ 新增前向声明
class HerringboneTileset;         // ⭐ 新增前向声明

class GameMap : public BaseMap 
{
public:
    GameMap(GamePlayer* player, IntVec2 const& mapSizeInSuperChunks);
    ~GameMap() override;

    // === BaseMap Interface ===
    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() const override;
    Rgba8 GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const override;

    // === Super Chunk Management ===
    SuperChunk* GetSuperChunkByCoords(IntVec2 const& superChunkCoords);
    SuperChunk* GetSuperChunkByWorldPos(int worldX, int worldY);
    void ActivateSuperChunk(IntVec2 const& superChunkCoords);
    void DeactivateSuperChunk(IntVec2 const& superChunkCoords);

    // === Chunk Access (for cross-chunk operations) ===
    CellChunk* GetChunkByWorldPos(int worldX, int worldY);
    CellChunk* GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords);

    // === Cell Access ===
    Cell& GetCellInChunk(int worldX, int worldY);
    const Cell& GetCellInChunk(int worldX, int worldY) const;

    // === Coordinate Conversion ===
    static IntVec2 WorldToSuperChunk(int worldX, int worldY);
    static IntVec2 SuperChunkToWorld(IntVec2 const& superChunkCoords);
    static IntVec2 ChunkToSuperChunk(IntVec2 const& chunkCoords);

    // === Generation (Phase 2-3) ===
	void GenerateSuperChunkCells(SuperChunk* sc);
    void GenerateMapPixels();
    void GenerateCellsFromPixels();
	void SetMapGenerator(HerringboneMapGenerator* generator) {
		m_mapGenerator = generator;
	}

    // === Update System (Phase 5) ===
    void UpdateCellsPhysInChunk();

    // === Debug ===
    void RenderDebugInfo() const;
    IntVec2 GetSuperChunkGridSize() const { return m_superChunkGridSize; }
    int GetActiveSuperChunkCount() const { return static_cast<int>(m_activeSuperChunks.size()); }

	// === Player Position Tracking ===
    GamePlayer* GetPlayer() { return m_player; }
	IntVec2 GetPlayerChunkCoords() const;
	IntVec2 GetPlayerSuperChunkCoords() const;
	CellChunk* GetPlayerCurrentChunk() const;
	SuperChunk* GetPlayerCurrentSuperChunk() const;

	// === Streaming Management ===
	void UpdateSuperChunkStreaming();
	void SetActivationRadius(int radius) { m_activationRadius = radius; }
	int GetActivationRadius() const { return m_activationRadius; }

	// ⭐ 新增：设置PCG资源
	void SetHerringboneTileset(HerringboneTileset* tileset);

	RegionManager* GetRegionManager() const { return m_regionManager; }
	const std::vector<SuperChunk*>& GetActiveSuperChunks() const {
		return m_activeSuperChunks;
	}

	// === Frustum Culling Implementation ===
	void GetVisibleSuperChunks(AABB2 const& viewBounds, std::vector<SuperChunk*>& outVisible) const;
	bool IsSuperChunkVisible(SuperChunk const* sc, AABB2 const& viewBounds) const;
	bool IsChunkVisible(CellChunk const* chunk, AABB2 const& viewBounds) const;

private:
    void InitializeSuperChunks();
    bool IsInBounds(int worldX, int worldY) const;
    bool IsSuperChunkCoordsValid(IntVec2 const& coords) const;
    void InitializeRegionDefs();

	// === Streaming Helpers ===
	void CalculateRequiredSuperChunks(IntVec2 const& playerSuperChunkCoords,
		std::vector<IntVec2>& outRequired) const;
	void ActivateRequiredSuperChunks(std::vector<IntVec2> const& required);
	void DeactivateDistantSuperChunks(std::vector<IntVec2> const& required);

	// ⭐ 新增：PCG辅助函数
	void InitializePCGSystem();
	CellMatType MapColorToMaterialType(Rgba8 const& color) const;


private:
    GamePlayer* m_player;
    
    // Super chunk grid
    IntVec2 m_superChunkGridSize;        // Grid size in super chunks
    std::vector<std::vector<SuperChunk*>> m_superChunks;  // [superChunkY][superChunkX]
    
    // Active super chunk tracking
    std::vector<SuperChunk*> m_activeSuperChunks;

    // PCG Generator (Phase 2)
    HerringboneMapGenerator* m_mapGenerator;
    unsigned int m_worldSeed;

	// ⭐ 新增：Region管理
	RegionManager* m_regionManager;
	HerringboneTileset* m_herringboneTileset;

    // Debug Render
    GameMapDebugRenderer* m_debugRenderer = nullptr;

	// Streaming
	int m_activationRadius=1;              // Radius in super chunks (1 = 3×3, 2 = 5×5)
	IntVec2 m_lastPlayerSuperChunk;      // Track player movement between super chunks

    // Region Def
    RegionDefinition m_jungleRegion;

    bool m_showDebugPanel = true;
};
