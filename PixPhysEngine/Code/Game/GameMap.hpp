#pragma once
#include "BaseMap.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "RegionDefinition.hpp"
#include <ThirdParty/box2d/include/box2d/id.h>
#include "ThirdParty/box2d/include/box2d/types.h"

class SuperChunk;
class GamePlayer;
class HerringboneMapGenerator;
class GameMapDebugRenderer;
class RegionManager;
class HerringboneTileset;
class RigidBodyObjectPool;
class Texture;
class Shader;
class ConstantBuffer;
class ParallaxBackground;

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

    // Cell behavior interface
	Cell& GetCell(int worldX, int worldY) override;
	Cell const& GetCell(int worldX, int worldY) const override;

	CellChunk* GetChunkByWorldPos(int worldX, int worldY) override;
	CellChunk const* GetChunkByWorldPos(int worldX, int worldY) const override;
	CellChunk* GetChunk(int globalChunkX, int globalChunkY) override;

	bool MSCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const override;
	bool LiquidCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const override;
	bool HasSupport(int x, int y) const override;
	bool IsInBounds(int worldX, int worldY) const override;
	//virtual bool CanMoveHorizontally(int x, int y, const Cell& cell) const override;
	

	void PlaceMaterialInChunk(int worldX, int worldY, CellMatType type, bool isRb = false) override;

    // === Super Chunk Management ===
    SuperChunk* GetSuperChunkByCoords(IntVec2 const& superChunkCoords);
    SuperChunk* GetSuperChunkByWorldPos(int worldX, int worldY);
    void ActivateSuperChunk(IntVec2 const& superChunkCoords);
    void DeactivateSuperChunk(IntVec2 const& superChunkCoords);

    // === Chunk Access (for cross-chunk operations) ===
    //CellChunk* GetChunkByWorldPos(int worldX, int worldY);
    CellChunk* GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords);

    // === Coordinate Conversion ===
    static IntVec2 WorldToSuperChunk(int worldX, int worldY);
    static IntVec2 SuperChunkToWorld(IntVec2 const& superChunkCoords);
    static IntVec2 ChunkToSuperChunk(IntVec2 const& chunkCoords);

    // === Generation (Phase 2-3) ===
	void GenerateSuperChunkCells(SuperChunk* sc);
    //void GenerateMapPixels();
    //void GenerateCellsFromPixels();
	void SetMapGenerator(HerringboneMapGenerator* generator) {
		m_mapGenerator = generator;
	}

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

	void UpdateSingleThreadChunk(CellChunk* chunk);

	AABB2 const& GetActiveBound() const { return m_curActiveBound; };

	void UpdateSingleChunkChemical(CellChunk* chunk);

	// ==================== NEW: Rigid Body System ====================

	// === Physics World ===
	void CreateBox2DWorld();                    // 创建Box2D物理世界
	void DestroyBox2DWorld();                   // 销毁Box2D物理世界
	b2WorldId GetPhysicsWorldId() const { return m_b2WorldId; }

	// === Rigid Body System ===
	void InitializeRigidBodySystem();           // 初始化刚体系统（创建对象池）
	void UpdatePhysicsWorld(float deltaTime);   // 更新物理世界
	void UpdateActiveSuperChunksRigidBodies(float deltaTime);  // 更新所有激活SuperChunk的刚体

	// === Object Pool Access ===
	RigidBodyObjectPool* GetRigidBodyObjectPool() { return m_rigidBodyObjectPool; }
	const RigidBodyObjectPool* GetRigidBodyObjectPool() const { return m_rigidBodyObjectPool; }

	// === Debug Rendering ===
	void InitializeDebugDraw();
	void RenderRigidBodiesDebug() const;        // 渲染刚体调试信息

	// ====================================================================
	// === Parallax Background Access ===
	ParallaxBackground* GetParallaxBackground() { return m_parallaxBackground; }
	const ParallaxBackground* GetParallaxBackground() const { return m_parallaxBackground; }

	// === Parallax Theme Management ===
	void SetParallaxTheme(const std::string& themeName);
	void UpdateParallaxForGameState();

	b2WorldId GetB2WorldId() const { return m_b2WorldId; }

private:
    void InitializeSuperChunks();
    void InitializeRegionDefs();

    //bool IsInBounds(int worldX, int worldY) const;
    bool IsSuperChunkCoordsValid(IntVec2 const& coords) const;

    // === Update ==============================================================
	// === Phase-Based Cell Update ===
	void UpdateCellsPhysInChunk();
	void UpdateActiveSuperChunks(float deltaTime);
	void RebuildVerts();

	// Chemical
	void UpdateCellsChemicalInChunk();

	// === Cross-SuperChunk Movement ===
	//bool CanMoveToWorldPos(int worldX, int worldY) const;
	bool IsSuperChunkActiveAt(int worldX, int worldY) const;

	// === Update Statistics ===
	int GetActiveCellCount() const;
	int GetUpdatedChunkCount() const;

	void UpdateSuperChunksOfPhase(int phaseIndex);
	void UpdateSingleThreadSuperChunk(SuperChunk* sc);

    // --------------------------------------------------------------------------

	// === Streaming Helpers ===
	void CalculateRequiredSuperChunks(IntVec2 const& playerSuperChunkCoords,
		std::vector<IntVec2>& outRequired);
	void ActivateRequiredSuperChunks(std::vector<IntVec2> const& required);
	void DeactivateDistantSuperChunks(std::vector<IntVec2> const& required);

	// PCG help funcs
	void InitializePCGSystem();

	// new Physics
	void UpdateRigidBodiesAsync();  // 新增：异步更新所有刚体
	void UpdateRigidBodiesSync(float deltaTime);  // 同步版本（用于对比/调试）

	void RenderActiveSuperChunk() const;
	void ApplyBloomEffect() const;

	void InitializeParallaxBackground();


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
	bool m_showMatBrushPanel = true;

	// =============== Update Control ==================
	UpdateOrder m_updateOrder = UpdateOrder::ROTATING;
	bool m_useJobSystem =true;
	bool m_useJobSystemBuildVerts = true;
	bool m_useJSResetFlags = true;
	bool m_useJSChemical = true;

	AABB2 m_curActiveBound = AABB2();
	IntVec2 m_curActiveBoundMin;   
	IntVec2 m_curActiveBoundMax;   

	// ==================== NEW: Physics & Rigid Body Members ====================

	// === Box2D World ===
	b2WorldId m_b2WorldId;                      // 全局唯一的Box2D物理世界
	//mutable b2DebugDraw m_debugDraw;

	// === Rigid Body System ===
	RigidBodyObjectPool* m_rigidBodyObjectPool; // 全局刚体对象池

	// === Configuration ===
	int m_rigidBodyUpdateInterval = 120;        // 刚体更新间隔（帧）

	// ====================================================================

	// === MRT for Cell Rendering ===
	Texture* m_cellMainColorTexture = nullptr;
	Texture* m_cellCollisionTexture = nullptr;
	Texture* m_cellBloomTexture = nullptr;

	// === Bloom Multi-level ===
	Texture* m_bloomDownsample1 = nullptr;  // 1/2分辨率
	Texture* m_bloomDownsample2 = nullptr;  // 1/4分辨率
	Texture* m_bloomDownsample3 = nullptr;  // 1/8分辨率（可选）

	Texture* m_bloomTempA = nullptr;  // ← 1/4分辨率（复用于Blur Quarter + Upsample Quarter）
	Texture* m_bloomTempB = nullptr;  // ← 1/2分辨率（复用于Blur Half + Upsample Half）

	// === Shaders ===
	Shader* m_downsampleShader = nullptr;   // Downsample
	Shader* m_gaussianBlurHShader = nullptr; // Horizontal Blur
	Shader* m_gaussianBlurVShader = nullptr; // Vertical Blur
	Shader* m_upsampleAddShader = nullptr;  // Upsample + Add
	Shader* m_bloomBlitShader = nullptr;    // Final composite

	ConstantBuffer* m_blurParamsCBO = nullptr;

	// Parallax Bkg
	ParallaxBackground* m_parallaxBackground = nullptr;
	bool m_showParallaxDebugPanel = false;
};
