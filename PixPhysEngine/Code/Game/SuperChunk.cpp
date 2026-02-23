#include "SuperChunk.hpp"
#include "CellChunk.hpp"
#include "GameMap.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GamePlayer.hpp"
#include "ThirdParty/tracy/tracy/Tracy.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/ChunkRigidBodyManager.hpp"
#include "GPUCellData.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include "Engine/JobSystem/JobSystem.hpp"
#include "CellDataCollectJob.hpp"

extern Renderer* g_theRenderer;
extern JobSystem* g_theJobSystem;

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

    InitializeGPUBuffers();
}

SuperChunk::~SuperChunk()
{
	ShutdownGPUBuffers();

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

void SuperChunk::ActivateAllRigidBodies()
{
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
			if (m_chunks[y][x]) {
				m_chunks[y][x]->ActivateRigidBodies();
			}
		}
	}
}

void SuperChunk::DeactivateAllRigidBodies()
{
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
			if (m_chunks[y][x]) {
				m_chunks[y][x]->DeactivateRigidBodies();
			}
		}
	}
}

void SuperChunk::UpdateAllRigidBodies(float deltaTime)
{
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) 
    {
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) 
        {
			if (m_chunks[y][x]&&m_chunks[y][x]->GetRigidBodyManager()->IsRbDirty())
            {
				m_chunks[y][x]->UpdateRigidBodies(deltaTime);
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
    
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
			if (m_chunks[y][x]) {
                m_chunks[y][x]->MarkDirty();
                m_chunks[y][x]->ReserveVertexCapacity(CHUNK_SIZE * CHUNK_SIZE * 6);
                m_chunks[y][x]->ActivateRigidBodies();  // ← 新增：激活刚体
			}
		}
	}
    // Generation and rendering will be handled by GameMap
}

void SuperChunk::Deactivate()
{
    if (!m_isActive) return;

    m_isActive = false;

    // ← 新增：先失活所有刚体
    DeactivateAllRigidBodies();

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

CellChunk* SuperChunk::GetChunkByLocalCoords(IntVec2 const& localChunkCoords)
{
	if (!IsLocalChunkValid(localChunkCoords.x, localChunkCoords.y)) {
		return nullptr;
	}

	return m_chunks[localChunkCoords.y][localChunkCoords.x];
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

//void SuperChunk::RenderSolid() const
//{
//	//ZoneScoped;
//	//if (!m_isActive) return;
//
//	//AABB2 viewBounds = m_map->GetPlayer()->GetBaseCamBound();
//
//	//for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
//	//{
//	//	for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
//	//	{
//	//		if (m_chunks[y][x])
//	//		{
//	//			if (!m_map->IsChunkVisible(m_chunks[y][x], viewBounds))
//	//			{
//	//				m_chunks[y][x]->SetIsVisible(false);
//	//			}
//	//			else
//	//			{
//	//				m_chunks[y][x]->SetIsVisible(true);
//
//	//				g_theRenderer->SetModelConstants();
//	//				g_theRenderer->BindTexture(nullptr);
//	//				g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
//	//				g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
//
//	//				m_chunks[y][x]->RenderSolidCells();
//	//			}
//	//		}
//	//	}
//	//}
//
//	ZoneScoped;
//	if (!m_isActive) return;
//
//	// **阶段1: 收集所有可见Chunk的顶点**
//	m_batchedSolidVertices.clear();
//
//
//	AABB2 viewBounds = m_map->GetPlayer()->GetBaseCamBound();
//
//	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
//	{
//		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
//		{
//			CellChunk* chunk = m_chunks[y][x];
//			if (!chunk) continue;
//
//            {
//                ZoneScopedN("Visibility Check");
//                // 可见性检测
//                if (!m_map->IsChunkVisible(chunk, viewBounds))
//                {
//                    chunk->SetIsVisible(false);
//                    continue;
//                }
//
//                chunk->SetIsVisible(true);
//            }
//
//            {
//                ZoneScopedN("Collect verts");
//                // **收集顶点而非立即渲染**
//                const auto& vertices = chunk->GetSolidVertices();
//                if (!vertices.empty()) {
//                    m_batchedSolidVertices.insert(
//                        m_batchedSolidVertices.end(),
//                        vertices.begin(),
//                        vertices.end()
//                    );
//                }
//            }
//		}
//	}
//
//
//    {
//        ZoneScopedN("GPU Draw");
//        // **阶段2: 一次性渲染所有顶点（1个Draw Call）**
//        if (!m_batchedSolidVertices.empty())
//        {
//            g_theRenderer->SetModelConstants();
//            g_theRenderer->BindTexture(nullptr);
//            g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
//            g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
//
//            g_theRenderer->DrawVertexArray(m_batchedSolidVertices);
//        }
//    }
//
//}

void SuperChunk::RenderSolid() const
{
	ZoneScoped;
	if (!m_isActive) return;

	m_batchedSolidVertices.clear();
	m_visibleChunks.clear();
	AABB2 viewBounds = m_map->GetPlayer()->GetBaseCamBound();
    
	//for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) 
	//{
	//	for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
	//	{
	//		CellChunk* chunk = m_chunks[y][x];
	//		if (!chunk) continue;
	//		if (!m_map->IsChunkVisible(chunk, viewBounds)) 
	//		{
	//			chunk->SetIsVisible(false);
	//			continue;
	//		}
	//		chunk->SetIsVisible(true);
	//		m_visibleChunks.push_back(chunk);
	//	}
	//}

	//if (m_gpuBuffersInitialized) {
	//	ZoneScopedN("Update Cell Data");
	//	UpdateCellDataBuffer();
	//}

	// === 新增：第一遍快速计算总顶点数 ===
	std::vector<CellChunk*> visibleChunks;
	int totalVertexCount = 0;
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) {
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x) {
			CellChunk* chunk = m_chunks[y][x];
			if (!chunk) continue;
			if (!m_map->IsChunkVisible(chunk, viewBounds)) {
				chunk->SetIsVisible(false);
				continue;
			}
			chunk->SetIsVisible(true);
			totalVertexCount += chunk->GetSolidVertices().size();
            visibleChunks.push_back(chunk);
		}
	}

	// === 一次性预分配内存 ===
	m_batchedSolidVertices.reserve(totalVertexCount);

	// === 阶段3: 高效收集顶点（memcpy代替insert）===
	{
		ZoneScopedN("Collect Vertices");
		for (CellChunk* chunk : visibleChunks) {
			const auto& vertices = chunk->GetSolidVertices();
			if (!vertices.empty()) {
				size_t oldSize = m_batchedSolidVertices.size();
				m_batchedSolidVertices.resize(oldSize + vertices.size());
				std::memcpy(
					&m_batchedSolidVertices[oldSize],
					vertices.data(),
					vertices.size() * sizeof(Vertex_PCU)
				);
			}
		}
	}

	// === GPU Draw ===
	if (!m_batchedSolidVertices.empty()) {
		ZoneScopedN("GPU Draw");
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_theRenderer->DrawVertexArray(m_batchedSolidVertices);
	}
}

void SuperChunk::RenderSolid_GPU() const
{
	ZoneScoped;
	if (!m_isActive) return;
	if (!m_gpuBuffersInitialized) return;

	m_visibleChunks.clear();
	AABB2 viewBounds = m_map->GetPlayer()->GetBaseCamBound();
	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y) 
	{
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
		{
			CellChunk* chunk = m_chunks[y][x];
			if (!chunk) continue;
			if (!m_map->IsChunkVisible(chunk, viewBounds)) 
			{
				chunk->SetIsVisible(false);
				continue;
			}
			chunk->SetIsVisible(true);
			m_visibleChunks.push_back(chunk);
		}
	}
	 
	// === 阶段1: 上传Cell数据 ===
	{
		ZoneScopedN("Upload Cell Data");
		UpdateCellDataBuffer();
	}

	// === 阶段2: 重置IndirectArgs（CPU方式）===
	{
		ZoneScopedN("Reset IndirectArgs");
		uint32_t resetArgs[5] = { 0, 1, 0, 0, 0 };  // vertexCount=0, instanceCount=1
		g_theRenderer->InitializeIndirectArgsBuffer(m_cellIndirectArgs, resetArgs, sizeof(resetArgs));
	}

	// === 阶段3: Dispatch Compute Shader（生成顶点）===
	{
		ZoneScopedN("Populate VBO");

		// 绑定Input Buffer（SRV）
		g_theRenderer->BindStructuredBufferSRV(0, m_cellDataBuffer);  // t0

		// 绑定Output Buffers（UAV）
		g_theRenderer->BindVertexBufferUAV(0, m_cellVertexBuffer);  // u0
		g_theRenderer->BindIndirectArgsBufferUAV(1, m_cellIndirectArgs);  // u1 ← 使用专用API

		// 绑定Constant Buffer
		g_theRenderer->BindConstantBufferCS(0, m_cellPopulateCBO);
	
		// 绑定Compute Shader
		g_theRenderer->BindComputeShader(m_cellPopulateShader);

		// Dispatch（基于totalCells）
		// 从UpdateCellDataBuffer传递的常量中读取实际Cell数
		// 简化：使用最大值（后续优化可以读取实际值）
		// === DEBUG 8: Dispatch前的参数 ===
		uint32_t numThreadGroups = (m_lastUploadedCellCount + 255) / 256;
		g_theRenderer->Dispatch(numThreadGroups, 1, 1);
	}

	// 解绑
	g_theRenderer->UnbindComputeShader();
	g_theRenderer->UnbindStructuredBuffers();
	g_theRenderer->BindIndirectArgsBufferUAV(1, nullptr);  // ← 使用专用API
	g_theRenderer->BindVertexBufferUAV(0, nullptr);


	// === 阶段5: DrawIndirect渲染 ===
	{
		ZoneScopedN("DrawIndirect");

		/*Texture* rts[3] = { m_mainColorTexture, m_collisionTexture, m_bloomTexture };
		g_theRenderer->BeginMRTRender(rts, 3);*/
		//g_theRenderer->ClearScreen(Rgba8::GREEN);
		// 绑定Shader
		g_theRenderer->BindShader(m_cellMRTShader);

		// 设置渲染状态
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetModelConstants();

		// DrawIndexedIndirect
		g_theRenderer->DrawIndexedIndirect(
			m_cellVertexBuffer,
			m_cellIndexBuffer,
			m_cellIndirectArgs,
			0  // byteOffset
		);

		// 恢复默认RenderTarget
		//g_theRenderer->EndMRTRender();

		//g_theRenderer->ClearScreen(Rgba8::RED);
		// ← 新增：把主颜色RT拷贝到屏幕
		// TODO: 这里需要一个Blit函数
		//{
		//	ZoneScopedN("Blit to Screen");
		//	g_theRenderer->DrawFullscreenQuad(m_mainColorTexture);
		//}
	}
}

void SuperChunk::RenderLiquid() const
{
	if (!m_isActive) return;

	for (int y = 0; y < CHUNKS_PER_SUPER_CHUNK; ++y)
	{
		for (int x = 0; x < CHUNKS_PER_SUPER_CHUNK; ++x)
		{
			if (m_chunks[y][x]&&m_chunks[y][x]->GetIsVisible())
			{
                m_chunks[y][x]->RenderLiquidCells();
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
                m_chunks[y][x]->ClearVertex();
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

void SuperChunk::ResetAllUpdateFlags()
{
    ZoneScoped;
	for (auto& row : m_chunks) 
    {
		for (auto& chunk : row)
        {
            if(chunk->IsDirty())
			    chunk->ResetUpdateFlags();
		}
	}
}

bool SuperChunk::IsLocalChunkValid(int localChunkX, int localChunkY) const
{
    return (localChunkX >= 0 && localChunkX < CHUNKS_PER_SUPER_CHUNK &&
            localChunkY >= 0 && localChunkY < CHUNKS_PER_SUPER_CHUNK);
}

void SuperChunk::InitializeGPUBuffers()
{
	if (m_gpuBuffersInitialized) return;

	// === 1. 计算最大Cell数量 ===
	// 8×8 Chunk × 64×64
	m_totalCellsInSuperChunk = CHUNKS_PER_SUPER_CHUNK * CHUNKS_PER_SUPER_CHUNK * CHUNK_SIZE * CHUNK_SIZE;

	// === 2. 创建Cell数据Buffer（StructuredBuffer）===
	m_cellDataBuffer = g_theRenderer->CreateStructuredBuffer(
		(unsigned int)m_totalCellsInSuperChunk,
		sizeof(GPUCellData),  // 12 bytes/cell
		true  // CPU只写，GPU只读（SRV）
	);

	// === 3. 创建顶点Buffer（临时用StructuredBuffer）===
	// 用途：GPU Compute Shader生成的顶点数据
	// 最大顶点数 = 262,144 cells × 6 verts = 1,572,864 verts
	// 每个顶点：Position(12) + Color(4) + UV(8) + PhysicsType(4) + Emission(4) = 32 bytes
	struct Vertex_GPU {
		float position[3];    // 12 bytes
		unsigned int color;   // 4 bytes
		float uv[2];          // 8 bytes
		unsigned int physicsType; // 4 bytes
		float emission;       // 4 bytes
	};  // Total: 32 bytes

	size_t maxVertices = m_totalCellsInSuperChunk *4;
	m_cellVertexBuffer = g_theRenderer->CreateVertexBuffer(
		maxVertices,
		32,        // stride = sizeof(Vertex_GPU)
		false,     // isPerInstance
		true       // enableUAV ← 关键！
	);

	// === 创建IndexBuffer ===
	size_t maxQuads = m_totalCellsInSuperChunk;  // 每个Cell一个Quad
	m_cellIndexBuffer = g_theRenderer->CreateIndexBuffer((unsigned int)(maxQuads * 6));

	std::vector<unsigned int> indices;
	indices.reserve(maxQuads * 6);

	for (size_t i = 0; i < maxQuads; ++i)
	{
		unsigned int baseVertex = (unsigned int)(i * 4);  // ← 每个Quad 4个顶点

		// Triangle 1: 0-1-2
		indices.push_back(baseVertex + 0);
		indices.push_back(baseVertex + 1);
		indices.push_back(baseVertex + 2);

		// Triangle 2: 0-2-3
		indices.push_back(baseVertex + 0);
		indices.push_back(baseVertex + 2);
		indices.push_back(baseVertex + 3);
	}

	g_theRenderer->CopyGameIndexBufferToGPU(indices.data(), (unsigned int)indices.size(), m_cellIndexBuffer);

	// === 4. 创建IndirectArgs Buffer ===
	// 用途：存储DrawIndirect的参数
	m_cellIndirectArgs = g_theRenderer->CreateIndirectArgsBuffer(20);

	// === 5. 创建Populate Constant Buffer ===
	// 用途：传递常量给Compute Shader
	struct PopulateConstants {
		unsigned int totalCells;
		float padding[3];
	};
	m_cellPopulateCBO = g_theRenderer->CreateConstantBuffer(sizeof(PopulateConstants));

	// === 6. 创建MRT Textures =================================
	IntVec2 screenSize = g_theRenderer->GetScreenDimensions();
	// RT0: 主颜色
	m_mainColorTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize,
		"CellMainColorTexture"
	);

	// RT1: Collision Texture
	m_collisionTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize,
		"CellCollisionTexture"
	);

	// RT2: Bloom Texture
	m_bloomTexture = g_theRenderer->CreateOrGetRenderTargetTexture(
		screenSize,
		"CellBloomTexture"
	);

	DebuggerPrintf("Created MRT textures: %dx%d\n", screenSize.x, screenSize.y);

	// === 6. 加载Shaders（暂时跳过，Phase 3会创建）===
	m_cellPopulateShader = g_theRenderer->CreateComputeShaderFromFile("Data/Shaders/CellPopulateVBO");
	m_cellMRTShader = g_theRenderer->CreateShaderFromFile("Data/Shaders/CellDrawIndirect",VertexType::VERTEX_GPU_CELL);

	m_gpuBuffersInitialized = true;

	DebuggerPrintf("SuperChunk(%d,%d) GPU Buffers initialized: %d max cells\n",
		m_superChunkCoords.x, m_superChunkCoords.y, m_totalCellsInSuperChunk);
}

void SuperChunk::ShutdownGPUBuffers()
{
	if (!m_gpuBuffersInitialized) return;

	// === 删除所有GPU资源 ===
	delete m_cellDataBuffer;
	delete m_cellVertexBuffer;
	delete m_cellIndirectArgs;
	delete m_cellPopulateCBO;
	delete m_cellIndexBuffer;

	// === 设置为nullptr ===
	m_cellDataBuffer = nullptr;
	m_cellVertexBuffer = nullptr;
	m_cellIndirectArgs = nullptr;
	m_cellPopulateCBO = nullptr;
	m_cellIndexBuffer=nullptr;

	m_gpuBuffersInitialized = false;

	m_mainColorTexture = nullptr;
	m_collisionTexture = nullptr;
	m_bloomTexture = nullptr;

	DebuggerPrintf("SuperChunk(%d,%d) GPU Buffers shutdown\n",
		m_superChunkCoords.x, m_superChunkCoords.y);
}

//void SuperChunk::UpdateCellDataBuffer() const
//{
//	if (!m_gpuBuffersInitialized) return;
//	if (m_visibleChunks.empty()) return;
//
//	// === 1. 收集所有可见Chunk的Cell数据 ===
//	std::vector<GPUCellData> cellDataArray;
//	cellDataArray.reserve(m_visibleChunks.size() * 64 * 64);
//
//	for (CellChunk* chunk : m_visibleChunks) 
//	{
//		if (!chunk) continue;
//
//		// 遍历Chunk的64×64个Cell
//		for (int localY = 0; localY < 64; ++localY) 
//		{
//			for (int localX = 0; localX < 64; ++localX) 
//			{
//				const Cell& cell = chunk->GetLocalCell(localX, localY);
//				if (cell.IsEmpty()) continue;
//
//				// 计算世界坐标
//				IntVec2 worldPos = chunk->LocalToWorld(localX, localY);
//
//				// 获取Cell类型和颜色
//				const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);
//				uint8_t physicsType = (uint8_t)matDef.m_physicsType;
//
//				// 获取发光值
//				uint8_t emission = 0;
//				if (matDef.m_isHighTemp) 
//				{
//					emission = 63;  // 最大发光
//				}
//				else if (matDef.m_emissionValue > 0) 
//				{
//					emission = matDef.m_emissionValue;
//				}
//
//				// 获取颜色（包含Alpha）
//				Rgba8 cellColor = cell.m_color;
//
//				// 打包数据
//				GPUCellData cellData = GPUCellData::Pack(worldPos, cellColor, physicsType, emission);
//				cellDataArray.push_back(cellData);
//			}
//		}
//	}
//
//	m_lastUploadedCellCount = (uint32_t)cellDataArray.size();
//
//	// === 2. 上传到GPU ===
//	if (!cellDataArray.empty()) 
//	{
//		size_t dataSizeKB = (cellDataArray.size() * sizeof(GPUCellData)) / 1024;
//		DebuggerPrintf("SuperChunk uploaded %zu cells (%zu KB) to GPU\n",
//			cellDataArray.size(), dataSizeKB);
//
//		g_theRenderer->CopyDataToStructuredBuffer(
//			m_cellDataBuffer,
//			cellDataArray.data(),
//			(unsigned int)cellDataArray.size()
//		);
//	}
//
//	// === 3. 更新Populate Shader常量 ===
//	struct PopulateConstants 
//	{
//		unsigned int totalCells;
//		float padding[3];
//	};
//
//	PopulateConstants constants;
//	constants.totalCells = (unsigned int)cellDataArray.size();
//	constants.padding[0] = 0.0f;
//	constants.padding[1] = 0.0f;
//	constants.padding[2] = 0.0f;
//
//	DebuggerPrintf("[CPU] === About to upload CBO ===\n");
//	DebuggerPrintf("[CPU] constants.totalCells = %u\n", constants.totalCells);
//	DebuggerPrintf("[CPU] constants.padding = [%f, %f, %f]\n",
//		constants.padding[0], constants.padding[1], constants.padding[2]);
//	DebuggerPrintf("[CPU] sizeof(PopulateConstants) = %zu\n", sizeof(PopulateConstants));
//	DebuggerPrintf("[CPU] m_cellPopulateCBO address = %p\n", m_cellPopulateCBO);
//
//	// === 检查CBO是否为空 ===
//	if (!m_cellPopulateCBO) {
//		DebuggerPrintf("[CPU] ERROR: m_cellPopulateCBO is NULL!\n");
//		return;
//	}
//
//	DebuggerPrintf("[CPU] Calling CopyCPUToGPU...\n");
//	g_theRenderer->CopyConstantBufferToGPU(&constants, sizeof(PopulateConstants), m_cellPopulateCBO);
//	DebuggerPrintf("[CPU] CopyCPUToGPU completed\n");
//}

void SuperChunk::UpdateCellDataBuffer() const
{
	if (!m_gpuBuffersInitialized) return;
	if (m_visibleChunks.empty()) return;

	ZoneScoped;

	// === 阶段1: 创建并提交Jobs（并行收集每个Chunk的数据）===
	std::vector<CellDataCollectJob*> jobs;
	jobs.reserve(m_visibleChunks.size());


	for (CellChunk* chunk : m_visibleChunks)
	{
		if (!chunk) continue;

		CellDataCollectJob* job = new CellDataCollectJob(chunk);
		jobs.push_back(job);
		g_theJobSystem->QueueJob(job);
	}


	int completedCount = 0;
	while (completedCount < (int)jobs.size())
	{
		Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
		if (completedJob != nullptr)
		{
			completedCount++;
		}
		else
		{
			std::this_thread::yield();
		}
	}


	// === 阶段3: 合并结果（单线程）===
	std::vector<GPUCellData> cellDataArray;
	{
		ZoneScopedN("Merge Results");

		// 预估总大小
		size_t totalCells = 0;
		for (const auto* job : jobs) {
			totalCells += job->GetCollectedData().size();
		}
		cellDataArray.reserve(totalCells);

		// 合并
		for (const auto* job : jobs) {
			const auto& chunkData = job->GetCollectedData();
			cellDataArray.insert(cellDataArray.end(), chunkData.begin(), chunkData.end());
		}
	}

	// === 阶段4: 清理Jobs ===

	for (auto* job : jobs) {
		delete job;
	}
	jobs.clear();


	m_lastUploadedCellCount = (uint32_t)cellDataArray.size();

	if (cellDataArray.empty()) return;

	// === 阶段5: 上传到GPU ===
	g_theRenderer->CopyDataToStructuredBuffer(
		m_cellDataBuffer,
		cellDataArray.data(),
		m_lastUploadedCellCount
	);


	// === 阶段6: 更新CBO ===
	struct PopulateConstants {
		unsigned int totalCells;
		float padding[3];
	};

	PopulateConstants constants = { m_lastUploadedCellCount, {0, 0, 0} };
	g_theRenderer->CopyConstantBufferToGPU(&constants, sizeof(constants), m_cellPopulateCBO);

}