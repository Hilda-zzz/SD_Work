#pragma once
#include <vector>
#include <unordered_map>
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <deque>
#include <set>
#include "BlockIterator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

class Chunk;
class Player;
class GenerateChunkJob;
class LoadChunkJob;
class SaveChunkJob;
class ConstantBuffer;
class Shader;

struct WorldConstants
{
	Vec3 CameraPosition;
	float Padding1;

	float  IndoorLightColor[4];
	float  OutdoorLightColor[4];
	float  SkyColor[4];

	float FogNearDistance;
	float FogFarDistance;
	float Padding2[2];
};


class World
{
public:
	World(Player* player);
	~World();
	void Shutdown();

	void Update(float deltaTime);
	void Render() const;
	void RenderChunkJobInfo() const;

	void ToggleDebugDraw();
	void ToggleDebugChunkJobStatInfo();

	void DigBlockByPlayerPos(Vec3 const& playerPos);
	void PlaceBlockByPlayerPos(std::string const& typeName, Vec3 const& playerPos);

	void HookUpChunkNeighbors(Chunk* chunk);
	void UnhookChunkNeighbors(Chunk* chunk);

private:
	//=========================
	void RequestChunkActivation();
	void ActivatingChunkAddToQueue(IntVec2 const& chunkCoords);

	void RequestChunkDeactivations();
	void RequestChunkDeactivation(Chunk* chunk);

	void ProcessCompletedJobs();

	void SubmitQueuedJobs();
	void SubmitGenerateJobs();
	void SubmitLoadJobs();
	void SubmitSaveJobs();

	void SortJobQueuesByDistance();
	void SortGenerateJobsByDistance();
	void SortLoadJobsByDistance();

	void CheckDirtyChunks();
	void SortMeshRebuildQueueByDistance();
	void RebuildChunkMeshes();
	//=========================

	void CancelPendingJobs();
	//=========================

	void GetChunkStateCounts(int& constructing, int& generating, int& loading,
		int& active, int& saving) const;

	void GetJobCounts(int& generateJobsQueued, int& generateJobsInFlight,
		int& loadJobsQueued, int& loadJobsInFlight,
		int& saveJobsQueued, int& saveJobsInFlight,
		int& meshRebuildsQueued) const;
	//=========================
	void InitializeChunkLighting(Chunk* chunk);
	// Lighting initialization helpers (Step 3-6)
	void MarkBoundaryBlocksAsDirty(Chunk* chunk);      // Step 3: Mark boundary blocks touching neighbors
	void MarkSkyBlocks(Chunk* chunk);                  // Step 4: Flag blocks as SKY (no opaque above)
	void SetSkyLightAndMarkNeighbors(Chunk* chunk);    // Step 5: Set sky outdoor light to 15, mark neighbors
	void MarkLightEmittingBlocksAsDirty(Chunk* chunk); // Step 6: Mark glowstone etc. as dirty

	void ProcessDirtyLighting();										// Processes all dirty light blocks until none remain
	void ProcessNextDirtyLightBlock();									// Pops the front block in the queue, recomputes its lighting
	void MarkLightingDirty(BlockIterator& blockIter);				// Adds a block iterator to the back of the dirty light queue
	void UndirtyAllBlocksInChunk(Chunk* chunk);							// Scans the dirty queue, removes all blocks from that chunk
	void MarkLightingDirtyIfNotOpaque(BlockIterator& blockIter);	// Called on neighbors when light influences change
	void MarkNeighborsAsDirtyIfNotOpaque(BlockIterator& centerIter);

	// Compute theoretically-correct light values
	uint8_t ComputeCorrectIndoorLight(Block* block, BlockIterator& blockIter);
	uint8_t ComputeCorrectOutdoorLight(Block* block, BlockIterator& blockIter);
	//=========================

	void DigBlock(Chunk* chunk, int blockIndex);
	void PlaceBlock(Chunk* chunk, int blockIndex, std::string const& typeName);
	void PropagateSkyFlagDownward(BlockIterator startIter);
	void ClearSkyFlagsDownward(BlockIterator startIter);

	// ========== 世界渲染相关（新增）==========
	void UpdateWorldRenderConstants();
	void UpdateDayNightCycle();
	void SetWorldConstantsToGPU() const;

private:
	std::vector<Chunk*> m_chunkUpdateList;
	std::unordered_map< IntVec2, Chunk*,IntVec2Hash > m_activeChunks;

	std::deque<GenerateChunkJob*> m_generateJobsQueued;  
	int m_maxConcurrentGenerateJobs = 100;

	std::deque<LoadChunkJob*> m_loadJobsQueued;   
	int m_maxConcurrentLoadJobs = 4;

	std::deque<SaveChunkJob*> m_saveJobsQueued;
	int m_maxConcurrentSaveJobs = 2;

	std::deque<Chunk*> m_chunksQueuedForMeshRebuild;
	int m_maxMeshRebuildsPerFrame = 2;

	// track the current chunks
	std::set<IntVec2> m_chunksGenerating;
	std::set<IntVec2> m_chunksLoading;
	std::set<Chunk*> m_chunksSaving;

	Player* m_player = nullptr;
	bool m_isDebugDraw = false;
	bool m_showChunkJobStats = false;

	std::deque<BlockIterator> m_dirtyLightingQueue;

	// ==========================================
	// ========== 世界渲染状态（新增）==========
	Shader* m_worldShader = nullptr;
	ConstantBuffer* m_worldConstantBuffer = nullptr;  // 自己管理的 CBO

	float m_timeOfDay = 0.5f;           // 0.0-1.0 (0=午夜, 0.5=正午)
	float m_fogNearDistance = 128.0f;
	float m_fogFarDistance = 256.0f;

	Rgba8 m_indoorLightColor = Rgba8(255, 230, 204);   // 暖黄色
	Rgba8 m_outdoorLightColor = Rgba8(255, 255, 255);  // 白色
	Rgba8 m_skyColor = Rgba8(135, 206, 235, 180);      // 蓝天
};