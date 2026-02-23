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
#include "RegionDefinition.hpp"
#include "ConfigDrivenCellGenerator.hpp"
#include "GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "CellBehaviorSystemInChunk.hpp"
#include "ChunkPhysUpdateJob.hpp"
#include "Engine/JobSystem/JobSystem.hpp"
#include "ThirdParty/tracy/tracy/Tracy.hpp"
#include "ChunkRebuildVertsJob.hpp"
#include "ChunkResetUpdateFlagJob.hpp"
#include "ChunkChemicalUpdateJob.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include "RigidBodyObjectPool.hpp"
#include "Box2DDebugDrawManager.hpp"
#include "ChunkRigidBodyDetectionJob.hpp"
#include "ChunkRigidBodyManager.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Nova2D/Nova2DSystem.hpp"

extern InputSystem* g_theInput;
extern JobSystem* g_theJobSystem;
extern Nova2DSystem* g_nova2D;

GameMap::GameMap(GamePlayer* player, IntVec2 const& mapSizeInSuperChunks)
    : BaseMap(
        IntVec2(mapSizeInSuperChunks.x * CHUNKS_PER_SUPER_CHUNK * CHUNK_SIZE,
                mapSizeInSuperChunks.y * CHUNKS_PER_SUPER_CHUNK * CHUNK_SIZE),
        CHUNK_SIZE)
    , m_player(player)
    , m_superChunkGridSize(mapSizeInSuperChunks)
    , m_mapGenerator(nullptr)
	, m_regionManager(nullptr)           
	, m_herringboneTileset(nullptr)      
	, m_b2WorldId(b2_nullWorldId)           // ← 新增
	, m_rigidBodyObjectPool(nullptr)        // ← 新增
	, m_rigidBodyUpdateInterval(120)        // ← 新增
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

	// 2. 销毁对象池（会delete所有RigidBodyObject）
	if (m_rigidBodyObjectPool) {
		delete m_rigidBodyObjectPool;
		m_rigidBodyObjectPool = nullptr;
	}

	// 3. 销毁Box2D世界
	if (!B2_IS_NULL(m_b2WorldId)) {
		b2DestroyWorld(m_b2WorldId);
		m_b2WorldId = b2_nullWorldId;
	}

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

	m_cellMainColorTexture = nullptr;
	m_cellCollisionTexture = nullptr;
	m_cellBloomTexture = nullptr;
	m_bloomBlitShader = nullptr;

	m_bloomDownsample1 = nullptr;
	m_bloomDownsample2 = nullptr;
	m_bloomDownsample3 = nullptr;

	m_bloomTempA = nullptr;
	m_bloomTempB = nullptr;

	m_downsampleShader = nullptr;
	m_gaussianBlurHShader = nullptr;
	m_gaussianBlurVShader = nullptr;
	m_upsampleAddShader = nullptr;
	m_bloomBlitShader = nullptr;

	delete m_blurParamsCBO;
	m_blurParamsCBO = nullptr;


}

void GameMap::Initialize()
{
    m_activationRadius = 1;

	InitializeRegionDefs();

	InitializePCGSystem();

    InitializeSuperChunks();

	// ← 新增：创建物理世界和刚体系统
	CreateBox2DWorld();
	InitializeRigidBodySystem();
	Box2DDebugDrawManager::GetInstance().Initialize(g_theRenderer);

	if (m_player) 
    {
		m_lastPlayerSuperChunk = IntVec2(-999, -999);
		UpdateSuperChunkStreaming();
	}

	// 创建MRT Textures
	IntVec2 screenSize = g_theRenderer->GetScreenDimensions();

	m_cellMainColorTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize, "GameMap_CellMainColor");

	m_cellCollisionTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize, "GameMap_CellCollision");

	m_cellBloomTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize, "GameMap_CellBloom");

	// bloom
	m_bloomBlitShader = g_theRenderer->CreateShaderFromFile(
		"Data/Shaders/BlitWithBloom",
		VertexType::VERTEX_PCU
	);

	// === Bloom Pyramid ===
	IntVec2 half = IntVec2(screenSize.x / 2, screenSize.y / 2);
	IntVec2 quarter = IntVec2(screenSize.x / 4, screenSize.y / 4);
	IntVec2 eighth = IntVec2(screenSize.x / 8, screenSize.y / 8);

	m_bloomDownsample1 = g_theRenderer->CreateOrGetRenderTargetTexture(
		half, "Bloom_Half");
	m_bloomDownsample2 = g_theRenderer->CreateOrGetRenderTargetTexture(
		quarter, "Bloom_Quarter");
	m_bloomDownsample3 = g_theRenderer->CreateOrGetRenderTargetTexture(
		eighth, "Bloom_Eighth");

	m_bloomTempA = g_theRenderer->CreateOrGetRenderTargetTexture(quarter, "Bloom_TempA");
	m_bloomTempB = g_theRenderer->CreateOrGetRenderTargetTexture(half, "Bloom_TempB");

	// === Shaders ===
	m_downsampleShader = g_theRenderer->CreateShaderFromFile(
		"Data/Shaders/Downsample", VertexType::VERTEX_PCU);
	m_gaussianBlurHShader = g_theRenderer->CreateShaderFromFile(
		"Data/Shaders/GaussianBlurH", VertexType::VERTEX_PCU);
	m_gaussianBlurVShader = g_theRenderer->CreateShaderFromFile(
		"Data/Shaders/GaussianBlurV", VertexType::VERTEX_PCU);
	m_upsampleAddShader = g_theRenderer->CreateShaderFromFile(
		"Data/Shaders/UpsampleAdd", VertexType::VERTEX_PCU);

	// === Blur参数CBO ===
	m_blurParamsCBO = g_theRenderer->CreateConstantBuffer(16);
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

void GameMap::InitializeDebugDraw()
{
	//// 1. 使用默认初始化
	//m_debugDraw = b2DefaultDebugDraw();

	//// 2. 设置回调（注意 Fcn 后缀）
	//m_debugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
	//m_debugDraw.context = this;

	//// 3. 配置绘制选项
	//m_debugDraw.drawShapes = true;
	//m_debugDraw.drawJoints = false;
	//m_debugDraw.drawBounds = false;
	//m_debugDraw.drawMass = false;
	//m_debugDraw.drawContacts = false;
}

void GameMap::RenderRigidBodiesDebug() const
{
	//// TODO: 实现Box2D调试渲染
	//// 可以使用 b2World_Draw() 配合自定义的 b2DebugDraw

	//if (!B2_IS_NULL(m_b2WorldId)) 
	//{
	//	b2World_Draw(m_b2WorldId, &m_debugDraw);
	//}
	Box2DDebugDrawManager::GetInstance().DrawWorld(m_b2WorldId);
}

void GameMap::InitializeSuperChunks()
{
    m_superChunks.resize(m_superChunkGridSize.y);

    for (int superY = 0; superY < m_superChunkGridSize.y; ++superY) {
        m_superChunks[superY].resize(m_superChunkGridSize.x);

        for (int superX = 0; superX < m_superChunkGridSize.x; ++superX) {
            IntVec2 superChunkCoords(superX, superY);
            m_superChunks[superY][superX] = new SuperChunk(superChunkCoords, this);

			GenerateSuperChunkCells(m_superChunks[superY][superX]);
        }
    }

    DebuggerPrintf("Initialized %d super chunks\n", 
        m_superChunkGridSize.x * m_superChunkGridSize.y);
}

void GameMap::Update(float deltaTime)
{
	ZoneScoped;
	m_deltaTime = deltaTime;

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		g_nova2D->CreateInstance(0, m_player->GetPosition().x, m_player->GetPosition().y);
	}

	g_nova2D->Update(deltaTime,m_player->GetCamera(),m_cellCollisionTexture);

    m_player->Update(deltaTime);

    UpdateSuperChunkStreaming();

	UpdateActiveSuperChunks(deltaTime);

	// ← 新增：更新所有激活SuperChunk的刚体shape
	UpdateActiveSuperChunksRigidBodies(deltaTime);

	// ← 新增：更新物理世界
	UpdatePhysicsWorld(deltaTime);
	
	// 切换调试面板 (例如按 F3)
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_showDebugPanel = !m_showDebugPanel;
	}

	// 渲染 ImGui 面板
	if (m_showDebugPanel)
	{
		m_debugRenderer->RenderDebugUI();
	}
	if (m_showMatBrushPanel)
	{
		m_debugRenderer->RenderMatBrushUI(m_player);
	}
}

void GameMap::Render() const
{
	ZoneScoped;
	g_theRenderer->BeginCamera(m_player->GetCamera());

	Texture* rts[3] = { m_cellMainColorTexture, m_cellCollisionTexture, m_cellBloomTexture };
	g_theRenderer->BeginMRTRender(rts, 3);

	{
		ZoneScopedN("RenderSolid");
		RenderActiveSuperChunk();
	}

	{
		ZoneScopedN("RenderNova2D");
		g_nova2D->Render(m_player->GetCamera());
	}

	// === 3. EndMRT ===
	g_theRenderer->EndMRTRender();

	//g_theRenderer->ClearScreen(Rgba8::BLUE);
	// === 4. Blit主颜色到屏幕 ===
	
	//g_theRenderer->DrawFullscreenQuad(m_cellMainColorTexture);

	ApplyBloomEffect();

	g_theRenderer->EndCamera(m_player->GetCamera());
	//g_nova2D->Render(m_player->GetCamera());
	//g_theRenderer->DrawFullscreenQuad(
	//	m_bloomBlitShader,  // 自定义Shader
	//	{ m_cellMainColorTexture, m_cellBloomTexture }  // 双纹理
	//);

	//for (SuperChunk* sc : m_activeSuperChunks)
	//{
	//	if (sc&&sc->GetIsVisible())
	//	{
	//		sc->RenderLiquid();

	//	}
	//}
	g_theRenderer->BeginCamera(m_player->GetCamera());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(nullptr);
    m_debugRenderer->Render(m_player->GetCamera());

	//  RenderRigidBodiesDebug();

    m_player->Render();

    g_theRenderer->EndCamera(m_player->GetCamera());
}

Rgba8 GameMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
    UNUSED(worldCoords);
    // Basic color based on material type
    return cell.m_color;
}

void GameMap::PlaceMaterialInChunk(int worldX, int worldY, CellMatType type, bool isRb)
{
	Cell& cell = GetCell(worldX, worldY);
	CellChunk* chunk = GetChunkByWorldPos(worldX, worldY);

	if (!chunk) return;  // 安全检查

	//chunk->MarkDirty();

	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(this, worldX, worldY);

	if (cell.m_type.load() == CellMatType::MAT_EMPTY)
	{
		CellMatDef const& curDef = CellMatManager::GetMaterialDef(type);

		cell.m_isBelongRb = isRb;
		cell.m_type.store(type);
		cell.m_color = curDef.m_color;
		cell.m_lifeCountDown = curDef.m_lifeCountDown.GetRandomInRange(&Game::s_rng);
		cell.m_flameCountDown = curDef.m_flameCountDown.GetRandomInRange(&Game::s_rng);
		cell.m_dissolveCountDown = curDef.m_dissolveCountDowm.GetRandomInRange(&Game::s_rng);
		cell.m_corrosionCountDown = curDef.m_corrosionCountDown.GetRandomInRange(&Game::s_rng);

		if (curDef.m_physicsType == PhyType::PHY_STATIC_SOLID)
		{
			if (chunk->GetRigidBodyManager())
			{
				chunk->GetRigidBodyManager()->SetRbDirty(true);
			}
		}
		// 石头特殊处理
		//if (type == CellMatType::MAT_STONE)
		//{
		//	cell.m_isFreeFalling = false;
		//}
	}
	else
	{
		if (type == CellMatType::MAT_EMPTY)
		{
			cell.SetEmpty();
			if (chunk->GetRigidBodyManager())
			{
				chunk->GetRigidBodyManager()->SetRbDirty(true);
			}
		}
	}
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
	if (!sc || sc->IsActive()) 
	{
		return;
	}

	// ⭐ 1. 激活SuperChunk（分配chunks）
	sc->Activate();
	m_activeSuperChunks.push_back(sc);

	// ⭐ 2. 生成该SuperChunk的cells
	// GenerateSuperChunkCells(sc);
	
	//for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
	//{
	//	for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx)
	//	{
	//		CellChunk* chunk = sc->GetChunk(cx, cy);
	//		// #TODO: dont rebuild all, consider the visibility
	//		chunk->RebuildVertexWithNewColor();
	//	}
	//}

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

Cell& GameMap::GetCell(int worldX, int worldY)
{
	//ZoneScoped;

	// 1. 边界检查
	//if (!IsInBounds(worldX, worldY)) {
	//	GUARANTEE_OR_DIE(false,
	//		Stringf("GetCell: Out of bounds (%d, %d)", worldX, worldY));
	//}

	// 2. 获取Chunk（通过SuperChunk）
	CellChunk* chunk = GetChunkByWorldPos(worldX, worldY);
	GUARANTEE_OR_DIE(chunk != nullptr,
		Stringf("GetCell: Cannot find chunk at (%d, %d)", worldX, worldY));

	// 3. 转换为局部坐标并获取Cell
	IntVec2 localCoords = CellChunk::WorldToLocal(worldX, worldY);
	return chunk->GetLocalCell(localCoords.x, localCoords.y);
}

Cell const& GameMap::GetCell(int worldX, int worldY) const
{
	//ZoneScoped;

	return const_cast<GameMap*>(this)->GetCell(worldX, worldY);
}

CellChunk* GameMap::GetChunkByWorldPos(int worldX, int worldY)
{
	//ZoneScoped;

	if (!IsInBounds(worldX, worldY)) {
		return nullptr;
	}

	IntVec2 chunkCoords = CellChunk::WorldToChunkIndex(worldX, worldY);
	return GetChunkByGlobalCoords(chunkCoords);
}

CellChunk const* GameMap::GetChunkByWorldPos(int worldX, int worldY) const
{
	//ZoneScoped;

	return const_cast<GameMap*>(this)->GetChunkByWorldPos(worldX, worldY);
}

CellChunk* GameMap::GetChunk(int globalChunkX, int globalChunkY)
{
	// 计算SuperChunk坐标
	IntVec2 scCoords(
		globalChunkX / CHUNKS_PER_SUPER_CHUNK,
		globalChunkY / CHUNKS_PER_SUPER_CHUNK
	);

	// 获取SuperChunk
	SuperChunk* sc = GetSuperChunkByCoords(scCoords);
	if (!sc || !sc->IsActive()) {
		return nullptr;
	}

	// 计算局部坐标
	int localX = globalChunkX % CHUNKS_PER_SUPER_CHUNK;
	int localY = globalChunkY % CHUNKS_PER_SUPER_CHUNK;

	return sc->GetChunk(localX, localY);
}

bool GameMap::MSCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const
{
	if (!useFastVersion)
	{
		// 1. 边界检查（包含SuperChunk激活检查）

		
	}

	if (!IsInBounds(x, y)) {
		return false;
	}

	// 2. 获取目标Cell
	const Cell& targetCell = GetCell(x, y);
	const CellMatDef& targetMatDef = CellMatManager::GetMaterialDef(targetCell.m_type.load());
	float targetDensity = targetMatDef.m_density;

	// 3. MoveSolid可以移动到：
	//    - 空cell
	//    - 密度更小的液体（浮力）
	return targetCell.IsEmpty() ||
		((curDensity > targetDensity) &&
			targetMatDef.m_physicsType == PhyType::PHY_LIQUID);
}

bool GameMap::LiquidCanMoveTo(int x, int y, float curDensity, bool useFastVersion) const
{
	if (!useFastVersion)
	{
		// 1. 边界检查（包含SuperChunk激活检查）
		if (!IsInBounds(x, y)) {
			return false;
		}
	}

	// 2. 获取目标Cell密度
	const Cell& targetCell = GetCell(x, y);
	float targetDensity = CellMatManager::GetMaterialDef(targetCell.m_type.load()).m_density;

	// 3. Liquid可以移动到密度更小的位置
	return curDensity > targetDensity;
}

bool GameMap::HasSupport(int x, int y) const
{
	//if (!IsSuperChunkActiveAt(x, y - 1)) {
	//	return true;  // 防止粒子继续向未激活区域下落
	//}
	if (!IsInBounds(x, y - 1)) {
		return true;  // 防止粒子继续向未激活区域下落
	}

	// 正常检查下方是否有非空cell
	return !GetCell(x, y - 1).IsEmpty();
}

CellChunk* GameMap::GetChunkByGlobalCoords(IntVec2 const& globalChunkCoords)
{
	//ZoneScoped;
    //IntVec2 superChunkCoords = ChunkToSuperChunk(globalChunkCoords);
    //SuperChunk* sc = GetSuperChunkByCoords(superChunkCoords);
    //
    //if (!sc) {
    //    return nullptr;
    //}

    //return sc->GetChunkByGlobalCoords(globalChunkCoords);

	//====================
	int superChunkX = globalChunkCoords.x >> 3;
	int superChunkY = globalChunkCoords.y >> 3;

	if (superChunkX < 0 || superChunkX >= m_superChunkGridSize.x ||
		superChunkY < 0 || superChunkY >= m_superChunkGridSize.y) {
		return nullptr;
	}

	SuperChunk* sc = m_superChunks[superChunkY][superChunkX];
	if (!sc) {
		return nullptr;
	}

	// 直接计算本地坐标（假设 CHUNKS_PER_SUPER_CHUNK = 8）
	int localX = globalChunkCoords.x & 0x7;  // 等价于 % 8
	int localY = globalChunkCoords.y & 0x7;

	return sc->GetChunkByLocalCoords(IntVec2(localX,localY));
}


// === Coordinate Conversion ===

IntVec2 GameMap::WorldToSuperChunk(int worldX, int worldY)
{
	constexpr int cellsPerSuperChunk = CHUNK_SIZE * CHUNKS_PER_SUPER_CHUNK;

	// 使用floor除法处理负数
	int scX = (worldX >= 0) ? (worldX / cellsPerSuperChunk)
		: ((worldX - cellsPerSuperChunk + 1) / cellsPerSuperChunk);
	int scY = (worldY >= 0) ? (worldY / cellsPerSuperChunk)
		: ((worldY - cellsPerSuperChunk + 1) / cellsPerSuperChunk);

	return IntVec2(scX, scY);
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
    //return IntVec2(
    //    chunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
    //    chunkCoords.y / CHUNKS_PER_SUPER_CHUNK
    //);
	return IntVec2(
		chunkCoords.x >> 3,
		chunkCoords.y >> 3
	);
}

// === Generation Stubs (Phase 2-3) ===

void GameMap::GenerateSuperChunkCells(SuperChunk* sc)
{
	if (!sc) return;

	IntVec2 scCoords = sc->GetSuperChunkCoords();

	// === 1. 确定该 SuperChunk 所属的 Region ===
	Vec2 scWorldCenter = Vec2(
		SuperChunkToWorld(scCoords).x + (CHUNK_SIZE * CHUNKS_PER_SUPER_CHUNK) / 2.0f,
		SuperChunkToWorld(scCoords).y + (CHUNK_SIZE * CHUNKS_PER_SUPER_CHUNK) / 2.0f
	);

	RegionBounds regionBounds = m_regionManager->GetRegionBoundsForPosition(scWorldCenter);
	RegionGenerationData* regionData = m_regionManager->GetRegionData(regionBounds);

	if (!regionData) {
		DebuggerPrintf("ERROR: No region data found for super chunk (%d, %d)\n",
			scCoords.x, scCoords.y);
		return;
	}

	// === 2. 获取该 Region 的 RegionDefinition ===
	RegionDefinition const& regionDef = m_jungleRegion;

	DebuggerPrintf("Generating cells for SuperChunk (%d,%d) in region '%s'\n",
		scCoords.x, scCoords.y, regionDef.GetName().c_str());

	// === 3. 遍历该 SuperChunk 的所有 Chunks ===
	for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy) 
	{
		for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) 
		{
			CellChunk* chunk = sc->GetChunk(cx, cy);
			if (!chunk) continue;

			// === 4. 生成该 Chunk 的 cells ===
			ConfigDrivenCellGenerator::GenerateChunkCells(chunk, regionDef, regionData);

			// === 5. 重建渲染顶点 ===
			chunk->RebuildVertexWithNewColor();

			chunk->ClearDirty();
		}
	}

	DebuggerPrintf("Successfully generated cells for SuperChunk (%d,%d)\n",
		scCoords.x, scCoords.y);
}

void GameMap::UpdateCellsPhysInChunk()
{
	//ZoneScoped;

	// 1. 重置所有激活SuperChunk的update flags
	if (!m_useJSResetFlags)
	{
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			if (!sc) continue;
			sc->ResetAllUpdateFlags();
		}
	}
	else
	{
		std::vector<ChunkResetUpdateFlagJob*> jobs;
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			for (int i=0;i<CHUNKS_PER_SUPER_CHUNK;i++)
			{
				for (int j = 0; j < CHUNKS_PER_SUPER_CHUNK; j++)
				{
					CellChunk* curChunk = sc->GetChunk(i, j);
					if (curChunk && curChunk->IsDirty())
					{
						ChunkResetUpdateFlagJob* job = new ChunkResetUpdateFlagJob(curChunk, this);
						jobs.push_back(job);
						g_theJobSystem->QueueJob(job);
					}
				}
			}
		}
		int completedCount = 0;
		while (completedCount < (int)jobs.size())
		{
			Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
			if (completedJob != nullptr)
			{
				completedCount++;
				delete completedJob;
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}


	if (!m_useJobSystem)
	{
		// === 单线程版本 ===
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			UpdateSingleThreadSuperChunk(sc);
		}
	}
	else  // 多线程版本
	{
		// 2. 计算phase更新顺序
		int phases[4] = { 0, 1, 2, 3 };

		switch (m_updateOrder)
		{
		case UpdateOrder::FIXED:
			// 保持 0, 1, 2, 3
			break;

		case UpdateOrder::ROTATING: {
			static int startPhase = 0;
			for (int i = 0; i < 4; ++i)
			{
				phases[i] = (startPhase + i) % 4;
			}
			startPhase = (startPhase + 1) % 4;
			break;
		}

		case UpdateOrder::RANDOM: {
			// 随机打乱
			for (int i = 3; i > 0; --i)
			{
				int j = rand() % (i + 1);
				std::swap(phases[i], phases[j]);
			}
			break;
		}
		}

		// 3. ⭐ 按phase更新激活的SuperChunks
		for (int i = 0; i < 4; ++i)
		{
			UpdateSuperChunksOfPhase(phases[i]);
		}
	}
}

void GameMap::UpdateActiveSuperChunks(float deltaTime)
{
	//RebuildVerts();

	UpdateCellsPhysInChunk();

	UpdateCellsChemicalInChunk();
}

void GameMap::RebuildVerts()
{
	ZoneScoped;
	if (m_useJobSystemBuildVerts)
	{
		std::vector<ChunkRebuildVertsJob*> jobs;
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			if (!sc) continue;

			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
			{
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx)
				{
					CellChunk* chunk = sc->GetChunk(cx, cy);
					// #TODO: dont rebuild all, consider the visibility
					if (chunk && chunk->GetIsVisible())
					{
						ChunkRebuildVertsJob* job = new ChunkRebuildVertsJob(chunk);
						jobs.push_back(job);
						g_theJobSystem->QueueJob(job);
					}
				}
			}
		}

		// 2. 等待所有Jobs完成
		int completedCount = 0;
		while (completedCount < (int)jobs.size())
		{
			Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
			if (completedJob != nullptr)
			{
				completedCount++;
				delete completedJob;
			}
			else
			{
				std::this_thread::yield();
			}
		}


	}
	else
	{
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			if (!sc) continue;

			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
			{
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx)
				{
					CellChunk* chunk = sc->GetChunk(cx, cy);
					// #TODO: dont rebuild all, consider the visibility
					//if (chunk && (chunk->IsDirty() || chunk->IsChemDirty() || chunk->GetIsVisible()))
					if (chunk && chunk->GetIsVisible())
					{
						chunk->RebuildVertexWithNewColor();
					}
				}
			}
		}
	}
}

void GameMap::UpdateCellsChemicalInChunk()
{
	ZoneScoped;

	if (!m_useJSChemical)
	{
		// 只遍历激活的SuperChunks
		for (SuperChunk* sc : m_activeSuperChunks) {
			if (!sc || !sc->IsActive()) continue;

			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy) {
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) {
					CellChunk* chunk = sc->GetChunk(cx, cy);
					if (chunk&&chunk->GetIsVisible()) {
						UpdateSingleChunkChemical(chunk);
					}
				}
			}
		}
	}
	else
	{
		std::vector<ChunkChemicalUpdateJob*> jobs;

		// 1. 遍历所有激活的SuperChunks，创建Jobs
		for (SuperChunk* sc : m_activeSuperChunks)
		{
			if (!sc || !sc->IsActive()) continue;

			for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
			{
				for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx)
				{
					CellChunk* chunk = sc->GetChunk(cx, cy);

					if (chunk&&chunk->GetIsVisible()) //&&
						//chunk->GetIsVisible())
					{
						ChunkChemicalUpdateJob* job = new ChunkChemicalUpdateJob(chunk, this);
						jobs.push_back(job);
						g_theJobSystem->QueueJob(job);
					}
				}
			}
		}

		// 2. 等待所有Jobs完成
		int completedCount = 0;
		while (completedCount < (int)jobs.size())
		{
			Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
			if (completedJob != nullptr)
			{
				completedCount++;
				delete completedJob;
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}
}

//void GameMap::UpdateSingleChunkChemical(CellChunk* chunk)
//{
//	ZoneScoped;
//
//	chunk->ClearChemDirty();
//
//	static const IntVec2 directions[8] = {
//		IntVec2(1,0), IntVec2(-1,0), IntVec2(0,1), IntVec2(0,-1),
//		IntVec2(1,1), IntVec2(-1,1), IntVec2(1,-1), IntVec2(-1,-1)
//	};
//
//	for (int localY = 0; localY < CHUNK_SIZE; ++localY)
//	{
//		for (int localX = 0; localX < CHUNK_SIZE; localX++)
//		{
//			Cell& cell = chunk->GetLocalCell(localX, localY);
//			if (cell.IsEmpty()) continue;
//
//			IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
//			CellMatDef curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
//
//			// High Temperature
//			if (curCellDef.m_isHighTemp) {
//				chunk->MarkChemDirty();
//			}
//
//			// Life Countdown
//			if (cell.m_lifeCountDown == 0)
//			{
//				cell.SetToType(curCellDef.m_lifeEndMatType);
//				curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
//			}
//			else if (cell.m_lifeCountDown > 0)
//			{
//				cell.m_lifeCountDown--;
//			}
//
//			// Flammable
//			if (curCellDef.m_isFlammable)
//			{
//				if (cell.m_flameCountDown <= 0)
//				{
//					cell.SetToType(curCellDef.m_flammableType);
//					chunk->MarkChemDirty();
//					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
//				}
//				else
//				{
//					for (int i = 0; i < 8; i++)
//					{
//						IntVec2 neighborWorldPos = worldPos + directions[i];
//						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
//						{
//							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(
//								GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
//							if (neighborCellDef.m_isHighTemp)
//							{
//								cell.m_flameCountDown--;
//							}
//						}
//					}
//				}
//			}
//
//			// Dissolve
//			if (curCellDef.m_isDissolve)
//			{
//				if (cell.m_dissolveCountDown <= 0)
//				{
//					cell.SetToType(curCellDef.m_dissolveType);
//					chunk->MarkChemDirty();
//					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
//				}
//				else
//				{
//					for (int i = 0; i < 8; i++)
//					{
//						IntVec2 neighborWorldPos = worldPos + directions[i];
//						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
//						{
//							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(
//								GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
//							if (neighborCellDef.m_physicsType == PhyType::PHY_LIQUID)
//							{
//								cell.m_dissolveCountDown--;
//							}
//						}
//					}
//				}
//			}
//
//			// Corrosion
//			if (curCellDef.m_isCorroded)
//			{
//				if (cell.m_corrosionCountDown <= 0)
//				{
//					cell.SetToType(curCellDef.m_corrodeType);
//					chunk->MarkChemDirty();
//					curCellDef = CellMatManager::GetMaterialDef(cell.m_type);
//				}
//				else
//				{
//					for (int i = 0; i < 8; i++)
//					{
//						IntVec2 neighborWorldPos = worldPos + directions[i];
//						if (IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
//						{
//							CellMatDef neighborCellDef = CellMatManager::GetMaterialDef(
//								GetCell(neighborWorldPos.x, neighborWorldPos.y).m_type);
//							if (neighborCellDef.m_isAcid)
//							{
//								cell.m_corrosionCountDown--;
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}
void GameMap::UpdateSingleChunkChemical(CellChunk* chunk)
{
	ZoneScoped;
	chunk->ClearChemDirty();

	static const IntVec2 directions[8] = {
		IntVec2(1,0), IntVec2(-1,0), IntVec2(0,1), IntVec2(0,-1),
		IntVec2(1,1), IntVec2(-1,1), IntVec2(1,-1), IntVec2(-1,-1)
	};

	static uint32_t frameCounter = 0;
	frameCounter++;

	for (int localY = 0; localY < CHUNK_SIZE; ++localY)
	{
		for (int localX = 0; localX < CHUNK_SIZE; localX++)
		{
			if ((frameCounter & 0x3) != ((localX + localY) & 0x3)) {
				continue;
			}

			Cell& cell = chunk->GetLocalCell(localX, localY);
			if (cell.IsEmpty()) continue;

			IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
			CellMatDef curCellDef = CellMatManager::GetMaterialDef(cell.m_type.load());

			//  new 
			if (curCellDef.m_physicsType == PhyType::PHY_LIQUID)
			{
				if (IsInBounds(worldPos.x, worldPos.y + 1))
				{
					Cell& upCell = GetCell(worldPos.x, worldPos.y + 1);
					CellMatDef upCellDef = CellMatManager::GetMaterialDef(upCell.m_type.load());
					if (upCellDef.m_name!=curCellDef.m_name)
					{
						cell.m_color = Rgba8(cell.m_color.r, cell.m_color.g, cell.m_color.b, 255);
					}
					else
					{
						cell.m_color = Rgba8(cell.m_color.r, cell.m_color.g, cell.m_color.b, 50);
					}
				}
			}

			// High Temperature
			if (curCellDef.m_isHighTemp) {
				chunk->MarkChemDirty();
			}

			// Life Countdown
			if (cell.m_lifeCountDown == 0)
			{
				cell.SetToType(curCellDef.m_lifeEndMatType);
				continue; // 类型已改变，跳过后续检查
			}
			else if (cell.m_lifeCountDown > 0)
			{
				cell.m_lifeCountDown--;
			}

			// 合并邻居检查逻辑
			bool needsNeighborCheck = curCellDef.m_isFlammable ||
				curCellDef.m_isDissolve ||
				curCellDef.m_isCorroded||
				curCellDef.m_isHighTemp;

			if (needsNeighborCheck)
			{
				// 预先收集邻居信息
				bool hasHighTemp = false;
				bool hasLiquid = false;
				bool hasAcid = false;

				for (int i = 0; i < 8; i++)
				{
					IntVec2 neighborWorldPos = worldPos + directions[i];
					if (!IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
						continue;

					const Cell& neighborCell = GetCell(neighborWorldPos.x, neighborWorldPos.y);
					if (neighborCell.IsEmpty()) continue;
					const CellMatDef& neighborDef = CellMatManager::GetMaterialDef(neighborCell.m_type.load());

					// 一次遍历收集所有需要的信息
					if (!hasHighTemp && neighborDef.m_isHighTemp) hasHighTemp = true;
					if (!hasLiquid && neighborDef.m_physicsType == PhyType::PHY_LIQUID) hasLiquid = true;
					if (!hasAcid && neighborDef.m_isAcid) hasAcid = true;

					// 如果所有需要的信息都找到了，提前退出
					if (hasHighTemp && hasLiquid && hasAcid) break;
				}

				// Flammable
				if (curCellDef.m_isFlammable)
				{
					if (cell.m_flameCountDown <= 0)
					{
						cell.SetToType(curCellDef.m_flammableType);
						//chunk->MarkChemDirty();
						//chunk->MarkDirty();
						CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(this, worldPos.x, worldPos.y);
						if (chunk->GetRigidBodyManager())
						{
							chunk->GetRigidBodyManager()->SetRbDirty(true);
						}
						continue;
					}
					else if (hasHighTemp)
					{
						cell.m_flameCountDown--;
					}
				}

				// Dissolve
				if (curCellDef.m_isDissolve)
				{
					if (cell.m_dissolveCountDown <= 0)
					{
						cell.SetToType(curCellDef.m_dissolveType);
						//chunk->MarkChemDirty();
						//chunk->MarkDirty();
						CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(this, worldPos.x, worldPos.y);
						if (chunk->GetRigidBodyManager())
						{
							chunk->GetRigidBodyManager()->SetRbDirty(true);
						}
						continue;
					}
					else if (hasLiquid)
					{
						cell.m_dissolveCountDown--;
					}
				}

				// Corrosion
				if (curCellDef.m_isCorroded)
				{
					if (cell.m_corrosionCountDown <= 0)
					{
						cell.SetToType(curCellDef.m_corrodeType);
						CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(this, worldPos.x, worldPos.y);
						if (chunk->GetRigidBodyManager())
						{
							chunk->GetRigidBodyManager()->SetRbDirty(true);
						}

						// 消耗一个周围的腐蚀性cell
						for (int i = 0; i < 8; i++)
						{
							IntVec2 neighborWorldPos = worldPos + directions[i];
							if (!IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
								continue;

							Cell& neighborCell = GetCell(neighborWorldPos.x, neighborWorldPos.y);
							if (neighborCell.IsEmpty()) continue;
							const CellMatDef& neighborDef = CellMatManager::GetMaterialDef(neighborCell.m_type.load());

							if (neighborDef.m_isAcid)
							{
								neighborCell.SetEmpty();
								CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(this, neighborWorldPos.x, neighborWorldPos.y);
								break; // 只消耗一个
							}
						}
					}
					else if (hasAcid)
					{
						cell.m_corrosionCountDown--;
					}
				}
				// High Temperature
				if (curCellDef.m_isHighTemp) {
					chunk->MarkChemDirty();

					// 检查周围是否有非高温液体
					for (int i = 0; i < 8; i++)
					{
						IntVec2 neighborWorldPos = worldPos + directions[i];
						if (!IsInBounds(neighborWorldPos.x, neighborWorldPos.y))
							continue;

						const Cell& neighborCell = GetCell(neighborWorldPos.x, neighborWorldPos.y);
						if (neighborCell.IsEmpty()) continue;
						const CellMatDef& neighborDef = CellMatManager::GetMaterialDef(neighborCell.m_type.load());

						// 如果邻居是液体且不是高温
						if (neighborDef.m_physicsType == PhyType::PHY_LIQUID && !neighborDef.m_isHighTemp)
						{
							cell.m_lifeCountDown = 0;
							break; // 找到一个就足够了
						}
					}
				}
			}
		}
	}
}

void GameMap::CreateBox2DWorld()
{
	if (!B2_IS_NULL(m_b2WorldId)) {
		DebuggerPrintf("Warning: Box2D world already created\n");
		return;
	}

	// 创建Box2D世界
	b2SetLengthUnitsPerMeter(CELLS_PER_METER);
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = b2Vec2{ 0.0f, -GRAVITY_METERS };  // 重力方向向下

	worldDef.enableSleep = true;              // 允许物体休眠
	worldDef.enableContinuous = true;         // 启用连续碰撞检测
	worldDef.contactHertz = 30.0f;           // 接触刚度
	worldDef.contactDampingRatio = 10.0f;    // 接触阻尼

	m_b2WorldId = b2CreateWorld(&worldDef);

	if (B2_IS_NULL(m_b2WorldId)) {
		ERROR_AND_DIE("Failed to create Box2D world!");
	}

	DebuggerPrintf("Box2D world created successfully\n");
}

void GameMap::DestroyBox2DWorld()
{
	if (!B2_IS_NULL(m_b2WorldId)) {
		b2DestroyWorld(m_b2WorldId);
		m_b2WorldId = b2_nullWorldId;
		DebuggerPrintf("Box2D world destroyed\n");
	}
}

void GameMap::InitializeRigidBodySystem()
{
	GUARANTEE_OR_DIE(!B2_IS_NULL(m_b2WorldId), "Box2D world must be created before initializing rigid body system");

	if (m_rigidBodyObjectPool) {
		DebuggerPrintf("Warning: Rigid body object pool already initialized\n");
		return;
	}

	// 1. 创建全局对象池
	m_rigidBodyObjectPool = new RigidBodyObjectPool();
	m_rigidBodyObjectPool->Initialize(m_b2WorldId, 500);  // 初始容量500

	// 2. 为所有SuperChunk中的所有CellChunk初始化刚体系统
	int totalChunks = 0;
	for (auto& row : m_superChunks) 
	{
		for (auto* superChunk : row) 
		{
			if (!superChunk) continue;

			// 遍历SuperChunk中的所有Chunk
			for (int localY = 0; localY < CHUNKS_PER_SUPER_CHUNK; ++localY) 
			{
				for (int localX = 0; localX < CHUNKS_PER_SUPER_CHUNK; ++localX) 
				{
					CellChunk* chunk = superChunk->GetChunk(localX, localY);
					if (chunk) 
					{
						chunk->InitializeRigidBodySystem(m_rigidBodyObjectPool);
						totalChunks++;
					}
				}
			}
		}
	}

	DebuggerPrintf("Rigid body system initialized for %d chunks\n", totalChunks);
}

void GameMap::UpdatePhysicsWorld(float deltaTime)
{
	if (B2_IS_NULL(m_b2WorldId)) {
		return;
	}

	// Box2D推荐使用固定时间步长
	// 这里使用4个子步骤来提高稳定性
	int subStepCount = 4;
	b2World_Step(m_b2WorldId, deltaTime, subStepCount);
}

void GameMap::UpdateActiveSuperChunksRigidBodies(float deltaTime)
{
	//for (auto* superChunk : m_activeSuperChunks) 
	//{
	//	if (superChunk && superChunk->IsActive()) 
	//	{
			//superChunk->UpdateAllRigidBodies(deltaTime);
	//	}
	//}

	ZoneScoped;

	// 使用异步版本
	UpdateRigidBodiesAsync();

	// 或者使用同步版本（调试用）
	//UpdateRigidBodiesSync(deltaTime);
}



bool GameMap::IsSuperChunkActiveAt(int worldX, int worldY) const
{
	//ZoneScoped;
	// 1. 转换为SuperChunk坐标
	IntVec2 scCoords = WorldToSuperChunk(worldX, worldY);

	// 2. 检查SuperChunk坐标是否有效
	if (!IsSuperChunkCoordsValid(scCoords)) {
		return false;
	}

	// 3. 获取SuperChunk并检查激活状态
	SuperChunk* sc = const_cast<GameMap*>(this)->GetSuperChunkByCoords(scCoords);
	return (sc && sc->IsActive());
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
	//ZoneScoped;
	//return IsSuperChunkActiveAt(worldX, worldY);
	return (worldX >= m_curActiveBoundMin.x &&
		worldX < m_curActiveBoundMax.x &&
		worldY >= m_curActiveBoundMin.y &&
		worldY < m_curActiveBoundMax.y);
}

bool GameMap::IsSuperChunkCoordsValid(IntVec2 const& coords) const
{
    return (coords.x >= 0 && coords.x < m_superChunkGridSize.x &&
            coords.y >= 0 && coords.y < m_superChunkGridSize.y);
}

void GameMap::InitializeRegionDefs()
{
	if (m_jungleRegion.LoadFromXML("Data/RegionConfigs/JungleRegion.xml"))
	{
		m_jungleRegion.PrintDebugInfo();

		// 访问配置
		BaseLayerConfig const& baseLayer = m_jungleRegion.GetBaseLayer();

		// 查找特定颜色的彩色层
		Rgba8 redColor(47, 85, 76, 255);
		ColoredLayerConfig const* redLayer = m_jungleRegion.FindColoredLayer(redColor);

		if (redLayer) {
			DebuggerPrintf("Found red layer: %s\n", redLayer->name.c_str());
		}
	}
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

void GameMap::UpdateSuperChunksOfPhase(int phaseIndex)
{
	ZoneScoped;
	std::vector<ChunkPhysUpdateJob*> jobs;

	// 1. 遍历所有激活的SuperChunks，创建Jobs
	for (SuperChunk* sc : m_activeSuperChunks)
	{
		if (!sc || !sc->IsActive()) continue;

		for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy)
		{
			for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx)
			{
				CellChunk* chunk = sc->GetChunk(cx, cy);

				if (chunk &&
					chunk->GetPhaseIndex() == phaseIndex &&
					chunk->IsDirty())
				{
					ChunkPhysUpdateJob* job = new ChunkPhysUpdateJob(chunk, this);
					jobs.push_back(job);
					g_theJobSystem->QueueJob(job);
				}
			}
		}
	}

	// 2. 等待所有Jobs完成
	int completedCount = 0;
	while (completedCount < (int)jobs.size())
	{
		Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
		if (completedJob != nullptr)
		{
			completedCount++;
			delete completedJob;
		}
		else
		{
			std::this_thread::yield();
		}
	}
}

void GameMap::UpdateSingleThreadSuperChunk(SuperChunk* sc)
{
	if (!sc || !sc->IsActive()) 
	{
		return;
	}

	// 遍历该SuperChunk的所有chunks
	for (int cy = 0; cy < CHUNKS_PER_SUPER_CHUNK; ++cy) 
	{
		for (int cx = 0; cx < CHUNKS_PER_SUPER_CHUNK; ++cx) 
		{
			CellChunk* chunk = sc->GetChunk(cx, cy);
			if (!chunk||!chunk->IsDirty()) continue;
			
			UpdateSingleThreadChunk(chunk);
		}
	}
}

void GameMap::UpdateSingleThreadChunk(CellChunk* chunk)
{
	//ZoneScoped;
	chunk->ClearDirty();

	for (int localY = 0; localY < CHUNK_SIZE; ++localY) 
	{
		// 奇数行从左到右,偶数行从右到左
		if (localY % 2 == 0) 
		{
			// 从左到右
			for (int localX = 0; localX < CHUNK_SIZE; localX++) 
			{
				Cell& cell = chunk->GetLocalCell(localX, localY);
				if (cell.IsEmpty()) continue;
				if (cell.m_isBelongRb) continue;
				IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
				CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
			}
		}
		else 
		{
			// 从右到左
			for (int localX = CHUNK_SIZE - 1; localX >= 0; localX--) 
			{
				Cell& cell = chunk->GetLocalCell(localX, localY);
				if (cell.IsEmpty()) continue;
				if (cell.m_isBelongRb) continue;
				IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
				CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
			}
		}
	}

	//-------------------------------------------------------------------
	//chunk->ClearDirty();

	//// 获取脏区域边界（世界坐标）
	//IntVec2 dirtyMin = chunk->m_dirtyBoundMin;
	//IntVec2 dirtyMax = chunk->m_dirtyBoundMax;

	//// 转换为chunk的local坐标
	//IntVec2 localMin = chunk->WorldToLocal(dirtyMin.x, dirtyMin.y);
	//IntVec2 localMax = chunk->WorldToLocal(dirtyMax.x, dirtyMax.y);

	//// 限制在chunk范围内
	//localMin.x = std::max(0, localMin.x);
	//localMin.y = std::max(0, localMin.y);
	//localMax.x = std::min(CHUNK_SIZE - 1, localMax.x);
	//localMax.y = std::min(CHUNK_SIZE - 1, localMax.y);

	//// 只遍历脏区域
	//for (int localY = localMin.y; localY <= localMax.y; ++localY)
	//{
	//	// 奇数行从左到右，偶数行从右到左
	//	if (localY % 2 == 0)
	//	{
	//		// 从左到右
	//		for (int localX = localMin.x; localX <= localMax.x; localX++)
	//		{
	//			Cell& cell = chunk->GetLocalCell(localX, localY);
	//			if (cell.IsEmpty()) continue;
	//			if (cell.m_isBelongRb) continue;
	//			IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
	//			CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
	//		}
	//	}
	//	else
	//	{
	//		// 从右到左
	//		for (int localX = localMax.x; localX >= localMin.x; localX--)
	//		{
	//			Cell& cell = chunk->GetLocalCell(localX, localY);
	//			if (cell.IsEmpty()) continue;
	//			if (cell.m_isBelongRb) continue;
	//			IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
	//			CellBehaviorSystemInChunk::UpdateCell(cell, worldPos.x, worldPos.y, this);
	//		}
	//	}
	//}
}

void GameMap::CalculateRequiredSuperChunks(IntVec2 const& playerSuperChunkCoords,
	std::vector<IntVec2>& outRequired)
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
	for (int y = minY; y <= maxY; ++y) 
	{
		for (int x = minX; x <= maxX; ++x) 
		{
			outRequired.push_back(IntVec2(x, y));
		}
	}

	int worldMinX = minX * CELLS_PER_SUPER_CHUNK;
	int worldMaxX = (maxX + 1) * CELLS_PER_SUPER_CHUNK;
	int worldMinY = minY * CELLS_PER_SUPER_CHUNK;
	int worldMaxY = (maxY + 1) * CELLS_PER_SUPER_CHUNK;
	m_curActiveBoundMin = IntVec2(worldMinX, worldMinY);
	m_curActiveBoundMax = IntVec2(worldMaxX, worldMaxY);

	DebuggerPrintf("Calculated %d required super chunks (radius %d)\n",
		(int)outRequired.size(), m_activationRadius);
}

void GameMap::ActivateRequiredSuperChunks(std::vector<IntVec2> const& required)
{
	int activatedCount = 0;

	for (IntVec2 const& coords : required) 
	{
		SuperChunk* sc = GetSuperChunkByCoords(coords);
		if (sc && !sc->IsActive()) 
		{
			ActivateSuperChunk(coords);
			activatedCount++;
		}
	}

	if (activatedCount > 0) 
	{
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

void GameMap::UpdateRigidBodiesAsync()
{
	//ZoneScoped;

	//std::vector<ChunkRigidBodyDetectionJob*> jobs;
	//std::vector<ChunkRigidBodyManager*> managersToUpdate;
	//jobs.reserve(64);
	//managersToUpdate.reserve(64);

	////=========================================
	//// 阶段1：收集需要更新的chunk并提交Jobs
	////=========================================
	//std::vector<ChunkRigidBodyDetectionJob*> jobs;
	//jobs.reserve(200);  // 预估

	//{
	//	ZoneScopedN("Collect and Submit Jobs");

	//	for (auto* superChunk : m_activeSuperChunks)
	//	{
	//		if (!superChunk || !superChunk->IsActive()) {
	//			continue;
	//		}

	//		// 遍历SuperChunk内的所有Chunks
	//		for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
	//		{
	//			for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
	//			{
	//				CellChunk* chunk = superChunk->GetChunk(x, y);
	//				if (!chunk) continue;

	//				ChunkRigidBodyManager* rbManager = chunk->GetRigidBodyManager();
	//				if (!rbManager) continue;

	//				// 检查是否需要检测
	//				if (rbManager->ShouldDetectRigidBodies())
	//				{
	//					rbManager->ClearAll();
	//					// 创建Job并提交
	//					ChunkRigidBodyDetectionJob* job = new ChunkRigidBodyDetectionJob(rbManager);
	//					jobs.push_back(job);
	//					g_theJobSystem->QueueJob(job);
	//					//rbManager->MarkAsUpdated();
	//					rbManager->MarkAsProcessing();
	//				}
	//				else
	//				{
	//					// 不需要检测，只增加计数器
	//					//rbManager->IncrementFrameCounter();
	//				}
	//			}
	//		}
	//	}
	//}

	////m_totalJobsThisFrame = static_cast<int>(jobs.size());

	//if (jobs.empty()) {
	//	return;  // 没有需要更新的chunk
	//}

	////=========================================
	//// 阶段2：等待所有Jobs完成
	////=========================================
	//{
	//	ZoneScopedN("Wait for Jobs");

	//	int completedCount = 0;
	//	while (completedCount < static_cast<int>(jobs.size()))
	//	{
	//		Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
	//		if (completedJob != nullptr)
	//		{
	//			completedCount++;
	//		}
	//		else
	//		{
	//			// 没有完成的Job，让出CPU
	//			std::this_thread::yield();
	//		}
	//	}
	//}

	////=========================================
	//// 阶段3：在主线程创建Box2D对象
	////=========================================
	//{
	//	ZoneScopedN("Finish in Main Thread");

	//	for (ChunkRigidBodyDetectionJob* job : jobs)
	//	{
	//		job->FinishInMainThread();
	//		delete job;
	//	}
	//}

	//jobs.clear();



	ZoneScoped;

	std::vector<ChunkRigidBodyManager*> managersToUpdate;
	std::vector<ChunkRigidBodyDetectionJob*> jobs;

	managersToUpdate.reserve(64);
	jobs.reserve(64);

	//=========================================
	// 阶段1：收集需要更新的managers
	//=========================================
	{
		ZoneScopedN("Collect Managers");

		for (auto* superChunk : m_activeSuperChunks)
		{
			if (!superChunk || !superChunk->IsActive()) continue;

			for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
			{
				for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
				{
					CellChunk* chunk = superChunk->GetChunk(x, y);
					if (!chunk) continue;

					ChunkRigidBodyManager* rbManager = chunk->GetRigidBodyManager();
					if (!rbManager) continue;

					if (rbManager->ShouldDetectRigidBodies())
					{
						managersToUpdate.push_back(rbManager);
					}
					else
					{
						//rbManager->IncrementFrameCounter();
					}
				}
			}
		}
	}

	if (managersToUpdate.empty()) return;

	//=========================================
	// 阶段2：批量ClearAll（主线程，串行但集中）
	//=========================================
	{
		ZoneScopedN("Batch ClearAll");

		for (ChunkRigidBodyManager* manager : managersToUpdate)
		{
			manager->ClearAll();
			manager->MarkAsProcessing();
		}
	}

	//=========================================
	// 阶段3：批量提交Jobs（非常快）
	//=========================================
	{
		ZoneScopedN("Submit Jobs");

		for (ChunkRigidBodyManager* manager : managersToUpdate)
		{
			ChunkRigidBodyDetectionJob* job = new ChunkRigidBodyDetectionJob(manager);
			jobs.push_back(job);
			g_theJobSystem->QueueJob(job);
		}
	}

	//=========================================
	// 阶段4：流水线处理完成的Jobs
	//=========================================
	{
		ZoneScopedN("Process Jobs");

		int completedCount = 0;
		while (completedCount < static_cast<int>(jobs.size()))
		{
			Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
			if (completedJob != nullptr)
			{
				ChunkRigidBodyDetectionJob* rbJob = static_cast<ChunkRigidBodyDetectionJob*>(completedJob);
				rbJob->FinishInMainThread();
				delete rbJob;
				completedCount++;
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

	managersToUpdate.clear();
	jobs.clear();
}

void GameMap::UpdateRigidBodiesSync(float deltaTime)
{
	ZoneScoped;

	// 同步版本：用于对比性能或调试
	for (auto* superChunk : m_activeSuperChunks)
	{
		if (superChunk && superChunk->IsActive())
		{
			for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
			{
				for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
				{
					CellChunk* chunk = superChunk->GetChunk(x, y);
					if (chunk)
					{
						chunk->UpdateRigidBodies(deltaTime);
					}
				}
			}
		}
	}
}
void GameMap::RenderActiveSuperChunk() const
{
	// Render all active super chunks
	for (SuperChunk* sc : m_activeSuperChunks)
	{
		if (sc)
		{
			if (IsSuperChunkVisible(sc, m_player->GetBaseCamBound()))
			{
				sc->SetIsVisible(true);
				//sc->RenderSolid();
				sc->RenderSolid_GPU();
			}
			else
				sc->SetIsVisible(false);
		}
	}
}
void GameMap::ApplyBloomEffect() const
{
	IntVec2 screenSize = g_theRenderer->GetScreenDimensions();

	// ========== Phase 1: Downsample ==========

	// Downsample: Full → Half
	{
		IntVec2 halfSize = IntVec2(screenSize.x / 2, screenSize.y / 2);
		g_theRenderer->SetViewport(halfSize);  // ← 手动设置Viewport

		Texture* rt[1] = { m_bloomDownsample1 };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_cellBloomTexture });
		g_theRenderer->EndMRTRender();
	}

	// Downsample: Half → Quarter
	{
		IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
		g_theRenderer->SetViewport(quarterSize);  // ← 手动设置

		Texture* rt[1] = { m_bloomDownsample2 };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_bloomDownsample1 });
		g_theRenderer->EndMRTRender();
	}

	// Downsample: Quarter → Eighth
	{
		IntVec2 eighthSize = IntVec2(screenSize.x / 8, screenSize.y / 8);
		g_theRenderer->SetViewport(eighthSize);  // ← 手动设置

		Texture* rt[1] = { m_bloomDownsample3 };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_bloomDownsample2 });
		g_theRenderer->EndMRTRender();
	}

	// ========== Phase 2: Blur Eighth ==========

	IntVec2 eighthSize = IntVec2(screenSize.x / 8, screenSize.y / 8);
	g_theRenderer->SetViewport(eighthSize);  // ← Blur也需要

	// Blur参数
	struct BlurParams {
				float texelSizeX;
				float texelSizeY;
				float padding[2];
			};
	BlurParams params;
	params.texelSizeX = 1.0f / eighthSize.x;
	params.texelSizeY = 1.0f / eighthSize.y;
	g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);
	g_theRenderer->BindConstantBuffer(0, m_blurParamsCBO);

	// Horizontal
	{
		Texture* rt[1] = { m_bloomTempA };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomDownsample3 });
		g_theRenderer->EndMRTRender();
	}

	// Vertical
	{
		Texture* rt[1] = { m_bloomDownsample3 };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomTempA });
		g_theRenderer->EndMRTRender();
	}

	// ========== Phase 3: Upsample ==========

	// Upsample Eighth → Quarter
	{
		IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
		g_theRenderer->SetViewport(quarterSize);  // ← 切换到Quarter尺寸

		Texture* rt[1] = { m_bloomTempA };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_upsampleAddShader,
			{ m_bloomDownsample3, m_bloomDownsample2 });
		g_theRenderer->EndMRTRender();
	}

	// Blur Quarter (可选)
	{
		IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
		g_theRenderer->SetViewport(quarterSize);

		params.texelSizeX = 1.0f / quarterSize.x;
		params.texelSizeY = 1.0f / quarterSize.y;
		g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);

		// H
		{
			Texture* rt[1] = { m_bloomDownsample2 };
			g_theRenderer->BeginMRTRender(rt, 1, true, false);
			g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomTempA });
			g_theRenderer->EndMRTRender();
		}
		// V
		{
			Texture* rt[1] = { m_bloomTempA };
			g_theRenderer->BeginMRTRender(rt, 1, true, false);
			g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomDownsample2 });
			g_theRenderer->EndMRTRender();
		}
	}

	// Upsample Quarter → Half
	{
		IntVec2 halfSize = IntVec2(screenSize.x / 2, screenSize.y / 2);
		g_theRenderer->SetViewport(halfSize);  // ← 切换到Half尺寸

		Texture* rt[1] = { m_bloomTempB };
		g_theRenderer->BeginMRTRender(rt, 1, true, false);
		g_theRenderer->DrawFullscreenQuad(m_upsampleAddShader,
			{ m_bloomTempA, m_bloomDownsample1 });
		g_theRenderer->EndMRTRender();
	}

	// Blur Half (可选)
	{
		IntVec2 halfSize = IntVec2(screenSize.x / 2, screenSize.y / 2);
		g_theRenderer->SetViewport(halfSize);

		params.texelSizeX = 1.0f / halfSize.x;
		params.texelSizeY = 1.0f / halfSize.y;
		g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);

		// H
		{
			Texture* rt[1] = { m_bloomDownsample1 };
			g_theRenderer->BeginMRTRender(rt, 1, true, false);
			g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomTempB });
			g_theRenderer->EndMRTRender();
		}
		// V
		{
			Texture* rt[1] = { m_bloomTempB };
			g_theRenderer->BeginMRTRender(rt, 1, true, false);
			g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomDownsample1 });
			g_theRenderer->EndMRTRender();
		}
	}

	// ========== Phase 4: 合成（恢复全屏Viewport）==========

	g_theRenderer->SetViewport(screenSize);  // ← 恢复全屏

	g_theRenderer->DrawFullscreenQuad(m_bloomBlitShader,
		{ m_cellMainColorTexture, m_bloomTempB });
}
//void GameMap::ApplyBloomEffect() const
//{
//	ZoneScopedN("Bloom Effect");
//	IntVec2 screenSize = g_theRenderer->GetScreenDimensions();
//
//	// ========== Phase 1: Downsample Pyramid ==========
//
//	// Downsample: Full → Half
//	{
//		IntVec2 halfSize = IntVec2(screenSize.x / 2, screenSize.y / 2);
//		g_theRenderer->SetViewport(halfSize);
//
//		Texture* rt[1] = { m_bloomDownsample1 };
//		g_theRenderer->BeginMRTRender(rt, 1,true,false);
//		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_cellBloomTexture });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// Downsample: Half → Quarter
//	{
//		IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
//		g_theRenderer->SetViewport(quarterSize);
//
//		Texture* rt[1] = { m_bloomDownsample2 };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_bloomDownsample1 });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// Downsample: Quarter → Eighth
//	{
//		IntVec2 eighthSize = IntVec2(screenSize.x / 8, screenSize.y / 8);
//		g_theRenderer->SetViewport(eighthSize);
//
//		Texture* rt[1] = { m_bloomDownsample3 };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_downsampleShader, { m_bloomDownsample2 });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// ========== Phase 2: Blur最小级别 (Eighth) ==========
//
//	IntVec2 eighthSize = IntVec2(screenSize.x / 8, screenSize.y / 8);
//	g_theRenderer->SetViewport(eighthSize);
//
//	struct BlurParams {
//		float texelSizeX;
//		float texelSizeY;
//		float padding[2];
//	};
//	BlurParams params;
//
//	// Blur Eighth级别（使用TempA作为ping-pong buffer）
//	IntVec2 eighthSize = IntVec2(screenSize.x / 8, screenSize.y / 8);
//	params.texelSizeX = 1.0f / eighthSize.x;
//	params.texelSizeY = 1.0f / eighthSize.y;
//	g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);
//	g_theRenderer->BindConstantBuffer(0, m_blurParamsCBO);
//
//	// Horizontal Blur: Downsample3 → TempA
//	{
//		Texture* rt[1] = { m_bloomTempA };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomDownsample3 });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// Vertical Blur: TempA → Downsample3
//	{
//		Texture* rt[1] = { m_bloomDownsample3 };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomTempA });
//		g_theRenderer->EndMRTRender();
//	}
//	// 现在 Downsample3 = Eighth级别已模糊
//
//	// ========== Phase 3: Upsample + Blur链 ==========
//
//	// Step 1: Upsample Eighth → Quarter
//	// 输入: Downsample3 (1/8, 已模糊) + Downsample2 (1/4, 原始)
//	// 输出: TempA (1/4分辨率)
//	{
//		IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
//		g_theRenderer->SetViewport(quarterSize);
//
//		Texture* rt[1] = { m_bloomTempA };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_upsampleAddShader,
//			{ m_bloomDownsample3, m_bloomDownsample2 });
//		g_theRenderer->EndMRTRender();
//	}
//	// 现在 TempA = Quarter级别合成结果（放大的Eighth + 原始Quarter）
//
//	// (可选) Blur Quarter级别
//	// 更新Blur参数到Quarter分辨率
//	IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
//	g_theRenderer->SetViewport(quarterSize);
//
//	IntVec2 quarterSize = IntVec2(screenSize.x / 4, screenSize.y / 4);
//	params.texelSizeX = 1.0f / quarterSize.x;
//	params.texelSizeY = 1.0f / quarterSize.y;
//	g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);
//
//	// Horizontal Blur: TempA → Downsample2 (复用)
//	{
//		Texture* rt[1] = { m_bloomDownsample2 };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomTempA });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// Vertical Blur: Downsample2 → TempA
//	{
//		Texture* rt[1] = { m_bloomTempA };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomDownsample2 });
//		g_theRenderer->EndMRTRender();
//	}
//	// 现在 TempA = Quarter级别已模糊的合成结果
//
//	// Step 2: Upsample Quarter → Half
//	// 输入: TempA (1/4, 已模糊) + Downsample1 (1/2, 原始)
//	// 输出: TempB (1/2分辨率)
//	{
//		Texture* rt[1] = { m_bloomTempB };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_upsampleAddShader,
//			{ m_bloomTempA, m_bloomDownsample1 });
//		g_theRenderer->EndMRTRender();
//	}
//	// 现在 TempB = Half级别合成结果
//
//	// (可选) Blur Half级别
//	// 更新Blur参数到Half分辨率
//	IntVec2 halfSize = IntVec2(screenSize.x / 2, screenSize.y / 2);
//	params.texelSizeX = 1.0f / halfSize.x;
//	params.texelSizeY = 1.0f / halfSize.y;
//	g_theRenderer->CopyConstantBufferToGPU(&params, sizeof(BlurParams), m_blurParamsCBO);
//
//	// Horizontal Blur: TempB → Downsample1 (复用)
//	{
//		Texture* rt[1] = { m_bloomDownsample1 };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurHShader, { m_bloomTempB });
//		g_theRenderer->EndMRTRender();
//	}
//
//	// Vertical Blur: Downsample1 → TempB
//	{
//		Texture* rt[1] = { m_bloomTempB };
//		g_theRenderer->BeginMRTRender(rt, 1, true, false);
//		g_theRenderer->DrawFullscreenQuad(m_gaussianBlurVShader, { m_bloomDownsample1 });
//		g_theRenderer->EndMRTRender();
//	}
//	// 现在 TempB = Half级别已模糊的合成结果
//
//	// ========== Phase 4: 最终合成 ==========
//
//	g_theRenderer->DrawFullscreenQuad(m_bloomBlitShader,
//		{ m_cellMainColorTexture, m_bloomTempB });  // ← 使用TempB（最终结果）
//}

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
