#include "Chunk.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "BlockDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"

RandomNumberGenerator Chunk::s_rng;
Texture* Chunk::s_blockAtlasTexture = nullptr;

extern Renderer* g_theRenderer;

Chunk::Chunk(IntVec2 const& chunkCoords)
	:m_chunkCoords(chunkCoords),m_blocks(BLOCKS_PER_CHUNK)
{
	CalculateWorldBounds();
	Initialize();
}

Chunk::~Chunk()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
}

void Chunk::SetBlockAtlasTexture(Texture* texture)
{
	s_blockAtlasTexture = texture;
}

void Chunk::Initialize()
{
	m_vertexBuffer= g_theRenderer->CreateVertexBuffer(BLOCKS_PER_CHUNK*24, sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(BLOCKS_PER_CHUNK*36);

	GenerateBlocks();

	RebuildMesh();
}

void Chunk::GenerateBlocks()
{
	for (int i = 0; i < BLOCKS_PER_CHUNK; ++i) 
	{
		m_blocks[i] = BlockDefinition::s_nameToIndexMap["Air"]; // initial all blocks to air
	}

	for (int x = 0; x < CHUNK_SIZE_X; ++x) 
	{
		for (int y = 0; y < CHUNK_SIZE_Y; ++y) 
		{
			IntVec3 localCoords(x, y, 0);
			IntVec3 globalCoords = LocalToGlobalCoords(localCoords);
			int terrainHeight = CalculateTerrainHeight(globalCoords.x, globalCoords.y);

			for (int z = 0; z < CHUNK_HEIGHT; ++z)
			{
				IntVec3 coords(x, y, z);
				Block block;

				if (z > terrainHeight) 
				{
					if (z <= WATER_LEVEL) {
						block = BlockDefinition::s_nameToIndexMap["Water"];
					}
					else {
						block = BlockDefinition::s_nameToIndexMap["Air"]; 
					}
				}
				else if (z == terrainHeight) 
				{
					block = BlockDefinition::s_nameToIndexMap["Grass"];  
				}
				else if (z >= terrainHeight - s_rng.RollRandomIntInRange(3, 4)) 
				{
					block = BlockDefinition::s_nameToIndexMap["Dirt"];  
				}
				else 
				{
					uint8_t blockType = GetRandomOreType();
					block = Block(blockType);
				}

				SetBlock(coords, block);
			}
		}
	}
}

void Chunk::RebuildMesh()
{
	if (!m_isDirty) return;

	m_vertices.clear();
	m_indices.clear();
	m_vertices.reserve(BLOCKS_PER_CHUNK * 24); 
	m_indices.reserve(BLOCKS_PER_CHUNK * 36);

	for (int z = 0; z < CHUNK_HEIGHT; ++z) 
	{        
		for (int y = 0; y < CHUNK_SIZE_Y; ++y) 
		{ 
			for (int x = 0; x < CHUNK_SIZE_X; ++x) 
			{ 
				IntVec3 coords(x, y, z);
				int index = x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;
				Block block = m_blocks[index];      
				if (BlockDefinition::s_blockDefs[block.GetTypeIndex()].m_isVisible) {
					AddBlockVerts(coords, block); 
				}
			}
		}
	}

 	g_theRenderer->CopyGameVertexBufferToGPU(m_vertices.data(), (int)m_vertices.size(),m_vertexBuffer);
	g_theRenderer->CopyGameIndexBufferToGPU(m_indices.data(), (int)m_indices.size(), m_indexBuffer);

	m_isDirty = false;
}

void Chunk::RebuildDebugMesh()
{
}

void Chunk::Update()
{
}

void Chunk::Render() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(s_blockAtlasTexture);
	g_theRenderer->DrawGameIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer);
}

void Chunk::RenderDebug() const
{
}

Block Chunk::GetBlock(const IntVec3& localCoords) const
{
	if (!IsValidLocalCoords(localCoords)) {
		return Block(0); // return air
	}

	int index = GetBlockIndex(localCoords);
	return m_blocks[index];
}

void Chunk::SetBlock(const IntVec3& localCoords, const Block& block)
{
	if (!IsValidLocalCoords(localCoords)) {
		return;
	}

	int index = GetBlockIndex(localCoords);
	m_blocks[index] = block;
	m_isDirty = true;
}

Block Chunk::GetBlockAtGlobalCoords(const IntVec3& globalCoords) const
{
	return Block();
}

void Chunk::SetBlockAtGlobalCoords(const IntVec3& globalCoords, const Block& block)
{
}

IntVec3 Chunk::GlobalToLocalCoords(const IntVec3& globalCoords) const
{
	int localX = globalCoords.x - (m_chunkCoords.x * CHUNK_SIZE_X);
	int localY = globalCoords.y - (m_chunkCoords.y * CHUNK_SIZE_Y);
	int localZ = globalCoords.z;

	return IntVec3(localX, localY, localZ);
}

IntVec3 Chunk::LocalToGlobalCoords(const IntVec3& localCoords) const
{
	int globalX = localCoords.x + (m_chunkCoords.x * CHUNK_SIZE_X);
	int globalY = localCoords.y + (m_chunkCoords.y * CHUNK_SIZE_Y);
	int globalZ = localCoords.z; 

	return IntVec3(globalX, globalY, globalZ);
}

bool Chunk::IsValidLocalCoords(const IntVec3& localCoords) const
{
	return localCoords.x>=0&&localCoords.x<=15
		&& localCoords.y >= 0 && localCoords.y <= 15
		&& localCoords.z >= 0 && localCoords.x <= 127;
}

Vec3 Chunk::LocalCoordsToWorldPos(const IntVec3& localCoords) const
{
	return Vec3(
		static_cast<float>(m_chunkCoords.x * CHUNK_SIZE_X + localCoords.x),
		static_cast<float>(m_chunkCoords.y * CHUNK_SIZE_Y + localCoords.y),
		static_cast<float>(localCoords.z)
	);
}

int Chunk::GetBlockIndex(const IntVec3& localCoords) const
{
	return localCoords.x + localCoords.y * CHUNK_SIZE_X + localCoords.z * CHUNK_SIZE_X * CHUNK_SIZE_Y;
}

int Chunk::CalculateTerrainHeight(int globalX, int globalY) const
{
	int baseHeight = 50;
	int xContribution = globalX / 3;
	int yContribution = globalY / 5;
	int randomVariation = s_rng.RollRandomIntInRange(0, 1);
	return baseHeight + xContribution + yContribution + randomVariation;
}

uint8_t Chunk::GetRandomOreType() const
{
	float roll = s_rng.RollRandomFloatZeroToOne();

	if (roll < 0.001f) { // 0.1%
		return BlockDefinition::s_nameToIndexMap["Diamond"]; // Diamond
	}
	else if (roll < 0.006f) { // 0.5%
		return BlockDefinition::s_nameToIndexMap["Gold"]; // Gold
	}
	else if (roll < 0.026f) { // 2%
		return BlockDefinition::s_nameToIndexMap["Iron"];  // Iron
	}
	else if (roll < 0.076f) { // 5%
		return BlockDefinition::s_nameToIndexMap["Coal"]; // Coal
	}
	else {
		return BlockDefinition::s_nameToIndexMap["Stone"];  // Stone
	}
}

void Chunk::AddBlockVerts(const IntVec3& localCoords, Block const& block)
{
	const BlockDefinition& def = BlockDefinition::s_blockDefs[block.GetTypeIndex()];

	Vec3 blockBL = LocalCoordsToWorldPos(localCoords);

	// +X (EAST)
	Vec3 bl = blockBL + Vec3(1.0f, 0.0f, 0.0f);
	Vec3 br = blockBL + Vec3(1.0f, 1.0f, 0.0f);
	Vec3 tr = blockBL + Vec3(1.0f, 1.0f, 1.0f);
	Vec3 tl = blockBL + Vec3(1.0f, 0.0f, 1.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(230, 230, 230), def.m_sideUVs);

	// -X (WEST)
	bl = blockBL + Vec3(0.0f, 1.0f, 0.0f);
	br = blockBL + Vec3(0.0f, 0.0f, 0.0f);
	tr = blockBL + Vec3(0.0f, 0.0f, 1.0f);
	tl = blockBL + Vec3(0.0f, 1.0f, 1.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(230, 230, 230), def.m_sideUVs);

	// +Y (NORTH)
	bl = blockBL + Vec3(1.0f, 1.0f, 0.0f);
	br = blockBL + Vec3(0.0f, 1.0f, 0.0f);
	tr = blockBL + Vec3(0.0f, 1.0f, 1.0f);
	tl = blockBL + Vec3(1.0f, 1.0f, 1.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(200, 200, 200), def.m_sideUVs);

	// -Y (SOUTH)
	bl = blockBL + Vec3(0.0f, 0.0f, 0.0f);
	br = blockBL + Vec3(1.0f, 0.0f, 0.0f);
	tr = blockBL + Vec3(1.0f, 0.0f, 1.0f);
	tl = blockBL + Vec3(0.0f, 0.0f, 1.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(200, 200, 200), def.m_sideUVs);

	// +Z (TOP)
	bl = blockBL + Vec3(0.0f, 0.0f, 1.0f);
	br = blockBL + Vec3(1.0f, 0.0f, 1.0f);
	tr = blockBL + Vec3(1.0f, 1.0f, 1.0f);
	tl = blockBL + Vec3(0.0f, 1.0f, 1.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(255, 255, 255), def.m_topUVs);

	// -Z (BOTTOM)
	bl = blockBL + Vec3(0.0f, 1.0f, 0.0f);
	br = blockBL + Vec3(1.0f, 1.0f, 0.0f);
	tr = blockBL + Vec3(1.0f, 0.0f, 0.0f);
	tl = blockBL + Vec3(0.0f, 0.0f, 0.0f);
	AddVertsForQuad3D_WithTBN(m_vertices, m_indices, bl, br, tr, tl,
		Rgba8(255, 255, 255), def.m_bottomUVs);
}



void Chunk::CalculateWorldBounds()
{
}
