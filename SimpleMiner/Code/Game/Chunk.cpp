#include "Chunk.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "BlockDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "TerrainGenerator.hpp"
#include "BlockIterator.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

RandomNumberGenerator Chunk::s_rng;
Texture* Chunk::s_blockAtlasTexture = nullptr;

extern Renderer* g_theRenderer;

Chunk::Chunk(IntVec2 const& chunkCoords)
	:m_chunkCoords(chunkCoords),m_blocks(BLOCKS_PER_CHUNK)
{
	CalculateWorldBounds();
	AddVertsForAABB3DWireFrame(m_debugVertexArray, m_worldBounds, 0.05f);

	m_isDirty = true;
	m_needsSaving = false;

	// Initialize();
}

Chunk::~Chunk()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;
	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}

void Chunk::SetBlockAtlasTexture(Texture* texture)
{
	s_blockAtlasTexture = texture;
}

//void Chunk::Initialize()
//{
//	m_isDirty = true;
//	m_needsSaving = false;
//
//	m_vertexBuffer= g_theRenderer->CreateVertexBuffer(BLOCKS_PER_CHUNK*24, sizeof(Vertex_PCUTBN));
//	m_indexBuffer = g_theRenderer->CreateIndexBuffer(BLOCKS_PER_CHUNK*36);
//
//	//AddVertsForAABB3DWireFrame(m_debugVertexArray, m_worldBounds,0.05f);
//
//	// block generation
//	// if have file
//	std::string filename = "Saves/Chunk(" + std::to_string(m_chunkCoords.x) + "," + std::to_string(m_chunkCoords.y) + ").chunk";
//	// Check if chunk file exists
//	if (FileExists(filename))
//	{
//		// Load from file
//		if (LoadChunkFromFile(filename))
//		{
//			// Successfully loaded from file
//			m_needsSaving = false;  // File is up to date
//		}
//		else
//		{
//			// File exists but failed to load, generate terrain as fallback
//			printf("Warning: Failed to load chunk (%d,%d) from file, generating new terrain\n",
//				m_chunkCoords.x, m_chunkCoords.y);
//			TerrainGenerator::GenerateBlocksForChunk(this);
//		}
//	}
//	else
//	{
//		// else no file
//		TerrainGenerator::GenerateBlocksForChunk(this);
//	}
//
//	RebuildMeshWithCulling();
//}

//void Chunk::GenerateBlocks()
//{
//	static const Block airBlock = BlockDefinition::s_nameToIndexMap["Air"];
//	static const Block waterBlock = BlockDefinition::s_nameToIndexMap["Water"];
//	static const Block grassBlock = BlockDefinition::s_nameToIndexMap["Grass"];
//	static const Block dirtBlock = BlockDefinition::s_nameToIndexMap["Dirt"];
//
//	for (int i = 0; i < BLOCKS_PER_CHUNK; ++i)
//	{
//		m_blocks[i] = airBlock;
//	}
//
//	for (int y = 0; y < CHUNK_SIZE_Y; ++y)
//	{
//		for (int x = 0; x < CHUNK_SIZE_X; ++x)
//		{
//			int globalX = x + (m_chunkCoords.x << CHUNK_BITS_X);
//			int globalY = y + (m_chunkCoords.y << CHUNK_BITS_Y);
//			int terrainHeight = CalculateTerrainHeight(globalX, globalY);
//
//			int dirtDepth = s_rng.RollRandomIntInRange(3, 4);
//			int dirtStartLevel = terrainHeight - dirtDepth;
//
//			for (int z = 0; z < CHUNK_SIZE_Z; ++z)
//			{
//				Block block;
//
//				if (z > terrainHeight)
//				{
//					block = (z <= WATER_LEVEL) ? waterBlock : airBlock;
//				}
//				else if (z == terrainHeight)
//				{
//					block = grassBlock;
//				}
//				else if (z >= dirtStartLevel)
//				{
//					block = dirtBlock;
//				}
//				else
//				{
//					uint8_t blockType = GetRandomOreType();
//					block = Block(blockType);
//				}
//
//				int blockIndex = (z << (CHUNK_BITS_X + CHUNK_BITS_Y)) |
//					(y << CHUNK_BITS_X) | x;
//				m_blocks[blockIndex] = block;
//			}
//		}
//	}
//	m_isDirty = true;
//}

//void Chunk::RebuildMesh()
//{
//	if (!m_isDirty) return;
//
//	m_vertsCount = 0;
//	m_indicesCount = 0;
//	m_vertices.clear();
//	m_indices.clear();
//	m_vertices.reserve(BLOCKS_PER_CHUNK * 24); 
//	m_indices.reserve(BLOCKS_PER_CHUNK * 36);
//
//	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
//	{
//		IntVec3 blockCoords = IndexToLocalCoords(i);
//		Block block = m_blocks[i];
//		if (BlockDefinition::s_blockDefs[block.GetTypeIndex()].m_isVisible) {
//			AddBlockVerts(blockCoords, block);
//		}
//	}
//
// 	g_theRenderer->CopyGameVertexBufferToGPU(m_vertices.data(), (int)m_vertices.size(),m_vertexBuffer);
//	g_theRenderer->CopyGameIndexBufferToGPU(m_indices.data(), (int)m_indices.size(), m_indexBuffer);
//
//	m_isDirty = false;
//	m_indicesCount = (int)m_indices.size();
//	m_vertsCount = (int)m_vertices.size();
//}

void Chunk::RebuildMeshWithCulling()
{
	if (!m_isDirty) return;

	m_vertsCount = 0;
	m_indicesCount = 0;
	m_vertices.clear();
	m_indices.clear();

	if (m_vertexBuffer) {
		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer) {
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}

	int estimatedVertices = static_cast<int>(BLOCKS_PER_CHUNK * 6 * 4 * VISIBLE_FACE_RATIO);
	int estimatedIndices = static_cast<int>(BLOCKS_PER_CHUNK * 6 * 6 * VISIBLE_FACE_RATIO);
	m_vertices.reserve(estimatedVertices);
	m_indices.reserve(estimatedIndices);
	//-----
	
	for (int i = 0; i < BLOCKS_PER_CHUNK; i++)
	{
		IntVec3 blockCoords = IndexToLocalCoords(i);
		Block block = m_blocks[i];
		if (BlockDefinition::s_blockDefs[block.GetTypeIndex()].m_isVisible) {
			AddBlockVertsWithCulling(i, block);
		}
	}

	//-----
	m_isDirty = false;
	m_indicesCount = (int)m_indices.size();
	m_vertsCount = (int)m_vertices.size();

	CreateGPUResources();
	g_theRenderer->CopyGameVertexBufferToGPU(m_vertices.data(), (int)m_vertices.size(), m_vertexBuffer);
	g_theRenderer->CopyGameIndexBufferToGPU(m_indices.data(), (int)m_indices.size(), m_indexBuffer);

	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indices.clear();
	m_indices.shrink_to_fit();
}

void Chunk::RebuildDebugMesh()
{
}

void Chunk::Update()
{
}

void Chunk::Render() const
{
	if (!m_vertexBuffer || !m_indexBuffer) {
		return;  
	}

	if (m_indicesCount == 0) {
		return;  
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindTexture(s_blockAtlasTexture);
	g_theRenderer->DrawGameIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer);
}

void Chunk::RenderDebug() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_debugVertexArray);

	//DebugRenderLightingAdvanced(true,false,false,0.2f);
}

// Add to Chunk.cpp

#include "TerrainConfig.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"

void Chunk::RenderNoiseDebug() const
{
	TerrainConfig& config = TerrainConfig::GetInstance();

	// Only render if debug mode is active
	if (!config.m_debug.m_showNoiseDebug ||
		config.m_debug.m_activeDebugMode == NoiseDebugMode::NONE)
	{
		return;
	}

	// Get the top surface Z coordinate (chunk bound top)
	float topZ = m_worldBounds.m_maxs.z;

	std::vector<Vertex_PCU> debugVerts;
	debugVerts.reserve(CHUNK_SIZE_X * CHUNK_SIZE_Y * 6); // 2 triangles per quad

	// ========================================
	// Determine data source and visualization parameters
	// ========================================
	const float* dataSource = nullptr;
	float minValue = 0.0f;
	float maxValue = 1.0f;
	Rgba8 colorLow = Rgba8::BLACK;
	Rgba8 colorHigh = Rgba8::WHITE;

	switch (config.m_debug.m_activeDebugMode)
	{
		// ========================================
		// Raw Noise Values
		// ========================================
	case NoiseDebugMode::CONTINENT_RAW:
		dataSource = m_continentNoiseRaw;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::EROSION_RAW:
		dataSource = m_erosionNoiseRaw;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::PEAKS_VALLEYS_RAW:
		dataSource = m_pvNoiseRaw;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::TEMPERATURE_RAW:
		dataSource = m_temperatureNoiseRaw;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::HUMIDITY_RAW:
		dataSource = m_humidityNoiseRaw;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

		// ========================================
		// Spline-Mapped Values
		// ========================================
	case NoiseDebugMode::CONTINENT_OFFSET_MAPPED:
		dataSource = m_continentOffsetMapped;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::EROSION_OFFSET_MAPPED:
		dataSource = m_erosionOffsetMapped;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::PV_OFFSET_MAPPED:
		dataSource = m_pvOffsetMapped;
		minValue = -1.0f;
		maxValue = 1.0f;
		break;

	case NoiseDebugMode::CONTINENT_AMPLITUDE_MAPPED:
		dataSource = m_continentAmplitudeMapped;
		minValue = -1.f;
		maxValue = 1.f;
		break;

	case NoiseDebugMode::EROSION_AMPLITUDE_MAPPED:
		dataSource = m_erosionAmplitudeMapped;
		minValue = -1.f;
		maxValue = 1.f;
		break;

	// ========================================
	// NEW: Biome Level Visualization
	// ========================================
	case NoiseDebugMode::CONTINENT_LEVEL:
	{
		// 不使用 dataSource，直接在循环中处理
		dataSource = nullptr;
		break;
	}

	case NoiseDebugMode::EROSION_LEVEL:
	{
		dataSource = nullptr;
		break;
	}

	case NoiseDebugMode::PV_LEVEL:
	{
		dataSource = nullptr;
		break;
	}

	case NoiseDebugMode::TEMPERATURE_LEVEL:
	{
		dataSource = nullptr;
		break;
	}

	case NoiseDebugMode::HUMIDITY_LEVEL:
	{
		dataSource = nullptr;
		break;
	}

	case NoiseDebugMode::BIOME_TYPE:
	{
		dataSource = nullptr;
		break;
	}

	default:
		return; // Unknown mode, don't render
	}

	for (int y = 0; y < CHUNK_SIZE_Y; ++y)
	{
		for (int x = 0; x < CHUNK_SIZE_X; ++x)
		{
			// 转换到扩展噪声坐标
			int noiseExtX = x + MAX_TREE_RADIUS;
			int noiseExtY = y + MAX_TREE_RADIUS;
			int noiseIdx = NoiseExtendedCoordsToIndex(noiseExtX, noiseExtY);

			Rgba8 color;

			// ========================================
			// Determine color for this (x, y) position
			// ========================================
			if (dataSource != nullptr)
			{
				// Use new color-coded visualization
				float value = dataSource[noiseIdx];  // 使用扩展数组索引
				// Normalize to [0, 1] range
				float t = (value - minValue) / (maxValue - minValue);
				t = GetClamped(t, 0.0f, 1.0f);
				// Interpolate between low and high color
				color = Interpolate(colorLow, colorHigh, t);
			}
			else
			{
				switch (config.m_debug.m_activeDebugMode)
				{
				case NoiseDebugMode::CONTINENT_LEVEL:
				{
					int level = static_cast<int>(m_continentLevel[noiseIdx]);
					color = GetLevelColor(level, 7); // 7 levels
					break;
				}
				case NoiseDebugMode::EROSION_LEVEL:
				{
					int level = static_cast<int>(m_erosionLevel[noiseIdx]);
					color = GetLevelColor(level, 7); // 7 levels
					break;
				}
				case NoiseDebugMode::PV_LEVEL:
				{
					int level = static_cast<int>(m_pvLevel[noiseIdx]);
					color = GetLevelColor(level, 5); // 5 levels
					break;
				}
				case NoiseDebugMode::TEMPERATURE_LEVEL:
				{
					int level = static_cast<int>(m_temperatureLevel[noiseIdx]);
					color = GetLevelColor(level, 5); // 5 levels
					break;
				}
				case NoiseDebugMode::HUMIDITY_LEVEL:
				{
					int level = static_cast<int>(m_humidityLevel[noiseIdx]);
					color = GetLevelColor(level, 5); // 5 levels
					break;
				}
				case NoiseDebugMode::BIOME_TYPE:
				{
					color = GetBiomeColor(m_biomeType[noiseIdx]);
					break;
				}
				default:
					color = Rgba8::CYAN; // fallback
					break;
				}
			}

			// ========================================
			// Create quad geometry
			// ========================================
			Vec3 worldMin = m_worldBounds.m_mins;
			float worldX = worldMin.x + static_cast<float>(x);
			float worldY = worldMin.y + static_cast<float>(y);

			// Create quad vertices (top face)
			Vec3 bl(worldX, worldY, topZ);
			Vec3 br(worldX + 1.0f, worldY, topZ);
			Vec3 tr(worldX + 1.0f, worldY + 1.0f, topZ);
			Vec3 tl(worldX, worldY + 1.0f, topZ);

			// Add two triangles to form quad
			// Triangle 1: bl, br, tr
			debugVerts.push_back(Vertex_PCU(bl, color, Vec2(0.0f, 0.0f)));
			debugVerts.push_back(Vertex_PCU(br, color, Vec2(1.0f, 0.0f)));
			debugVerts.push_back(Vertex_PCU(tr, color, Vec2(1.0f, 1.0f)));

			// Triangle 2: bl, tr, tl
			debugVerts.push_back(Vertex_PCU(bl, color, Vec2(0.0f, 0.0f)));
			debugVerts.push_back(Vertex_PCU(tr, color, Vec2(1.0f, 1.0f)));
			debugVerts.push_back(Vertex_PCU(tl, color, Vec2(0.0f, 1.0f)));
		}
	}

	// ========================================
	// Render the debug visualization
	// ========================================
	if (!debugVerts.empty())
	{
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->BindTexture(nullptr); // No texture, just colors
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexArray((int)debugVerts.size(), debugVerts.data());
	}
}


Block Chunk::GetBlock(const IntVec3& localCoords) const
{
	if (!IsValidLocalCoords(localCoords)) {
		return Block(0); // return air
	}

	int index = LocalCoordsToIndex(localCoords);
	return m_blocks[index];
}

Block Chunk::GetBlock(int index) const
{
	return m_blocks[index];
}

Block& Chunk::GetBlockRef(int index)
{
	return m_blocks[index];
}

void Chunk::SetBlock(const IntVec3& localCoords, const Block& block)
{
	if (!IsValidLocalCoords(localCoords)) {
		return;
	}

	int index = LocalCoordsToIndex(localCoords);
	m_blocks[index] = block;
	m_isDirty = true;
}

void Chunk::SetBlock(int index, const Block& block)
{
	if (!IsValidIndex(index)) return;

	m_blocks[index] = block;
	m_isDirty = true;
}

// Block Chunk::GetBlockAtGlobalCoords(const IntVec3& globalCoords) const
// {
// 	return Block();
// }

// void Chunk::SetBlockAtGlobalCoords(const IntVec3& globalCoords, const Block& block)
// {
// 	
// }

// IntVec3 Chunk::GlobalToLocalCoords(const IntVec3& globalCoords) const
// {
// 	int localX = globalCoords.x - (m_chunkCoords.x * CHUNK_SIZE_X);
// 	int localY = globalCoords.y - (m_chunkCoords.y * CHUNK_SIZE_Y);
// 	int localZ = globalCoords.z;
// 
// 	return IntVec3(localX, localY, localZ);
// }

IntVec3 Chunk::LocalToGlobalCoords(const IntVec3& localCoords) const
{
	int globalX = localCoords.x + (m_chunkCoords.x << CHUNK_BITS_X);  
	int globalY = localCoords.y + (m_chunkCoords.y << CHUNK_BITS_Y);  
	int globalZ = localCoords.z;  
	return IntVec3(globalX, globalY, globalZ);
}

bool Chunk::IsValidLocalCoords(const IntVec3& localCoords) const
{
	return localCoords.x>=0&&localCoords.x<=CHUNK_MAX_X
		&& localCoords.y >= 0 && localCoords.y <= CHUNK_MAX_Y
		&& localCoords.z >= 0 && localCoords.z <= CHUNK_MAX_Z;
}

bool Chunk::IsValidIndex(int index) const
{
	return index>=0&&index<BLOCKS_PER_CHUNK;
}

Vec3 Chunk::LocalCoordsToWorldPos(const IntVec3& localCoords) const
{
	return Vec3(
		static_cast<float>((m_chunkCoords.x << CHUNK_BITS_X) + localCoords.x),  
		static_cast<float>((m_chunkCoords.y << CHUNK_BITS_Y) + localCoords.y),  
		static_cast<float>(localCoords.z)
	);
}

void Chunk::CreateGPUResources()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertsCount, sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indicesCount);
}

int Chunk::LocalCoordsToIndex(const IntVec3& localCoords) const
{
	//return localCoords.x + localCoords.y * CHUNK_SIZE_X + localCoords.z * CHUNK_SIZE_X * CHUNK_SIZE_Y;
	return localCoords.x | (localCoords.y << CHUNK_BITS_X)
		| (localCoords.z << (CHUNK_BITS_X + CHUNK_BITS_Y));
}

int Chunk::LocalCoordsToIndex(int x, int y, int z) const
{
	return x | (y << CHUNK_BITS_X)
		| (z << (CHUNK_BITS_X + CHUNK_BITS_Y));
}

int Chunk::IndexToLocalX(int index)
{
	return index & CHUNK_MASK_X;
}

int Chunk::IndexToLocalY(int index)
{
	return (index & CHUNK_MASK_Y) >> CHUNK_BITS_X;
}

int Chunk::IndexToLocalZ(int index)
{
	return (index & CHUNK_MASK_Z) >> (CHUNK_BITS_X + CHUNK_BITS_Y);
}

IntVec3 Chunk::IndexToLocalCoords(int index) const
{
	int x = index & CHUNK_MASK_X;                                    
	int y = (index & CHUNK_MASK_Y) >> CHUNK_BITS_X;                
	int z = (index & CHUNK_MASK_Z) >> (CHUNK_BITS_X + CHUNK_BITS_Y); 
	return IntVec3(x, y, z);
}

int Chunk::GlobalCoordsToIndex(const IntVec3& globalCoords) const
{
	int localX = globalCoords.x & CHUNK_MAX_X;
	int localY = globalCoords.y & CHUNK_MAX_Y;
	int localZ = globalCoords.z & CHUNK_MAX_Z;

	return (localZ << (CHUNK_BITS_X + CHUNK_BITS_Y)) |
		(localY << CHUNK_BITS_X) |
		localX;
}

int Chunk::GlobalCoordsToIndex(int x, int y, int z)
{
	int localX = x & CHUNK_MAX_X;
	int localY = y & CHUNK_MAX_Y;
	int localZ = z & CHUNK_MAX_Z;

	return (localZ << (CHUNK_BITS_X + CHUNK_BITS_Y)) |
		(localY << CHUNK_BITS_X) |
		localX;
}

IntVec2 Chunk::GetChunkCoords(Vec3 const& position)
{
	int globalX = static_cast<int>(floor(position.x));
	int globalY = static_cast<int>(floor(position.y));
	//int globalZ = static_cast<int>(floor(position.z));

	int chunkX, chunkY;

	// Correct handling for negative coordinates
	if (globalX >= 0) {
		chunkX = globalX >> CHUNK_BITS_X;
	}
	else {
		chunkX = (globalX + 1) / CHUNK_SIZE_X - 1;
	}

	if (globalY >= 0) {
		chunkY = globalY >> CHUNK_BITS_Y;
	}
	else {
		chunkY = (globalY + 1) / CHUNK_SIZE_Y - 1;
	}

	return IntVec2(chunkX, chunkY);
}

IntVec2 Chunk::GetChunkCoords(const IntVec3& globalCoords)
{
	int chunkX, chunkY;
	if (globalCoords.x >= 0) {
		chunkX = globalCoords.x >> CHUNK_BITS_X;  
	}
	else {
		chunkX = (globalCoords.x - CHUNK_MAX_X) >> CHUNK_BITS_X; 
	}

	if (globalCoords.y >= 0) {
		chunkY = globalCoords.y >> CHUNK_BITS_Y;  
	}
	else {
		chunkY = (globalCoords.y - CHUNK_MAX_Y) >> CHUNK_BITS_Y; 
	}

	return IntVec2(chunkX, chunkY);
}

IntVec2 Chunk::GetChunkCenter() const
{
	int centerX = m_chunkCoords.x * CHUNK_SIZE_X + CHUNK_SIZE_X / 2;
	int centerY = m_chunkCoords.y * CHUNK_SIZE_Y + CHUNK_SIZE_Y / 2;

	return IntVec2(centerX, centerY);
}

IntVec2 Chunk::GetChunkCenter(const IntVec2& chunkCoords)
{
	int centerX = chunkCoords.x * CHUNK_SIZE_X + CHUNK_SIZE_X / 2;
	int centerY = chunkCoords.y * CHUNK_SIZE_Y + CHUNK_SIZE_Y / 2;

	return IntVec2(centerX, centerY);
}

IntVec3 Chunk::GlobalCoordsToLocalCoords(const IntVec3& globalCoords)
{
	int localX = globalCoords.x & CHUNK_MAX_X;
	int localY = globalCoords.y & CHUNK_MAX_Y;
	int localZ = globalCoords.z & CHUNK_MAX_Z;
	return IntVec3(localX, localY, localZ);
}

IntVec3 Chunk::GetGlobalCoords(const IntVec2& chunkCoords, int blockIndex)
{
	int localX = blockIndex & CHUNK_MAX_X;                                    
	int localY = (blockIndex & CHUNK_MASK_Y) >> CHUNK_BITS_X;              
	int localZ = (blockIndex & CHUNK_MASK_Z) >> (CHUNK_BITS_X + CHUNK_BITS_Y); 

	int globalX = (chunkCoords.x << CHUNK_BITS_X) + localX;  
	int globalY = (chunkCoords.y << CHUNK_BITS_Y) + localY; 
	int globalZ = localZ;

	return IntVec3(globalX, globalY, globalZ);
}

IntVec3 Chunk::GetGlobalCoords(const IntVec2& chunkCoords, const IntVec3& localCoords)
{
	int globalX = (chunkCoords.x << CHUNK_BITS_X) + localCoords.x;  
	int globalY = (chunkCoords.y << CHUNK_BITS_Y) + localCoords.y;  
	int globalZ = localCoords.z; 

	return IntVec3(globalX, globalY, globalZ);
}

IntVec2 Chunk::GetGlobalCoords(int localX, int localY)
{
	int globalX = (m_chunkCoords.x << CHUNK_BITS_X) + localX;
	int globalY = (m_chunkCoords.y << CHUNK_BITS_Y) + localY;

	return IntVec2(globalX, globalY);
}

IntVec3 Chunk::GetGlobalCoords(const Vec3& position)
{
	int globalX = static_cast<int>(floor(position.x));
	int globalY = static_cast<int>(floor(position.y));
	int globalZ = static_cast<int>(floor(position.z));

	return IntVec3(globalX, globalY, globalZ);
}

//int Chunk::CalculateTerrainHeight(int globalX, int globalY) const
//{
//	int baseHeight = 50;
//	int xContribution = globalX / 3;
//	int yContribution = globalY / 5;
//	int randomVariation = s_rng.RollRandomIntInRange(0, 1);
//	return baseHeight + xContribution + yContribution + randomVariation;
//}
//
//uint8_t Chunk::GetRandomOreType() const
//{
//	float roll = s_rng.RollRandomFloatZeroToOne();
//
//	if (roll < 0.001f) { // 0.1%
//		return BlockDefinition::s_nameToIndexMap["Diamond"]; // Diamond
//	}
//	else if (roll < 0.006f) { // 0.5%
//		return BlockDefinition::s_nameToIndexMap["Gold"]; // Gold
//	}
//	else if (roll < 0.026f) { // 2%
//		return BlockDefinition::s_nameToIndexMap["Iron"];  // Iron
//	}
//	else if (roll < 0.076f) { // 5%
//		return BlockDefinition::s_nameToIndexMap["Coal"]; // Coal
//	}
//	else {
//		return BlockDefinition::s_nameToIndexMap["Stone"];  // Stone
//	}
//}
//
//void Chunk::AddBlockVerts(const IntVec3& localCoords, Block const& block)
//{
//	const BlockDefinition& def = BlockDefinition::s_blockDefs[block.GetTypeIndex()];
//
//	Vec3 blockBL = LocalCoordsToWorldPos(localCoords);
//
//	// +X (EAST)
//	Vec3 bl = blockBL + Vec3(1.0f, 0.0f, 0.0f);
//	Vec3 br = blockBL + Vec3(1.0f, 1.0f, 0.0f);
//	Vec3 tr = blockBL + Vec3(1.0f, 1.0f, 1.0f);
//	Vec3 tl = blockBL + Vec3(1.0f, 0.0f, 1.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(230, 230, 230), def.m_sideUVs);
//
//	// -X (WEST)
//	bl = blockBL + Vec3(0.0f, 1.0f, 0.0f);
//	br = blockBL + Vec3(0.0f, 0.0f, 0.0f);
//	tr = blockBL + Vec3(0.0f, 0.0f, 1.0f);
//	tl = blockBL + Vec3(0.0f, 1.0f, 1.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(230, 230, 230), def.m_sideUVs);
//
//	// +Y (NORTH)
//	bl = blockBL + Vec3(1.0f, 1.0f, 0.0f);
//	br = blockBL + Vec3(0.0f, 1.0f, 0.0f);
//	tr = blockBL + Vec3(0.0f, 1.0f, 1.0f);
//	tl = blockBL + Vec3(1.0f, 1.0f, 1.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(200, 200, 200), def.m_sideUVs);
//
//	// -Y (SOUTH)
//	bl = blockBL + Vec3(0.0f, 0.0f, 0.0f);
//	br = blockBL + Vec3(1.0f, 0.0f, 0.0f);
//	tr = blockBL + Vec3(1.0f, 0.0f, 1.0f);
//	tl = blockBL + Vec3(0.0f, 0.0f, 1.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(200, 200, 200), def.m_sideUVs);
//
//	// +Z (TOP)
//	bl = blockBL + Vec3(0.0f, 0.0f, 1.0f);
//	br = blockBL + Vec3(1.0f, 0.0f, 1.0f);
//	tr = blockBL + Vec3(1.0f, 1.0f, 1.0f);
//	tl = blockBL + Vec3(0.0f, 1.0f, 1.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(255, 255, 255), def.m_topUVs);
//
//	// -Z (BOTTOM)
//	bl = blockBL + Vec3(0.0f, 1.0f, 0.0f);
//	br = blockBL + Vec3(1.0f, 1.0f, 0.0f);
//	tr = blockBL + Vec3(1.0f, 0.0f, 0.0f);
//	tl = blockBL + Vec3(0.0f, 0.0f, 0.0f);
//	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//		Rgba8(255, 255, 255), def.m_bottomUVs);
//}

//void Chunk::AddBlockVertsWithCulling(int blockIndex,Block const& block)
//{
//	const BlockDefinition& def = BlockDefinition::s_blockDefs[block.GetTypeIndex()];
//	if (!def.m_isVisible) return;
//
//	BlockIterator iter(this, blockIndex);
//	IntVec3 localCoords = iter.GetLocalCoords();
//	Vec3 blockWorldPos = LocalCoordsToWorldPos(localCoords);
//
//	// +X
//	BlockIterator fwdX = iter.GetFwdX();
//	if (!fwdX.IsValid() || !fwdX.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
//		Vec3 br = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
//		Vec3 tr = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
//		Vec3 tl = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(230, 230, 230), def.m_sideUVs);
//	}
//
//	// -X
//	BlockIterator negX = iter.GetNegX();
//	if (!negX.IsValid() || !negX.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
//		Vec3 br = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
//		Vec3 tr = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
//		Vec3 tl = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(230, 230, 230), def.m_sideUVs);
//	}
//
//	// +Y
//	BlockIterator fwdY = iter.GetFwdY();
//	if (!fwdY.IsValid() || !fwdY.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
//		Vec3 br = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
//		Vec3 tr = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
//		Vec3 tl = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(200, 200, 200), def.m_sideUVs);
//	}
//
//	// -Y
//	BlockIterator negY = iter.GetNegY();
//	if (!negY.IsValid() || !negY.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
//		Vec3 br = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
//		Vec3 tr = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
//		Vec3 tl = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(200, 200, 200), def.m_sideUVs);
//	}
//
//	// +Z
//	BlockIterator fwdZ = iter.GetFwdZ();
//	if (!fwdZ.IsValid() || !fwdZ.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
//		Vec3 br = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
//		Vec3 tr = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
//		Vec3 tl = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(255, 255, 255), def.m_topUVs);
//	}
//
//	// -Z
//	BlockIterator negZ = iter.GetNegZ();
//	if (!negZ.IsValid() || !negZ.IsOpaque()) {
//		Vec3 bl = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
//		Vec3 br = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
//		Vec3 tr = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
//		Vec3 tl = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
//		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
//			Rgba8(255, 255, 255), def.m_bottomUVs);
//	}
//}

void Chunk::AddBlockVertsWithCulling(int blockIndex, Block const& block)
{
	const BlockDefinition& def = BlockDefinition::s_blockDefs[block.GetTypeIndex()];
	if (!def.m_isVisible) return;

	BlockIterator iter(this, blockIndex);
	IntVec3 localCoords = iter.GetLocalCoords();
	Vec3 blockWorldPos = LocalCoordsToWorldPos(localCoords);

	// +X face
	BlockIterator fwdX = iter.GetFwdX();
	if (!fwdX.IsValid() || !fwdX.IsOpaque()) {
		// Get neighbor block's light influences
		Block neighborBlock = fwdX.IsValid() ? fwdX.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;  // Normalize [0-15] to [0-1]
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;    // Normalize [0-15] to [0-1]
		uint8_t grayScale = 230;  // Face directional shading

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),  // Red channel: outdoor light
			static_cast<uint8_t>(indoorLight * 255.0f),   // Green channel: indoor light
			grayScale,                                     // Blue channel: directional shading
			255
		);

		Vec3 bl = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
		Vec3 br = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
		Vec3 tr = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
		Vec3 tl = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_sideUVs);
	}

	// -X face
	BlockIterator negX = iter.GetNegX();
	if (!negX.IsValid() || !negX.IsOpaque()) {
		Block neighborBlock = negX.IsValid() ? negX.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;
		uint8_t grayScale = 230;

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),
			static_cast<uint8_t>(indoorLight * 255.0f),
			grayScale,
			255
		);

		Vec3 bl = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
		Vec3 br = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
		Vec3 tr = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
		Vec3 tl = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_sideUVs);
	}

	// +Y face
	BlockIterator fwdY = iter.GetFwdY();
	if (!fwdY.IsValid() || !fwdY.IsOpaque()) {
		Block neighborBlock = fwdY.IsValid() ? fwdY.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;
		uint8_t grayScale = 200;

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),
			static_cast<uint8_t>(indoorLight * 255.0f),
			grayScale,
			255
		);

		Vec3 bl = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
		Vec3 br = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
		Vec3 tr = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
		Vec3 tl = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_sideUVs);
	}

	// -Y face
	BlockIterator negY = iter.GetNegY();
	if (!negY.IsValid() || !negY.IsOpaque()) {
		Block neighborBlock = negY.IsValid() ? negY.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;
		uint8_t grayScale = 200;

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),
			static_cast<uint8_t>(indoorLight * 255.0f),
			grayScale,
			255
		);

		Vec3 bl = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
		Vec3 br = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
		Vec3 tr = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
		Vec3 tl = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_sideUVs);
	}

	// +Z face (top)
	BlockIterator fwdZ = iter.GetFwdZ();
	if (!fwdZ.IsValid() || !fwdZ.IsOpaque()) {
		Block neighborBlock = fwdZ.IsValid() ? fwdZ.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;
		uint8_t grayScale = 255;

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),
			static_cast<uint8_t>(indoorLight * 255.0f),
			grayScale,
			255
		);

		Vec3 bl = blockWorldPos + Vec3(0.0f, 0.0f, 1.0f);
		Vec3 br = blockWorldPos + Vec3(1.0f, 0.0f, 1.0f);
		Vec3 tr = blockWorldPos + Vec3(1.0f, 1.0f, 1.0f);
		Vec3 tl = blockWorldPos + Vec3(0.0f, 1.0f, 1.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_topUVs);
	}

	// -Z face (bottom)
	BlockIterator negZ = iter.GetNegZ();
	if (!negZ.IsValid() || !negZ.IsOpaque()) {
		Block neighborBlock = negZ.IsValid() ? negZ.GetBlock() : Block(0);
		float outdoorLight = neighborBlock.GetOutdoorLightInfluence() / 15.0f;
		float indoorLight = neighborBlock.GetIndoorLightInfluence() / 15.0f;
		uint8_t grayScale = 255;

		Rgba8 vertexColor = Rgba8(
			static_cast<uint8_t>(outdoorLight * 255.0f),
			static_cast<uint8_t>(indoorLight * 255.0f),
			grayScale,
			255
		);

		Vec3 bl = blockWorldPos + Vec3(0.0f, 1.0f, 0.0f);
		Vec3 br = blockWorldPos + Vec3(1.0f, 1.0f, 0.0f);
		Vec3 tr = blockWorldPos + Vec3(1.0f, 0.0f, 0.0f);
		Vec3 tl = blockWorldPos + Vec3(0.0f, 0.0f, 0.0f);
		AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
			vertexColor, def.m_bottomUVs);
	}
}


void Chunk::CalculateWorldBounds()
{
	Vec3 minBound = Vec3((float)m_chunkCoords.x*(float)CHUNK_SIZE_X, (float)m_chunkCoords.y * (float)CHUNK_SIZE_Y, 0.f);
	Vec3 maxBound = minBound + Vec3((float)CHUNK_SIZE_X, (float)CHUNK_SIZE_Y, (float)CHUNK_SIZE_Z);
	m_worldBounds = AABB3(minBound, maxBound);
}

void Chunk::DebugRenderLightingAdvanced(bool showIndoor, bool showOutdoor, bool showZero) const
{
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		Block const& block = m_blocks[blockIndex];

		uint8_t indoorLight = block.GetIndoorLightInfluence();
		uint8_t outdoorLight = block.GetOutdoorLightInfluence();

		// Skip if both are zero and we're not showing zeros
		if (!showZero && indoorLight == 0 && outdoorLight == 0)
			continue;

		// Skip if not showing the type we have
		if (!showIndoor && outdoorLight == 0)
			continue;
		if (!showOutdoor && indoorLight == 0)
			continue;

		IntVec3 localCoords = IndexToLocalCoords(blockIndex);
		Vec3 blockWorldPos = LocalCoordsToWorldPos(localCoords);
		Vec3 centerPos = blockWorldPos + Vec3(0.5f, 0.5f, 0.5f);

		char text[16];
		sprintf_s(text, "%d,%d", indoorLight, outdoorLight);

		// Color based on dominant light source
		Rgba8 textColor = Rgba8::WHITE;
		if (indoorLight > outdoorLight)
			textColor = Rgba8::RED;      // Indoor dominant
		else if (outdoorLight > indoorLight)
			textColor = Rgba8::CYAN;     // Outdoor dominant
		else if (indoorLight > 0)
			textColor = Rgba8::YELLOW;   // Equal

		DebugAddWorldBillboardText(text, centerPos, 0.2f,Vec2(0.f,0.f),0.f);
	}
}

//int Chunk::SaveChunkToFile(std::string const& saveFolder)
//{
//	std::vector<uint8_t> byteBuffer;
//
//	//== header ==
//	ChunkFileHeader header;
//	header.m_fourCC[0] = 'G';
//	header.m_fourCC[1] = 'C';
//	header.m_fourCC[2] = 'H';
//	header.m_fourCC[3] = 'K';
//	header.m_version = 1;
//	header.m_chunkBitsX = CHUNK_BITS_X;
//	header.m_chunkBitsY = CHUNK_BITS_Y;
//	header.m_chunkBitsZ = CHUNK_BITS_Z;
//
//	byteBuffer.push_back(header.m_fourCC[0]);
//	byteBuffer.push_back(header.m_fourCC[1]);
//	byteBuffer.push_back(header.m_fourCC[2]);
//	byteBuffer.push_back(header.m_fourCC[3]);
//	byteBuffer.push_back(header.m_version);
//	byteBuffer.push_back(header.m_chunkBitsX);
//	byteBuffer.push_back(header.m_chunkBitsY);
//	byteBuffer.push_back(header.m_chunkBitsZ);
//
//	ChunkFileRun currentRun;
//	currentRun.blockType = m_blocks[0].GetTypeIndex();
//	currentRun.runLength = 0;
//
//	// Loop over each block in the chunk
//	for (int blockIndex = 0; blockIndex < BLOCKS_PER_CHUNK; blockIndex++)
//	{
//		uint8_t currentBlockType = m_blocks[blockIndex].GetTypeIndex();
//
//		// Check if run is complete
//		bool runComplete = false;
//
//		if (currentBlockType != currentRun.blockType)
//		{
//			// Current block type is not equal to the block type of the run
//			runComplete = true;
//		}
//		else if (blockIndex == BLOCKS_PER_CHUNK - 1)
//		{
//			// We are at the end of the chunk
//			currentRun.runLength++;  // Increment for the current block
//			runComplete = true;
//		}
//		else if (currentRun.runLength >= BLOCKS_PER_LAYER-1)
//		{
//			// The run length is at the max of 255
//			runComplete = true;
//		}
//		else
//		{
//			// Increment the length of the run
//			currentRun.runLength++;
//		}
//
//		// When the run is complete, push back its member variables into the byte buffer vector
//		if (runComplete)
//		{
//			byteBuffer.push_back(currentRun.blockType);
//			byteBuffer.push_back(currentRun.runLength);
//
//			// Start a new run by setting the type to the current block type and zeroing out the length
//			if (blockIndex < BLOCKS_PER_CHUNK - 1)  // Don't start new run if we're at the end
//			{
//				currentRun.blockType = currentBlockType;
//				currentRun.runLength = 1;  // Start with 1 for the current block
//			}
//		}
//	}
//
//	// Generate filename: "Chunk(x,y).chunk"
//	std::string filename = saveFolder + "/Chunk(" + std::to_string(m_chunkCoords.x) + "," + std::to_string(m_chunkCoords.y) + ").chunk";
//
//	// Call FileWriteFromBuffer with the byte buffer vector
//	return FileWriteFromBuffer(byteBuffer, filename);
//
//	return 0;
//}
//
//bool Chunk::LoadChunkFromFile(std::string const& filename)
//{
//	std::vector<uint8_t> fileBuffer;
//	int bytesRead = FileReadToBuffer(fileBuffer, filename);
//
//	if (bytesRead == -1)
//	{
//		printf("Failed to read chunk file: %s\n", filename.c_str());
//		return false;
//	}
//
//	// Check minimum file size
//	if (fileBuffer.size() < sizeof(ChunkFileHeader))
//	{
//		printf("Chunk file too small: %s (size: %zu, minimum: %zu)\n",
//			filename.c_str(), fileBuffer.size(), sizeof(ChunkFileHeader));
//		return false;
//	}
//
//	// Read and validate header
//	ChunkFileHeader header;
//	size_t offset = 0;
//
//	// Copy header data from buffer
//	header.m_fourCC[0] = fileBuffer[offset++];
//	header.m_fourCC[1] = fileBuffer[offset++];
//	header.m_fourCC[2] = fileBuffer[offset++];
//	header.m_fourCC[3] = fileBuffer[offset++];
//	header.m_version = fileBuffer[offset++];
//	header.m_chunkBitsX = fileBuffer[offset++];
//	header.m_chunkBitsY = fileBuffer[offset++];
//	header.m_chunkBitsZ = fileBuffer[offset++];
//
//	// Validate header
//	if (header.m_fourCC[0] != 'G' || header.m_fourCC[1] != 'C' ||
//		header.m_fourCC[2] != 'H' || header.m_fourCC[3] != 'K')
//	{
//		printf("Invalid chunk file header (bad fourCC): %s\n", filename.c_str());
//		return false;
//	}
//
//	if (header.m_version != 1)
//	{
//		printf("Unsupported chunk file version: %d in file %s\n",
//			header.m_version, filename.c_str());
//		return false;
//	}
//
//	if (header.m_chunkBitsX != 4 || header.m_chunkBitsY != 4 || header.m_chunkBitsZ != 7)
//	{
//		printf("Chunk dimensions mismatch in file %s (got %d,%d,%d, expected 4,4,7)\n",
//			filename.c_str(), header.m_chunkBitsX, header.m_chunkBitsY, header.m_chunkBitsZ);
//		return false;
//	}
//
//	// Decode RLE data
//	int blockIndex = 0;
//	//size_t totalRuns = (fileBuffer.size() - sizeof(ChunkFileHeader)) / sizeof(ChunkFileRun);
//
//	while (offset < fileBuffer.size() && blockIndex < BLOCKS_PER_CHUNK)
//	{
//		// Check if we have enough bytes for a complete run
//		if (offset + 1 >= fileBuffer.size())
//		{
//			printf("Incomplete run data in chunk file: %s\n", filename.c_str());
//			return false;
//		}
//
//		// Read run data
//		ChunkFileRun currentRun;
//		currentRun.blockType = fileBuffer[offset++];
//		currentRun.runLength = fileBuffer[offset++];
//
//		// Validate run length
//		if (currentRun.runLength == 0)
//		{
//			printf("Invalid run length (0) in chunk file: %s\n", filename.c_str());
//			return false;
//		}
//
//		// Check if run would exceed chunk bounds
//		if (blockIndex + currentRun.runLength > BLOCKS_PER_CHUNK)
//		{
//			printf("Run extends beyond chunk bounds in file %s (index: %d, length: %d, max: %d)\n",
//				filename.c_str(), blockIndex, currentRun.runLength, BLOCKS_PER_CHUNK);
//			return false;
//		}
//
//		// Apply run to blocks
//		for (int i = 0; i < currentRun.runLength; i++)
//		{
//			Block curBlock = Block(currentRun.blockType);
//			SetBlock(blockIndex, curBlock);
//			blockIndex++;
//		}
//	}
//
//	if (blockIndex != BLOCKS_PER_CHUNK)
//	{
//		printf("Block count mismatch in chunk file %s (got %d blocks, expected %d)\n",
//			filename.c_str(), blockIndex, BLOCKS_PER_CHUNK);
//		return false;
//	}
//
//	if (offset < fileBuffer.size())
//	{
//		printf("Warning: Extra data at end of chunk file: %s (%zu extra bytes)\n",
//			filename.c_str(), fileBuffer.size() - offset);
//	}
//
//	printf("Successfully loaded chunk from file: %s\n", filename.c_str());
//	return true;
//}
