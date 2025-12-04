#include "GameMap.hpp"
#include "SuperChunk.hpp"
#include "CellChunk.hpp"
#include "GamePlayer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "GameMapDebugRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <set>
#include "RegionManager.hpp"
#include "Engine/Math/MathUtils.hpp"

GameMap::GameMap(GamePlayer* player, IntVec2 const& mapSizeInSuperChunks)
    : BaseMap(
        IntVec2(mapSizeInSuperChunks.x * CHUNKS_PER_SUPER_CHUNK * CHUNK_SIZE,
                mapSizeInSuperChunks.y * CHUNKS_PER_SUPER_CHUNK * CHUNK_SIZE),
        CHUNK_SIZE)
    , m_player(player)
    , m_superChunkGridSize(mapSizeInSuperChunks)
    , m_mapGenerator(nullptr)
	, m_regionManager(nullptr)           // ⭐ 新增
	, m_herringboneTileset(nullptr)      // ⭐ 新增
{
    DebuggerPrintf("=== GameMap Created ===\n");
    DebuggerPrintf("Super Chunk Grid: %d×%d\n", m_superChunkGridSize.x, m_superChunkGridSize.y);
    DebuggerPrintf("Total Chunks: %d×%d\n", 
        m_superChunkGridSize.x * CHUNKS_PER_SUPER_CHUNK,
        m_superChunkGridSize.y * CHUNKS_PER_SUPER_CHUNK);
    DebuggerPrintf("Total Cells: %d×%d\n", m_mapSize.x, m_mapSize.y);

    m_debugRenderer = new GameMapDebugRenderer(this);
}

GameMap::~GameMap()
{
    // Delete all super chunks
    for (auto& row : m_superChunks) {
        for (auto* superChunk : row) {
            delete superChunk;
        }
    }
    m_superChunks.clear();
    m_activeSuperChunks.clear();

    if (m_mapGenerator) {
        delete m_mapGenerator;
        m_mapGenerator = nullptr;
    }

	if (m_regionManager) {
		delete m_regionManager;
		m_regionManager = nullptr;
	}

    delete m_debugRenderer;
    m_debugRenderer = nullptr;
}

void GameMap::Initialize()
{
    m_activationRadius = 1;

	InitializePCGSystem();

    InitializeSuperChunks();
    
	if (m_player) 
    {
		m_lastPlayerSuperChunk = IntVec2(-999, -999);
		UpdateSuperChunkStreaming();
	}
}

void GameMap::SetHerringboneTileset(HerringboneTileset* tileset)
{
	m_herringboneTileset = tileset;

	// 如果RegionManager已经创建，更新其tileset
	if (m_regionManager) 
	{
		m_regionManager->SetTileset(tileset);
	}
}

void GameMap::InitializeSuperChunks()
{
    m_superChunks.resize(m_superChunkGridSize.y);

    for (int superY = 0; superY < m_superChunkGridSize.y; ++superY) {
        m_superChunks[superY].resize(m_superChunkGridSize.x);

        for (int superX = 0; superX < m_superChunkGridSize.x; ++superX) {
            IntVec2 superChunkCoords(superX, superY);
            m_superChunks[superY][superX] = new SuperChunk(superChunkCoords, this);
        }
    }

    DebuggerPrintf("Initialized %d super chunks\n", 
        m_superChunkGridSize.x * m_superChunkGridSize.y);
}

void GameMap::Update(float deltaTime)
{
    m_player->Update(deltaTime);

    UpdateSuperChunkStreaming();

    // Phase 5: Update active super chunks
    // UpdateCellsPhysInChunk();
}

void GameMap::Render() const
{
    g_theRenderer->BeginCamera(m_player->GetCamera());

    // Render all active super chunks
    for (SuperChunk* sc : m_activeSuperChunks) 
	{
        if (sc) 
		{
			if (IsSuperChunkVisible(sc, m_player->GetBaseCamBound()))
			{
				sc->SetIsVisible(true);
				sc->Render();
			}
			else
				sc->SetIsVisible(false);
        }
    }

    m_debugRenderer->Render(m_player->GetCamera());

    m_player->Render();

    g_theRenderer->EndCamera(m_player->GetCamera());
}

Rgba8 GameMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
    UNUSED(worldCoords);
    // Basic color based on material type
    return cell.m_color;
}

SuperChunk* GameMap::GetSuperChunkByCoords(IntVec2 const& superChunkCoords)
{
    if (!IsSuperChunkCoordsValid(superChunkCoords)) {
        return nullptr;
    }
    return m_superChunks[superChunkCoords.y][superChunkCoords.x];
}

SuperChunk* GameMap::GetSuperChunkByWorldPos(int worldX, int worldY)
{
    IntVec2 superChunkCoords = WorldToSuperChunk(worldX, worldY);
    return GetSuperChunkByCoords(superChunkCoords);
}

void GameMap::ActivateSuperChunk(IntVec2 const& superChunkCoords)
{
	SuperChunk* sc = GetSuperChunkByCoords(superChunkCoords);
	if (!sc || sc->IsActive()) {
		return;
	}

	// ⭐ 1. 激活SuperChunk（分配chunks）
	sc->Activate();
	m_activeSuperChunks.push_back(sc);

	// ⭐ 2. 生成该SuperChunk的cells
	GenerateSuperChunkCells(sc);

	DebuggerPrintf("Activated and generated super chunk (%d, %d)\n",
		superChunkCoords.x, superChunkCoords.y);
}

void GameMap::DeactivateSuperChunk(IntVec2 const& superChunkCoords)
{
    SuperChunk* sc = GetSuperChunkByCoords(superChunkCoords);
    if (!sc || !sc->IsActive()) {
        return;
    }

    sc->Deactivate();

    // Remove from active list
    auto it = std::find(m_activeSuperChunks.begin(), m_activeSuperChunks.end(), sc);
    if (it != m_activeSuperChunks.end()) {
        m_activeSuperChunks.erase(it);
    }

    DebuggerPrintf("Deactivated super chunk (%d, %d) - Total active: %d\n",
        superChunkCoords.x, superChunkCoords.y,
        static_cast<int>(m_activeSuperChunks.size()));
}

CellChunk* GameMap::GetChunkByWorldPos(int worldX, int worldY)
{
    if (!IsInBounds(worldX, worldY)) {
        return nullptr;
    }

    IntVec2 chunkCoords = CellChunk::WorldToChunkIndex(worldX, worldY);
    return GetChunkByGlobalCoords(chunkCoords);
}

CellChunk* GameMap::GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords)
{
    IntVec2 superChunkCoords = ChunkToSuperChunk(globalChunkCoords);
    SuperChunk* sc = GetSuperChunkByCoords(superChunkCoords);
    
    if (!sc) {
        return nullptr;
    }

    return sc->GetChunkByGlobalCoords(globalChunkCoords);
}

Cell& GameMap::GetCellInChunk(int worldX, int worldY)
{
    CellChunk* chunk = GetChunkByWorldPos(worldX, worldY);
    GUARANTEE_OR_DIE(chunk != nullptr,
        Stringf("Cannot access cell at invalid world position (%d, %d)", worldX, worldY));

    IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);
    return chunk->GetLocalCell(localCoords.x, localCoords.y);
}

const Cell& GameMap::GetCellInChunk(int worldX, int worldY) const
{
    CellChunk* chunk = const_cast<GameMap*>(this)->GetChunkByWorldPos(worldX, worldY);
    GUARANTEE_OR_DIE(chunk != nullptr,
        Stringf("Cannot access cell at invalid world position (%d, %d)", worldX, worldY));

    IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);
    return chunk->GetLocalCell(localCoords.x, localCoords.y);
}

// === Coordinate Conversion ===

IntVec2 GameMap::WorldToSuperChunk(int worldX, int worldY)
{
    // World → Chunk → Super Chunk
    constexpr int cellsPerSuperChunk = CHUNK_SIZE * CHUNKS_PER_SUPER_CHUNK;
    
    return IntVec2(
        worldX / cellsPerSuperChunk,
        worldY / cellsPerSuperChunk
    );
}

IntVec2 GameMap::SuperChunkToWorld(IntVec2 const& superChunkCoords)
{
    // Return bottom-left corner in world cells
    constexpr int cellsPerSuperChunk = CHUNK_SIZE * CHUNKS_PER_SUPER_CHUNK;
    
    return IntVec2(
        superChunkCoords.x * cellsPerSuperChunk,
        superChunkCoords.y * cellsPerSuperChunk
    );
}

IntVec2 GameMap::ChunkToSuperChunk(IntVec2 const& chunkCoords)
{
    return IntVec2(
        chunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
        chunkCoords.y / CHUNKS_PER_SUPER_CHUNK
    );
}

// === Generation Stubs (Phase 2-3) ===

void GameMap::GenerateSuperChunkCells(SuperChunk* sc)
{

}

void GameMap::GenerateMapPixels()
{
    // Phase 2: Initialize HerringboneMapGenerator
    // Will be implemented in Phase 2
}

void GameMap::GenerateCellsFromPixels()
{
    // Phase 3: Convert pixels to cells
    // Will be implemented in Phase 3
}

// === Update System Stub (Phase 5) ===

void GameMap::UpdateCellsPhysInChunk()
{
    // Phase 5: Update cells with phase-based scheduling
    // Will be implemented in Phase 5
}

// === Debug ===

void GameMap::RenderDebugInfo() const
{
    // Could add ImGui debug window here
    // For now, just console output is sufficient
}

// === Player Position Tracking ===

IntVec2 GameMap::GetPlayerChunkCoords() const
{
	if (!m_player) {
		return IntVec2(-1, -1);
	}

	Vec2 playerPos = m_player->GetPosition();
	int worldX = static_cast<int>(playerPos.x);
	int worldY = static_cast<int>(playerPos.y);

	return CellChunk::WorldToChunkIndex(worldX, worldY);
}

IntVec2 GameMap::GetPlayerSuperChunkCoords() const
{
	if (!m_player) {
		return IntVec2(-1, -1);
	}

	Vec2 playerPos = m_player->GetPosition();
	int worldX = static_cast<int>(playerPos.x);
	int worldY = static_cast<int>(playerPos.y);

	return WorldToSuperChunk(worldX, worldY);
}

CellChunk* GameMap::GetPlayerCurrentChunk() const
{
	if (!m_player) {
		return nullptr;
	}

	Vec2 playerPos = m_player->GetPosition();
	int worldX = static_cast<int>(playerPos.x);
	int worldY = static_cast<int>(playerPos.y);

	return const_cast<GameMap*>(this)->GetChunkByWorldPos(worldX, worldY);
}

SuperChunk* GameMap::GetPlayerCurrentSuperChunk() const
{
	if (!m_player) {
		return nullptr;
	}

	IntVec2 superChunkCoords = GetPlayerSuperChunkCoords();
	return const_cast<GameMap*>(this)->GetSuperChunkByCoords(superChunkCoords);
}

// === Private Helpers ===

bool GameMap::IsInBounds(int worldX, int worldY) const
{
    return (worldX >= 0 && worldX < m_mapSize.x &&
            worldY >= 0 && worldY < m_mapSize.y);
}

bool GameMap::IsSuperChunkCoordsValid(IntVec2 const& coords) const
{
    return (coords.x >= 0 && coords.x < m_superChunkGridSize.x &&
            coords.y >= 0 && coords.y < m_superChunkGridSize.y);
}

// === Streaming System ===

void GameMap::UpdateSuperChunkStreaming()
{
	if (!m_player) return;

	IntVec2 currentPlayerSuperChunk = GetPlayerSuperChunkCoords();

	// Only update if player has moved to a different super chunk
	if (currentPlayerSuperChunk == m_lastPlayerSuperChunk) {
		return;
	}

	DebuggerPrintf("Player moved to super chunk (%d, %d)\n",
		currentPlayerSuperChunk.x, currentPlayerSuperChunk.y);

	m_lastPlayerSuperChunk = currentPlayerSuperChunk;

	// Calculate which super chunks should be active
	std::vector<IntVec2> requiredSuperChunks;
	CalculateRequiredSuperChunks(currentPlayerSuperChunk, requiredSuperChunks);

	// Activate new super chunks
	ActivateRequiredSuperChunks(requiredSuperChunks);

	// Deactivate distant super chunks
	DeactivateDistantSuperChunks(requiredSuperChunks);
}

void GameMap::CalculateRequiredSuperChunks(IntVec2 const& playerSuperChunkCoords,
	std::vector<IntVec2>& outRequired) const
{
	outRequired.clear();

	// Calculate square region around player
	// radius = 1 → 3×3 grid (9 super chunks)
	// radius = 2 → 5×5 grid (25 super chunks)

	int minX = playerSuperChunkCoords.x - m_activationRadius;
	int maxX = playerSuperChunkCoords.x + m_activationRadius;
	int minY = playerSuperChunkCoords.y - m_activationRadius;
	int maxY = playerSuperChunkCoords.y + m_activationRadius;

	// Clamp to map bounds
	minX = (minX < 0) ? 0 : minX;
	maxX = (maxX >= m_superChunkGridSize.x) ? m_superChunkGridSize.x - 1 : maxX;
	minY = (minY < 0) ? 0 : minY;
	maxY = (maxY >= m_superChunkGridSize.y) ? m_superChunkGridSize.y - 1 : maxY;

	// Add all super chunks in the region
	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			outRequired.push_back(IntVec2(x, y));
		}
	}

	DebuggerPrintf("Calculated %d required super chunks (radius %d)\n",
		(int)outRequired.size(), m_activationRadius);
}

void GameMap::ActivateRequiredSuperChunks(std::vector<IntVec2> const& required)
{
	int activatedCount = 0;

	for (IntVec2 const& coords : required) {
		SuperChunk* sc = GetSuperChunkByCoords(coords);
		if (sc && !sc->IsActive()) {
			ActivateSuperChunk(coords);
			activatedCount++;
		}
	}

	if (activatedCount > 0) {
		DebuggerPrintf("Activated %d new super chunks\n", activatedCount);
	}
}

void GameMap::DeactivateDistantSuperChunks(std::vector<IntVec2> const& required)
{
	int deactivatedCount = 0;

	// Build a set for fast lookup
	std::set<IntVec2> requiredSet;
	for (IntVec2 const& coords : required) {
		requiredSet.insert(coords);
	}

	// Check all currently active super chunks
	std::vector<SuperChunk*> toDeactivate;
	for (SuperChunk* sc : m_activeSuperChunks) {
		if (!sc) continue;

		IntVec2 coords = sc->GetSuperChunkCoords();

		// If this super chunk is not in the required set, mark for deactivation
		if (requiredSet.find(coords) == requiredSet.end()) {
			toDeactivate.push_back(sc);
		}
	}

	// Deactivate marked super chunks
	for (SuperChunk* sc : toDeactivate) {
		DeactivateSuperChunk(sc->GetSuperChunkCoords());
		deactivatedCount++;
	}

	if (deactivatedCount > 0) {
		DebuggerPrintf("Deactivated %d distant super chunks\n", deactivatedCount);
	}
}

void GameMap::InitializePCGSystem()
{
	DebuggerPrintf("\n=== Initializing PCG System ===\n");

	// 1. 创建RegionManager
	m_worldSeed = 12345;  // 或者从配置读取
	m_regionManager = new RegionManager(m_worldSeed);

	// 2. 设置Tileset（如果已经设置）
	if (m_herringboneTileset) 
	{
		m_regionManager->SetTileset(m_herringboneTileset);
		DebuggerPrintf("Tileset set successfully\n");
	}
	else 
	{
		DebuggerPrintf("WARNING: No tileset set yet\n");
	}

	// 3. 初始化世界布局
	m_regionManager->InitializeWorldLayout();

	m_regionManager->PreGenerateAllRegions();

	DebuggerPrintf("PCG System initialized (seed=%u)\n\n", m_worldSeed);
}

// === Frustum Culling Implementation ===
void GameMap::GetVisibleSuperChunks(AABB2 const& viewBounds, std::vector<SuperChunk*>& outVisible) const
{
	outVisible.clear();

	for (SuperChunk* sc : m_activeSuperChunks) {
		if (sc && IsSuperChunkVisible(sc, viewBounds)) {
			outVisible.push_back(sc);
		}
	}
}

bool GameMap::IsSuperChunkVisible(SuperChunk const* sc, AABB2 const& viewBounds) const
{
	if (!sc) return false;

	AABB2 scBounds = sc->GetWorldBounds();
	return DoAABB2Overlap(scBounds, viewBounds);
}

bool GameMap::IsChunkVisible(CellChunk const* chunk, AABB2 const& viewBounds) const
{
	if (!chunk) return false;

	AABB2 chunkBounds = chunk->GetWorldBounds();
	return DoAABB2Overlap(chunkBounds, viewBounds);
}
