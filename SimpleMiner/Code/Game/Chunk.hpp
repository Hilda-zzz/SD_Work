#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Block.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/IntVec3.hpp"

class VertexBuffer;
class IndexBuffer;
class Texture;

constexpr int BLOCKS_PER_CHUNK = 32768;
constexpr int CHUNK_HEIGHT = 128;
constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Y = 16;
constexpr int WATER_LEVEL = CHUNK_HEIGHT / 2;

class Chunk
{
public:
	Chunk(IntVec2 const& chunkCoords);
	~Chunk();

	static void SetBlockAtlasTexture(Texture* texture);

	void Initialize();
	void GenerateBlocks();
	void GenerateEachBlockColumn();
	void RebuildMesh();
	void RebuildDebugMesh();

	void Update();
	void Render() const;
	void RenderDebug() const;

	Block GetBlock(const IntVec3& localCoords) const;
	void SetBlock(const IntVec3& localCoords, const Block& block);
	Block GetBlockAtGlobalCoords(const IntVec3& globalCoords) const;
	void SetBlockAtGlobalCoords(const IntVec3& globalCoords, const Block& block);

	IntVec3 GlobalToLocalCoords(const IntVec3& globalCoords) const;
	IntVec3 LocalToGlobalCoords(const IntVec3& localCoords) const;
	bool IsValidLocalCoords(const IntVec3& localCoords) const;
	Vec3 LocalCoordsToWorldPos(const IntVec3& localCoords) const; //bl pos

	IntVec2 const& GetChunkCoords() const { return m_chunkCoords; }
	AABB3 const& GetWorldBounds() const { return m_worldBounds; }
	bool IsDirty() const { return m_isDirty; }
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }

private:
	int GetBlockIndex(const IntVec3& localCoords) const;
	int CalculateTerrainHeight(int globalX, int globalY) const;
	uint8_t GetRandomOreType() const;
	void AddBlockVerts(const IntVec3& localCoords, Block const& block);
	// 	bool ShouldRenderFace(const IntVec3& localCoords, const IntVec3& neighborOffset) const;
	void CalculateWorldBounds();

private:
	static RandomNumberGenerator s_rng;
	static Texture* s_blockAtlasTexture;

	IntVec2 m_chunkCoords;
	std::vector<Block> m_blocks;        // 32768
	AABB3 m_worldBounds;

	std::vector<Vertex_PCUTBN> m_vertices;
	VertexBuffer* m_vertexBuffer;
	std::vector<unsigned int> m_indices;
	IndexBuffer* m_indexBuffer;

	std::vector<Vertex_PCUTBN> m_debugVertexArray;
	VertexBuffer* m_debugVertexBuffer;

	bool m_isDirty = false;      
};
