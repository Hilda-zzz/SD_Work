#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Block.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/IntVec3.hpp"

class VertexBuffer;
class IndexBuffer;
class Texture;

// === Basic Info ===
constexpr int CHUNK_BITS_X = 4;  //2^4=16
constexpr int CHUNK_BITS_Y = 4;
constexpr int CHUNK_BITS_Z = 7;  //2^7=128
constexpr int CHUNK_SIZE_X = 1 << CHUNK_BITS_X;
constexpr int CHUNK_SIZE_Y = 1 << CHUNK_BITS_Y;
constexpr int CHUNK_SIZE_Z = 1 << CHUNK_BITS_Z;

constexpr int CHUNK_MAX_X = CHUNK_SIZE_X - 1;
constexpr int CHUNK_MAX_Y = CHUNK_SIZE_Y - 1;
constexpr int CHUNK_MAX_Z = CHUNK_SIZE_Z - 1;

constexpr int CHUNK_MASK_X = CHUNK_MAX_X;
constexpr int CHUNK_MASK_Y = CHUNK_MAX_Y << CHUNK_BITS_X;
constexpr int CHUNK_MASK_Z = CHUNK_MAX_Z << (CHUNK_BITS_X + CHUNK_BITS_Y);

constexpr int BLOCKS_PER_CHUNK = CHUNK_SIZE_X*CHUNK_SIZE_Y*CHUNK_SIZE_Z;
constexpr int WATER_LEVEL = CHUNK_SIZE_Z / 2;

constexpr int MAP_SIZE = CHUNK_SIZE_X * CHUNK_SIZE_Y;

// === Chunk Management ===
constexpr int CHUNK_ACTIVATION_RANGE = 320;
constexpr int CHUNK_DEACTIVATION_RANGE = CHUNK_ACTIVATION_RANGE + CHUNK_SIZE_X + CHUNK_SIZE_Y;

constexpr int CHUNK_ACTIVATION_RADIUS_X = 1 + (CHUNK_ACTIVATION_RANGE / CHUNK_SIZE_X);
constexpr int CHUNK_ACTIVATION_RADIUS_Y = 1 + (CHUNK_ACTIVATION_RANGE / CHUNK_SIZE_Y);
constexpr int MAX_ACTIVE_CHUNKS = (2 * CHUNK_ACTIVATION_RADIUS_X) * (2 * CHUNK_ACTIVATION_RADIUS_Y);

constexpr float VISIBLE_FACE_RATIO = 0.2f;

// === PCG Map ===
constexpr unsigned int GAME_SEED = 0u;

constexpr float DEFAULT_OCTAVE_PERSISTANCE = 0.5f;
constexpr float DEFAULT_NOISE_OCTAVE_SCALE = 2.0f;

constexpr float DEFAULT_TERRAIN_HEIGHT = 64.0f;
constexpr float RIVER_DEPTH = 8.0f;
constexpr float TERRAIN_NOISE_SCALE = 200.0f;
constexpr unsigned int TERRAIN_NOISE_OCTAVES = 5u;

constexpr float HUMIDITY_NOISE_SCALE = 800.0f;
constexpr unsigned int HUMIDITY_NOISE_OCTAVES = 4u;

constexpr float TEMPERATURE_RAW_NOISE_SCALE = 0.0075f;
constexpr float TEMPERATURE_NOISE_SCALE = 400.0f;
constexpr unsigned int TEMPERATURE_NOISE_OCTAVES = 4u;

constexpr float HILLINESS_NOISE_SCALE = 250.0f;
constexpr unsigned int HILLINESS_NOISE_OCTAVES = 4u;

constexpr float OCEAN_START_THRESHOLD = 0.0f;
constexpr float OCEAN_END_THRESHOLD = 0.5f;
constexpr float OCEAN_DEPTH = 30.0f;

constexpr float OCEANESS_NOISE_SCALE = 600.0f;
constexpr unsigned int OCEANESS_NOISE_OCTAVES = 3u;

constexpr int MIN_DIRT_OFFSET_Z = 3;
constexpr int MAX_DIRT_OFFSET_Z = 4;
constexpr float MIN_SAND_HUMIDITY = 0.4f;
constexpr float MAX_SAND_HUMIDITY = 0.7f;
constexpr int SEA_LEVEL_Z = CHUNK_SIZE_Z / 2;

constexpr float ICE_TEMPERATURE_MAX = 0.37f;
constexpr float ICE_TEMPERATURE_MIN = 0.0f;
constexpr float ICE_DEPTH_MIN = 0.0f;
constexpr float ICE_DEPTH_MAX = 8.0f;

constexpr float MIN_SAND_DEPTH_HUMIDITY = 0.4f;
constexpr float MAX_SAND_DEPTH_HUMIDITY = 0.0f;
constexpr float SAND_DEPTH_MIN = 0.0f;
constexpr float SAND_DEPTH_MAX = 6.0f;

constexpr float COAL_CHANCE = 0.05f;
constexpr float IRON_CHANCE = 0.02f;
constexpr float GOLD_CHANCE = 0.005f;
constexpr float DIAMOND_CHANCE = 0.0001f;
constexpr int OBSIDIAN_Z = 1;
constexpr int LAVA_Z = 0;

struct ChunkFileHeader
{
	char m_fourCC[4];        // 4-byte character code "GCHK"
	uint8_t m_version;       // File format version (1)
	uint8_t m_chunkBitsX;    // CHUNK_BITS_X (4)
	uint8_t m_chunkBitsY;    // CHUNK_BITS_Y (4) 
	uint8_t m_chunkBitsZ;    // CHUNK_BITS_Z (7)
};

struct ChunkFileRun
{
	uint8_t blockType;     // Block type [0-255], where 0 = AIR
	uint8_t runLength;     // Number of consecutive blocks [1-255]
};

class Chunk
{
public:
	Chunk(IntVec2 const& chunkCoords);
	~Chunk();

	static void SetBlockAtlasTexture(Texture* texture);

	void Initialize();
	void GenerateBlocks();
	void RebuildMesh();
	void RebuildMeshWithCulling();
	void RebuildDebugMesh();

	void Update();
	void Render() const;
	void RenderDebug() const;

	//=== GET AND SET ===
	Block GetBlock(const IntVec3& localCoords) const;
	Block GetBlock(int index) const;
	void SetBlock(const IntVec3& localCoords, const Block& block);
	void SetBlock(int index, const Block& block);
// 	Block GetBlockAtGlobalCoords(const IntVec3& globalCoords) const;
// 	void SetBlockAtGlobalCoords(const IntVec3& globalCoords, const Block& block);

	IntVec2 const& GetChunkCoords() const { return m_chunkCoords; }
	AABB3 const& GetWorldBounds() const { return m_worldBounds; }
	bool IsDirty() const { return m_isDirty; }
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }

	int GetVertsCount() { return m_vertsCount; }
	int GetIndicesCount() { return m_indicesCount; }

public:
	// === UTILITY ===
	int LocalCoordsToIndex(const IntVec3& localCoords) const;
	int LocalCoordsToIndex(int x, int y, int z) const;

	int IndexToLocalX(int index);
	int IndexToLocalY(int index);
	int IndexToLocalZ(int index);
	IntVec3 IndexToLocalCoords(int index) const;

	int GlobalCoordsToIndex(const IntVec3& globalCoords);
	int GlobalCoordsToIndex(int x, int y, int z);

	static IntVec2 GetChunkCoords(Vec3 const& position);
	IntVec2 GetChunkCoords(const IntVec3& globalCoords);
	IntVec2 GetChunkCenter();
	static IntVec2 GetChunkCenter(const IntVec2& chunkCoords);
	IntVec3 GlobalCoordsToLocalCoords(const IntVec3& globalCoords);

	IntVec3 GetGlobalCoords(const IntVec2& chunkCoords, int blockIndex);
	IntVec3 GetGlobalCoords(const IntVec2& chunkCoords, const IntVec3& localCoords);
	IntVec2 GetGlobalCoords(int localX, int localY);
	IntVec3 GetGlobalCoords(const Vec3& position);
	IntVec3 LocalToGlobalCoords(const IntVec3& localCoords) const;
	bool IsValidLocalCoords(const IntVec3& localCoords) const;
	bool IsValidIndex(int index) const;
	Vec3 LocalCoordsToWorldPos(const IntVec3& localCoords) const; //bl pos
	
	
	int CalculateTerrainHeight(int globalX, int globalY) const;
	uint8_t GetRandomOreType() const;
	void AddBlockVerts(const IntVec3& localCoords, Block const& block);
	void AddBlockVertsWithCulling(int blockIndex, Block const& block);
	void CalculateWorldBounds();

	int SaveChunkToFile(std::string const& saveFolder);
	bool LoadChunkFromFile(std::string const& filename);

public:
	static RandomNumberGenerator s_rng;
	static Texture* s_blockAtlasTexture;

	IntVec2 m_chunkCoords;
	std::vector<Block> m_blocks;        // 32768
	AABB3 m_worldBounds;

	std::vector<Vertex_PCUTBN> m_vertices;
	VertexBuffer* m_vertexBuffer;
	std::vector<unsigned int> m_indices;
	IndexBuffer* m_indexBuffer;

	std::vector<Vertex_PCU> m_debugVertexArray;

	bool m_isDirty = true;    
	bool m_needsSaving = false;

	int m_vertsCount = 0;
	int m_indicesCount = 0;
};
