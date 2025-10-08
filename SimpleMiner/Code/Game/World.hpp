#pragma once
#include <vector>
#include <unordered_map>
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <deque>
#include <set>
class Chunk;
class Player;
struct Vec3;

class GenerateChunkJob;
class LoadChunkJob;
class SaveChunkJob;

struct IntVec2Hash 
{
	size_t operator()(const IntVec2& vec) const {
		return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
	}
};

class World
{
public:
	World(Player* player);
	~World();

	void Update(float deltaTime);
	void Render() const;
	void RenderChunkJobInfo() const;

	void ToggleDebugDraw();
	void ToggleDebugChunkJobStatInfo();

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

	//void InitializeChunks();
	//void ActivateChunk(IntVec2 const& chunkCoords);
	//void DeactivateChunk(Chunk* farChunk);

	//Chunk* FindNearestDirtyChunk(Vec3 const& playerPos);
	//bool FindNearestMissingChunk(Vec3 const& playerPos,IntVec2& chunkCoords);
	//Chunk* FindFarestOutrangeChunk(Vec3 const& playerPos);

	//bool IsChunkInActivationRange(const IntVec2& chunkCoords, const Vec3& playerPos) const;

	//=========================
	void GetChunkStateCounts(int& constructing, int& generating, int& loading,
		int& active, int& saving) const;

	void GetJobCounts(int& generateJobsQueued, int& generateJobsInFlight,
		int& loadJobsQueued, int& loadJobsInFlight,
		int& saveJobsQueued, int& saveJobsInFlight,
		int& meshRebuildsQueued) const;

public:
	void DigBlock(Vec3 const& playerPos);
	void PlaceBlock(std::string const& typeName, Vec3 const& playerPos);

	void HookUpChunkNeighbors(Chunk* chunk);
	void UnhookChunkNeighbors(Chunk* chunk);

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
	int m_maxMeshRebuildsPerFrame = 4;

	// track the current chunks
	std::set<IntVec2> m_chunksGenerating;
	std::set<IntVec2> m_chunksLoading;
	std::set<Chunk*> m_chunksSaving;

	Player* m_player = nullptr;
	bool m_isDebugDraw = false;
	bool m_showChunkJobStats = false;
};