#include "World.hpp"
#include "Chunk.hpp"
#include "Player.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <algorithm>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "BlockDefinition.hpp"
#include <Engine/Core/FileUtils.hpp>
#include "Engine/JobSystem/Job.hpp"
#include "Game/GenerateChunkJob.hpp"
#include "LoadChunkJob.hpp"
#include "Engine/JobSystem/JobSystem.hpp"
#include "SaveChunkJob.hpp"
#include "TerrainGenerator.hpp"
#include "TerrainConfig.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

extern Game* g_theGame;
extern JobSystem* g_theJobSystem;

World::World(Player* player):m_player(player)
{
	//InitializeChunks();
	m_player->SetCurWorld(this);

	m_worldShader = g_theRenderer->CreateShaderFromFile("Data/Shaders/WorldShader");
	m_worldConstantBuffer = g_theRenderer->CreateConstantBuffer(sizeof(WorldConstants));

	//*****************************************************************************************************
	//for (int i = -1; i < 1; i++)
	//{
	//	for (int j = -1; j < 1; j++)
	//	{
	//		Chunk* newChunk = new Chunk(IntVec2(i, j));
	//		m_chunkUpdateList.push_back(newChunk);
	//		TerrainGenerator::GenerateBlocksForChunk_New(newChunk);
	//		newChunk->RebuildMeshWithCulling();
	//		newChunk->m_needsSaving = false;
	//	}
	//}
	//Chunk * newChunk = new Chunk(IntVec2(0, 0));
	//m_chunkUpdateList.push_back(newChunk);
	//TerrainGenerator::GenerateBlocksForChunk_New(newChunk);
	//newChunk->RebuildMeshWithCulling();
	//newChunk->m_needsSaving = false;
}

World::~World()
{
	Shutdown();

	m_player = nullptr;

	if (m_worldConstantBuffer)
	{
		delete m_worldConstantBuffer;
		m_worldConstantBuffer = nullptr;
	}

	for (Chunk* chunk : m_chunkUpdateList)
	{
		delete chunk;
	}

	m_chunkUpdateList.clear();
	m_activeChunks.clear();

	for (GenerateChunkJob* job : m_generateJobsQueued) delete job;
	for (LoadChunkJob* job : m_loadJobsQueued) delete job;
	for (SaveChunkJob* job : m_saveJobsQueued) delete job;

	m_generateJobsQueued.clear();
	m_loadJobsQueued.clear();
	m_saveJobsQueued.clear();
}

void World::Shutdown()
{
	// ========== Step 1: 取消所有未提交的jobs ==========
	CancelPendingJobs();

	// ========== Step 2: 将所有active chunks加入保存队列 ==========
	for (auto& pair : m_activeChunks)
	{
		Chunk* chunk = pair.second;
		if (!chunk) continue;

		// 跳过已经在保存的chunk
		if (m_chunksSaving.find(chunk) != m_chunksSaving.end()) {
			continue;
		}

		// 只保存需要保存的chunk
		if (chunk->m_needsSaving)
		{
			std::string filename = Stringf("Chunk(%d,%d).chunk",
				chunk->m_chunkCoords.x, chunk->m_chunkCoords.y);

			SaveChunkJob* job = new SaveChunkJob(this, chunk, filename);
			m_saveJobsQueued.push_back(job);
		}
	}

	// ========== Step 3: 提交所有保存jobs（忽略并发限制） ==========
	while (!m_saveJobsQueued.empty())
	{
		SaveChunkJob* job = m_saveJobsQueued.front();
		m_saveJobsQueued.pop_front();

		g_theJobSystem->QueueJob(job);
		m_chunksSaving.insert(job->GetChunk());
	}

	// ========== Step 4: 等待所有保存jobs完成 ==========
	int frameCount = 0;
	const int MAX_WAIT_FRAMES = 300; // 5秒超时 (60fps)

	while (!m_chunksSaving.empty())
	{
		// 处理完成的jobs
		std::vector<Job*> completedJobs;
		g_theJobSystem->RetrieveCompletedJobs(completedJobs);

		for (Job* job : completedJobs)
		{
			if (SaveChunkJob* saveJob = dynamic_cast<SaveChunkJob*>(job))
			{
				Chunk* chunk = saveJob->GetChunk();
				m_chunksSaving.erase(chunk);
			}

			delete job;
		}

		frameCount++;

		// 短暂休眠以避免忙等待
		if (!m_chunksSaving.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
		}
	}

	//// ========== Step 5: 检查是否超时 ==========
	//if (!m_chunksSaving.empty())
	//{
	//	printf("WARNING: %zu chunks failed to save within timeout!\n",
	//		m_chunksSaving.size());

	//	// 超时后同步保存剩余的chunks
	//	for (Chunk* chunk : m_chunksSaving)
	//	{
	//		if (chunk && chunk->m_needsSaving)
	//		{
	//			printf("Force-saving chunk (%d,%d)...\n",
	//				chunk->m_chunkCoords.x, chunk->m_chunkCoords.y);

	//			std::string filename = Stringf("Saves/Chunk(%d,%d).chunk",
	//				chunk->m_chunkCoords.x, chunk->m_chunkCoords.y);
	//			ChunkFileIO::SaveChunk("Saves/", filename, chunk);
	//		}
	//	}
	//}

	//printf("=== World Shutdown Complete: All chunks saved ===\n");
}

void World::Update(float deltaTime)
{
	m_player->Update(deltaTime);

	//--------------------------
	UpdateWorldRenderConstants();

	//*****************************************************************************************************
	// ========== debug draw basis ==========
	Vec3 basisPos = m_player->m_position + m_player->m_orientation.GetForward_IFwd() * 50.f;
	Mat44 basisTransform = Mat44::MakeTranslation3D(basisPos);
	DebugAddWorldBasis(basisTransform, 0.f, DebugRenderMode::ALWAYS, 1.5f);
	DebugAddWorldBasis(Mat44(), 0.f, DebugRenderMode::USE_DEPTH, 1.f);

	// ========== dealing with completed jobs ==========
	ProcessCompletedJobs();

	// ========== Sort the job queue by distance ==========
	SortJobQueuesByDistance();

	// ========== Submit jobs from queues ==========
	SubmitQueuedJobs();

	// ========== Rebuild meshes ==========
	CheckDirtyChunks();
	SortMeshRebuildQueueByDistance();
	RebuildChunkMeshes();

	// ========== Activate new chunk ==========
	RequestChunkActivation();
	RequestChunkDeactivations();

	// ========== Update active chunks ==========
	for (Chunk* chunk : m_chunkUpdateList)
	{
		chunk->Update();
		if (chunk->m_chunkCoords.x == 0 && chunk->m_chunkCoords.y == 0)
		{
			int a = 0;
		}
	}

	ProcessDirtyLighting();
	//*****************************************************************************************************

	//---------------------------------------------------------------------------------
	// rebuild nearest dirty mesh chunk
	//Chunk* nearestDirtyChunk = FindNearestDirtyChunk(m_player->m_position);
	//if (nearestDirtyChunk) {
	//	nearestDirtyChunk->RebuildMeshWithCulling();
	//}

	//// or activate a nearest missing chunk
	//else if (m_chunkUpdateList.size() < MAX_ACTIVE_CHUNKS) {
	//	IntVec2 chunkCoords = IntVec2(-999999, -999999);
	//	if (FindNearestMissingChunk(m_player->m_position, chunkCoords))
	//	{
	//		ActivateChunk(chunkCoords);
	//	}
	//}

	//// or deactivate farthest active chunk if its center is outside the deactivation range
	//else
	//{
	//	Chunk* farestChunk = FindFarestOutrangeChunk(m_player->m_position);
	//	if (farestChunk)
	//	{
	//		DeactivateChunk(farestChunk);
	//	}
	//}
}

void World::Render() const
{
	//---------------------------------
	SetWorldConstantsToGPU();

	g_theRenderer->BindShader(m_worldShader);
	for (Chunk* chunk : m_chunkUpdateList)
	{
		chunk->Render();
	}

	// === Render Noise Debug Visualization ===
	TerrainConfig& config = TerrainConfig::GetInstance();
	if (config.m_debug.m_showNoiseDebug &&
		config.m_debug.m_activeDebugMode != NoiseDebugMode::NONE)
	{
		for (Chunk* chunk : m_chunkUpdateList)
		{
			chunk->RenderNoiseDebug();
		}
	}

	if (m_isDebugDraw)
	{
		int totalChunkCount = (int)m_chunkUpdateList.size();
		int totalVertsCount = 0;
		int totalIndicesCount = 0;
		for (Chunk* chunk : m_chunkUpdateList)
		{
			chunk->RenderDebug();
			totalIndicesCount += chunk->GetIndicesCount();
			totalVertsCount += chunk->GetVertsCount();
		}

		// Get position information
		IntVec2 chunkCoords = Chunk::GetChunkCoords(m_player->m_position);
		auto it = m_activeChunks.find(chunkCoords);
		Chunk* curChunk = nullptr;
		if (it != m_activeChunks.end())
		{
			curChunk = it->second;
		}

		IntVec3 globalCoords(0, 0, 0);
		IntVec3 localCoords(0, 0, 0);
		if (curChunk)
		{
			globalCoords = curChunk->GetGlobalCoords(m_player->m_position);
			localCoords = curChunk->GlobalCoordsToLocalCoords(globalCoords);
		}

		// Create comprehensive debug info buffer
		char debugBuffer[1024];
		snprintf(debugBuffer, sizeof(debugBuffer),
			"Chunk (In update list): %d  Vertices: %d  Indices: %d\n"
			"Player Pos: (%.2f, %.2f, %.2f)\n"
			"Chunk Coords: (%d, %d)\n"
			"Global Block: (%d, %d, %d)\n"
			"Local Block: (%d, %d, %d)",
			totalChunkCount, totalVertsCount, totalIndicesCount,
			m_player->m_position.x, m_player->m_position.y, m_player->m_position.z,
			chunkCoords.x, chunkCoords.y,
			globalCoords.x, globalCoords.y, globalCoords.z,
			localCoords.x, localCoords.y, localCoords.z);

		float textWidth = 800.f;
		float textHeight = 400.f;  // Increased height for multiple lines
		float margin = 10.f;
		float verticalOffset = 60.f;  // Move down from the top debug line
		Vec2 topLeft = Vec2(margin, g_theGame->GetScreenSize().y - margin - textHeight - verticalOffset);
		Vec2 bottomRight = Vec2(margin + textWidth, g_theGame->GetScreenSize().y - margin - verticalOffset);
		DebugAddScreenText(std::string(debugBuffer),
			AABB2(topLeft, bottomRight),
			16.f,  // Slightly smaller font to fit more text
			Vec2(0.f, 1.f),  // Left-aligned, top-aligned
			0.f,
			Rgba8::WHITE,
			Rgba8::WHITE);
	}

	RenderChunkJobInfo();
}

void World::RenderChunkJobInfo() const
{
	if (!m_showChunkJobStats)
		return;

	// 获取统计数据
	int constructing, generating, loading, active, saving;
	GetChunkStateCounts(constructing, generating, loading, active, saving);

	int genQueued, genInFlight, loadQueued, loadInFlight;
	int saveQueued, saveInFlight, meshQueued;
	GetJobCounts(genQueued, genInFlight, loadQueued, loadInFlight,
		saveQueued, saveInFlight, meshQueued);

	// 获取JobSystem的统计信息
	int pendingJobs = 0;
	int executingJobs = 0;
	int completedJobs = 0;
	if (g_theJobSystem)
	{
		pendingJobs = genQueued + loadQueued + saveQueued;
		executingJobs = genInFlight + loadInFlight + saveInFlight;
		completedJobs = g_theJobSystem->GetNumCompletedJobs();
	}

	// 构建显示文本
	char buffer[1024];
	snprintf(buffer, sizeof(buffer),
		"=== World ===\n"
		"chunksPendingSave:        %d\n"
		"chunksSaving:             %d\n"
		"chunks:                   %d\n"
		"chunksPendingLoad:        %d\n"
		"chunksLoading:            %d\n"
		"chunksPendingGeneration:  %d\n"
		"chunksGenerating:         %d\n"
		"=== JobSystem ===\n"
		"pendingJobs:              %d\n"
		"executingJobs:            %d\n"
		"completedJobs:            %d",
		saveQueued,           
		saveInFlight,        
		(int)m_activeChunks.size(),
		loadQueued,          
		loadInFlight,         
		genQueued,            
		genInFlight,         
		pendingJobs,          
		executingJobs,       
		completedJobs         
	);

	Vec2 screenSize = g_theGame->GetScreenSize();
	float margin = 10.f;
	float textWidth = 500.f;
	float textHeight = 300.f;

	Vec2 bottomLeft = Vec2(margin, margin);
	Vec2 topLeft = Vec2(margin + textWidth, margin + textHeight);

	AABB2 textBox(bottomLeft, topLeft);

	DebugAddScreenText(
		std::string(buffer),
		textBox,
		16.f,                 
		Vec2(0.f, 0.f),       
		0.f,                    
		Rgba8::YELLOW,          
		Rgba8::BLACK            
	);
}

void World::ToggleDebugDraw()
{
	m_isDebugDraw = !m_isDebugDraw;
}

void World::ToggleDebugChunkJobStatInfo()
{
	m_showChunkJobStats = !m_showChunkJobStats;
}

void World::RequestChunkActivation()
{
	if (!m_player) return;

	// Get player's current chunk coordinates
	IntVec2 playerChunkCoords = Chunk::GetChunkCoords(m_player->m_position);
	Vec2 playerPosXY = Vec2(m_player->m_position.x, m_player->m_position.y);

	// Calculate how many chunks we can still activate
	int totalActiveChunks = (int)m_activeChunks.size();
	int totalGenerating = (int)m_chunksGenerating.size();
	int totalLoading = (int)m_chunksLoading.size();
	int queuedGenerate = (int)m_generateJobsQueued.size();  // ← 加上排队的！
	int queuedLoad = (int)m_loadJobsQueued.size();          // ← 加上排队的！

	// real total count
	int totalInSystem = totalActiveChunks
		+ totalGenerating
		+ totalLoading
		+ queuedGenerate  
		+ queuedLoad;     

	int slotsAvailable = MAX_ACTIVE_CHUNKS - totalInSystem;

	if (slotsAvailable <= 0) {
		return; // Already at max capacity
	}

	// Create a list of candidate chunks with their distances
	struct ChunkCandidate {
		IntVec2 coords;
		float distanceSq;
	};
	std::vector<ChunkCandidate> candidates;
	candidates.reserve((2 * CHUNK_ACTIVATION_RADIUS_X + 1) * (2 * CHUNK_ACTIVATION_RADIUS_Y + 1));

	// Collect all chunks within activation range
	for (int dx = -CHUNK_ACTIVATION_RADIUS_X; dx <= CHUNK_ACTIVATION_RADIUS_X; ++dx) 
	{
		for (int dy = -CHUNK_ACTIVATION_RADIUS_Y; dy <= CHUNK_ACTIVATION_RADIUS_Y; ++dy) {
			IntVec2 candidateCoords = playerChunkCoords + IntVec2(dx, dy);

			// Skip if chunk is already active or being processed
			if (m_activeChunks.find(candidateCoords) != m_activeChunks.end()) {
				continue;
			}
			if (m_chunksLoading.find(candidateCoords) != m_chunksLoading.end()) {
				continue;
			}
			if (m_chunksGenerating.find(candidateCoords) != m_chunksGenerating.end()) {
				continue;
			}

			// Calculate distance
			IntVec2 chunkCenter = Chunk::GetChunkCenter(candidateCoords);
			float distSq = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
			if (distSq > static_cast<float>(CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE))
				continue;

			candidates.push_back({ candidateCoords, distSq });
		}
	}

	// Sort candidates by distance (nearest first)
	std::sort(candidates.begin(), candidates.end(),
		[](const ChunkCandidate& a, const ChunkCandidate& b) {
			return a.distanceSq < b.distanceSq;
		});

	// Activate chunks from nearest to farthest until we hit the limit
	int activated = 0;
	for (const ChunkCandidate& candidate : candidates) {
		if (activated >= slotsAvailable) {
			break; // Reached activation limit
		}

		ActivatingChunkAddToQueue(candidate.coords);
		activated++;
	}
}

void World::ActivatingChunkAddToQueue(IntVec2 const& chunkCoords)
{
	for (auto* job : m_generateJobsQueued)
	{
		if (job->GetChunkCoords() == chunkCoords)
		{
			return;  
		}
	}

	for (auto* job : m_loadJobsQueued)
	{
		if (job->GetChunkCoords() == chunkCoords)
		{
			return;  
		}
	}

	// ====== if have file, else generate ======
	std::string filename = "Chunk(" + std::to_string(chunkCoords.x) + "," + std::to_string(chunkCoords.y) + ").chunk";
	// Check if chunk file exists
	if (FileExists("Saves/"+filename))
	{
		LoadChunkJob* job = new LoadChunkJob(this, chunkCoords, filename);
		m_loadJobsQueued.push_back(job);
	}
	else
	{
		GenerateChunkJob* job = new GenerateChunkJob(this, chunkCoords);
		m_generateJobsQueued.push_back(job);
	}
}

void World::RequestChunkDeactivations()
{
	if (!m_player) return;

	Vec2 playerPosXY = Vec2(m_player->m_position.x, m_player->m_position.y);
	static float deactivationRangeSq = static_cast<float>(CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE);

	// Create a list of chunks to deactivate (can't modify m_activeChunks while iterating)
	std::vector<Chunk*> chunksToDeactivate;

	// Find all chunks that are outside the deactivation range
	for (auto& pair : m_activeChunks) {
		Chunk* chunk = pair.second;
		if (!chunk) continue;

		// Skip chunks that are currently saving
		if (m_chunksSaving.find(chunk) != m_chunksSaving.end()) {
			continue;
		}

		// Calculate distance from player to chunk center
		IntVec2 chunkCenter = chunk->GetChunkCenter();
		float distanceSq = GetDistanceSquared2D(
			playerPosXY,
			Vec2((float)chunkCenter.x, (float)chunkCenter.y)
		);

		// If chunk is outside deactivation range, mark for deactivation
		if (distanceSq > deactivationRangeSq) {
			chunksToDeactivate.push_back(chunk);
		}

		//if (IsChunkInActivationRange(chunk->m_chunkCoords, m_player->m_position))
		//{

		//}
	}

	// Request deactivation for all chunks outside range
	for (Chunk* chunk : chunksToDeactivate) {
		RequestChunkDeactivation(chunk);
	}
}

void World::RequestChunkDeactivation(Chunk* chunk)
{
	if (!chunk)
		return;

	UndirtyAllBlocksInChunk(chunk);

	IntVec2 coords = chunk->GetChunkCoords();

	UnhookChunkNeighbors(chunk);

	m_activeChunks.erase(coords);

	auto it = std::find(m_chunkUpdateList.begin(), m_chunkUpdateList.end(), chunk);
	if (it != m_chunkUpdateList.end())
	{
		m_chunkUpdateList.erase(it);
	}

	// 2. if in mesh rebuild queue
	auto meshIt = std::find(m_chunksQueuedForMeshRebuild.begin(),
		m_chunksQueuedForMeshRebuild.end(),
		chunk);
	if (meshIt != m_chunksQueuedForMeshRebuild.end())
	{
		m_chunksQueuedForMeshRebuild.erase(meshIt);
	}

	// 3. Save job
	if (chunk->m_needsSaving)
	{
		std::string filename = Stringf("Chunk(%d,%d).chunk",
			coords.x, coords.y);

		SaveChunkJob* job = new SaveChunkJob(this, chunk, filename);
		m_saveJobsQueued.push_back(job); // deleted by main threading when ProcessCompletedJobs
	}
	else
	{
		delete chunk;
	}
}

void World::ProcessCompletedJobs()
{
	std::vector<Job*> completedJobs;
	g_theJobSystem->RetrieveCompletedJobs(completedJobs);

	for (Job* job : completedJobs)
	{
		// dealing with finished generating chunk
		if (GenerateChunkJob* genJob = dynamic_cast<GenerateChunkJob*>(job))
		{
			Chunk* chunk = genJob->GetChunk();
			IntVec2 coords = chunk->GetChunkCoords();

			// add the chunk to active chunk lists
			m_activeChunks[coords] = chunk;
			m_chunkUpdateList.push_back(chunk);

			// update state
			chunk->m_state = ChunkState::ACTIVE;
			m_chunksGenerating.erase(coords);

			// add it to the queue for generating mesh
			m_chunksQueuedForMeshRebuild.push_back(chunk);

			HookUpChunkNeighbors(chunk);
			InitializeChunkLighting(chunk);
		}
		// dealing with finished loading chunk
		else if (LoadChunkJob* loadJob = dynamic_cast<LoadChunkJob*>(job))
		{
			Chunk* chunk = loadJob->GetChunk();
			IntVec2 coords = chunk->GetChunkCoords();

			m_activeChunks[coords] = chunk;
			m_chunkUpdateList.push_back(chunk);

			chunk->m_state = ChunkState::ACTIVE;
			m_chunksLoading.erase(coords);

			m_chunksQueuedForMeshRebuild.push_back(chunk);

			HookUpChunkNeighbors(chunk);
			InitializeChunkLighting(chunk);
		}
		// dealing with finished saving chunk
		else if (SaveChunkJob* saveJob = dynamic_cast<SaveChunkJob*>(job))
		{
			Chunk* chunk = saveJob->GetChunk();
			m_chunksSaving.erase(chunk);
			delete chunk;
		}

		delete job;
	}
}

void World::SubmitQueuedJobs()
{
	SubmitGenerateJobs();
	SubmitLoadJobs();
	SubmitSaveJobs();
}

void World::SubmitGenerateJobs()
{
	while (!m_generateJobsQueued.empty() &&
		m_chunksGenerating.size() < (unsigned int)m_maxConcurrentGenerateJobs)
	{
		GenerateChunkJob* job = m_generateJobsQueued.front();
		m_generateJobsQueued.pop_front();
		
		// discard far chunk
		Vec2 playerPosXY = Vec2(m_player->m_position.x, m_player->m_position.y);
		IntVec2 chunkCoords = job->GetChunkCoords();
		IntVec2 chunkCenter = Chunk::GetChunkCenter(chunkCoords);
		float distance = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
		if (distance > CHUNK_DEACTIVATION_RANGE* CHUNK_DEACTIVATION_RANGE)
		{
			delete job;
			continue;
		}

		g_theJobSystem->QueueJob(job);
		m_chunksGenerating.insert(job->GetChunkCoords());
	}
}

void World::SubmitLoadJobs()
{
	while (!m_loadJobsQueued.empty() &&
		m_chunksLoading.size() <(unsigned int) m_maxConcurrentLoadJobs)
	{
		LoadChunkJob* job = m_loadJobsQueued.front();
		m_loadJobsQueued.pop_front();

		IntVec2 chunkCoords = job->GetChunkCoords();

		// discard far chunk
		Vec2 playerPosXY = Vec2(m_player->m_position.x, m_player->m_position.y);
		IntVec2 chunkCenter = Chunk::GetChunkCenter(chunkCoords);
		float distance = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
		if (distance > CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE)
		{
			delete job;
			continue;
		}

		g_theJobSystem->QueueJob(job);
		m_chunksLoading.insert(chunkCoords);
	}
}

void World::SubmitSaveJobs()
{
	while (!m_saveJobsQueued.empty() &&
		m_chunksSaving.size() < (unsigned int)m_maxConcurrentSaveJobs)
	{
		SaveChunkJob* job = m_saveJobsQueued.front();
		m_saveJobsQueued.pop_front();

		g_theJobSystem->QueueJob(job);
		m_chunksSaving.insert(job->GetChunk());
	}
}

void World::SortJobQueuesByDistance()
{
	void SortGenerateJobsByDistance();
	void SortLoadJobsByDistance();
}

void World::SortGenerateJobsByDistance()
{
	if (m_generateJobsQueued.empty())
		return;

	Vec3 playerPos = m_player->m_position;

	std::sort(m_generateJobsQueued.begin(),
		m_generateJobsQueued.end(),
		[playerPos](GenerateChunkJob const* a, GenerateChunkJob const* b) {
			IntVec2 centerA = Chunk::GetChunkCenter(a->GetChunkCoords());
			IntVec2 centerB = Chunk::GetChunkCenter(b->GetChunkCoords());

			float distA = GetDistanceSquared2D(
				Vec2(playerPos.x, playerPos.y),
				Vec2((float)centerA.x, (float)centerA.y)
			);
			float distB = GetDistanceSquared2D(
				Vec2(playerPos.x, playerPos.y),
				Vec2((float)centerB.x, (float)centerB.y)
			);

			return distA < distB;  
		}
	);
}

void World::SortLoadJobsByDistance()
{
	if (m_loadJobsQueued.empty())
		return;

	Vec3 playerPos = m_player->m_position;

	std::sort(m_loadJobsQueued.begin(),
		m_loadJobsQueued.end(),
		[playerPos](LoadChunkJob const* a, LoadChunkJob const* b) {
			IntVec2 centerA = Chunk::GetChunkCenter(a->GetChunkCoords());
			IntVec2 centerB = Chunk::GetChunkCenter(b->GetChunkCoords());

			float distA = GetDistanceSquared2D(
				Vec2(playerPos.x, playerPos.y),
				Vec2((float)centerA.x, (float)centerA.y)
			);
			float distB = GetDistanceSquared2D(
				Vec2(playerPos.x, playerPos.y),
				Vec2((float)centerB.x, (float)centerB.y)
			);

			return distA < distB;
		}
	);
}

void World::CheckDirtyChunks()
{
	for (Chunk* chunk : m_chunkUpdateList)
	{
		if (chunk&&chunk->m_isDirty)
		{
			// check if already in the queue
			auto it = std::find(m_chunksQueuedForMeshRebuild.begin(),
				m_chunksQueuedForMeshRebuild.end(),chunk);

			if (it == m_chunksQueuedForMeshRebuild.end())
			{
				m_chunksQueuedForMeshRebuild.push_back(chunk);
			}
		}
	}
}

void World::SortMeshRebuildQueueByDistance()
{
	if (m_chunksQueuedForMeshRebuild.empty())
		return;

	Vec3 playerPos = m_player->m_position;

	std::sort(m_chunksQueuedForMeshRebuild.begin(),
		m_chunksQueuedForMeshRebuild.end(),
		[playerPos](Chunk const* a, Chunk const* b) {
			IntVec2 centerA = a->GetChunkCenter();
			IntVec2 centerB = b->GetChunkCenter();

			float distA = GetDistanceSquared2D(Vec2(playerPos.x, playerPos.y), Vec2((float)centerA.x, (float)centerA.y));
			float distB = GetDistanceSquared2D(Vec2(playerPos.x, playerPos.y), Vec2((float)centerB.x, (float)centerB.y));

			return distA < distB;
		});
}

void World::RebuildChunkMeshes()
{
	int rebuiltThisFrame = 0;

	while (!m_chunksQueuedForMeshRebuild.empty() &&
		rebuiltThisFrame < m_maxMeshRebuildsPerFrame)
	{
		Chunk* chunk = m_chunksQueuedForMeshRebuild.front();
		m_chunksQueuedForMeshRebuild.pop_front();
		if (!chunk->m_neighborNorth || !chunk->m_neighborEast || !chunk->m_neighborSouth || !chunk->m_neighborWest)
			continue;
		// Too far
		Vec2 playerPosXY = Vec2(m_player->m_position.x, m_player->m_position.y);
		IntVec2 chunkCenter = chunk->GetChunkCenter();
		float distance = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
		if (distance > CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE)
		{
			chunk->m_isDirty = false;
			continue;
		}

		// Inactive chunk
		if (chunk->m_state != ChunkState::ACTIVE)
		{
			chunk->m_isDirty = false;
			continue;
		}

		chunk->RebuildMeshWithCulling();
		rebuiltThisFrame++;
	}
}

void World::CancelPendingJobs()
{
	for (GenerateChunkJob* job : m_generateJobsQueued)
	{
		delete job;
	}
	m_generateJobsQueued.clear();
	m_chunksGenerating.clear();

	for (LoadChunkJob* job : m_loadJobsQueued)
	{
		delete job;
	}
	m_loadJobsQueued.clear();
	m_chunksLoading.clear();

}

//void World::InitializeChunks()
//{
//
//}
//
//void World::ActivateChunk(IntVec2 const& chunkCoords)
//{
//	Chunk* newChunk = new Chunk(chunkCoords);
//	m_activeChunks[chunkCoords] = newChunk;
//	m_chunkUpdateList.push_back(newChunk);
//}
//
//void World::DeactivateChunk(Chunk* farChunk)
//{
//	if (farChunk)
//	{
//		if (farChunk->m_needsSaving)
//		{
//			farChunk->SaveChunkToFile("Saves");
//		}
//
//		// erase from list and map
//		auto vecIt = std::find(m_chunkUpdateList.begin(), m_chunkUpdateList.end(), farChunk);
//		if (vecIt != m_chunkUpdateList.end()) {
//			m_chunkUpdateList.erase(vecIt);
//		}
//
//		auto it = m_activeChunks.find(farChunk->m_chunkCoords);
//		if (it != m_activeChunks.end()) {
//			m_activeChunks.erase(it);
//		}
//		delete farChunk;
//		farChunk = nullptr;
//	}
//}
//
//Chunk* World::FindNearestDirtyChunk(Vec3 const& playerPos)
//{
//	Chunk* nearest = nullptr;
//	float nearestDistSq = FLT_MAX;
//
//	for (Chunk* chunk : m_chunkUpdateList) {
//		if (chunk->IsDirty()) {
//			Vec2 playerPosXY =Vec2(playerPos.x,playerPos.y);
//			IntVec2 chunkCenter = chunk->GetChunkCenter();
//			float dist = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
//			if (dist < nearestDistSq) {
//				nearest = chunk;
//				nearestDistSq = dist;
//			}
//		}
//	}
//	return nearest;
//}
//
//bool World::FindNearestMissingChunk(Vec3 const& playerPos, IntVec2& chunkCoords)
//{
//	IntVec2 playerChunkCoords = Chunk::GetChunkCoords(playerPos);
//
//	float nearestDistSq = FLT_MAX;
//	IntVec2 nearestMissing(-999999, -999999);
//	bool found = false;
//
//	for (int dx = -CHUNK_ACTIVATION_RADIUS_X; dx <= CHUNK_ACTIVATION_RADIUS_X; ++dx) {
//		for (int dy = -CHUNK_ACTIVATION_RADIUS_Y; dy <= CHUNK_ACTIVATION_RADIUS_Y; ++dy) {
//			IntVec2 candidateCoords = playerChunkCoords + IntVec2(dx, dy);
//
//			if (!IsChunkInActivationRange(candidateCoords, playerPos)) {
//				continue;
//			}
//
//			if (m_activeChunks.find(candidateCoords) != m_activeChunks.end()) {
//				continue;
//			}
//
//			IntVec2 chunkCenter = Chunk::GetChunkCenter(candidateCoords);
//			float dx_world = playerPos.x - static_cast<float>(chunkCenter.x);
//			float dy_world = playerPos.y - static_cast<float>(chunkCenter.y);
//			float distSq = dx_world * dx_world + dy_world * dy_world;
//
//			if (distSq < nearestDistSq) {
//				nearestDistSq = distSq;
//				nearestMissing = candidateCoords;
//				found = true;
//			}
//		}
//	}
//
//	if (found) {
//		chunkCoords = nearestMissing;
//		return true;
//	}
//	return false;
//}
//
//Chunk* World::FindFarestOutrangeChunk(Vec3 const& playerPos)
//{
//	Chunk* farthestChunk = nullptr;
//	float farthestDistSq = 0.0f;
//
//	static float deactivationRangeSq = static_cast<float>(CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE);
//
//	for (Chunk* chunk : m_chunkUpdateList) {
//		IntVec2 chunkCenter = Chunk::GetChunkCenter(chunk->GetChunkCoords());
//
//		float dx = playerPos.x - static_cast<float>(chunkCenter.x);
//		float dy = playerPos.y - static_cast<float>(chunkCenter.y);
//		float distSq = dx * dx + dy * dy;
//
//		if (distSq > deactivationRangeSq && distSq > farthestDistSq) {
//			farthestChunk = chunk;
//			farthestDistSq = distSq;
//		}
//	}
//
//	return farthestChunk;
//}
//
//bool World::IsChunkInActivationRange(const IntVec2& chunkCoords, const Vec3& playerPos) const
//{
//	IntVec2 chunkCenter = Chunk::GetChunkCenter(chunkCoords);
//
//	float dx = playerPos.x - static_cast<float>(chunkCenter.x);
//	float dy = playerPos.y - static_cast<float>(chunkCenter.y);
//	float distanceSquared = dx * dx + dy * dy;
//
//	float activationRangeSquared = static_cast<float>(CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE);
//	return distanceSquared <= activationRangeSquared;
//}

void World::GetChunkStateCounts(int& constructing, int& generating, int& loading, int& active, int& saving) const
{
	constructing = 0;
	generating = 0;
	loading = 0;
	active = 0;
	saving = 0;

	for (auto const& pair : m_activeChunks)
	{
		Chunk const* chunk = pair.second;
		if (!chunk) continue;

		ChunkState state = chunk->m_state.load();

		switch (state)
		{
		case ChunkState::CONSTRUCTING:
			constructing++;
			break;
		case ChunkState::ACTIVATING_GENERATING:
			generating++;
			break;
		case ChunkState::ACTIVATING_LOADING:
			loading++;
			break;
		case ChunkState::ACTIVE:
			active++;
			break;
		case ChunkState::DEACTIVATING_SAVING:
			saving++;
			break;
		default:
			break;
		}
	}

	generating += (int)m_chunksGenerating.size();
	loading += (int)m_chunksLoading.size();
	saving += (int)m_chunksSaving.size();
}

void World::GetJobCounts(int& generateJobsQueued, int& generateJobsInFlight, int& loadJobsQueued, int& loadJobsInFlight, int& saveJobsQueued, int& saveJobsInFlight, int& meshRebuildsQueued) const
{
	generateJobsQueued = (int)m_generateJobsQueued.size();
	generateJobsInFlight = (int)m_chunksGenerating.size();

	loadJobsQueued = (int)m_loadJobsQueued.size();
	loadJobsInFlight = (int)m_chunksLoading.size();

	saveJobsQueued = (int)m_saveJobsQueued.size();
	saveJobsInFlight = (int)m_chunksSaving.size();

	meshRebuildsQueued = (int)m_chunksQueuedForMeshRebuild.size();
}

void World::InitializeChunkLighting(Chunk* chunk)
{
	if (!chunk)
		return;

	MarkBoundaryBlocksAsDirty(chunk);

	MarkSkyBlocks(chunk);

	SetSkyLightAndMarkNeighbors(chunk);

	MarkLightEmittingBlocksAsDirty(chunk);
}

void World::MarkBoundaryBlocksAsDirty(Chunk* chunk)
{
	if (chunk->m_neighborEast)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; ++z)
		{
			for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			{
				int index = chunk->LocalCoordsToIndex(CHUNK_MAX_X, y, z);
				BlockIterator blockIter(chunk, index);

				// If non-opaque, mark as dirty to receive light from neighbor
				if (!blockIter.IsOpaque())
				{
					MarkLightingDirty(blockIter);
				}
			}
		}
	}

	// Check West face (x = 0)
	if (chunk->m_neighborWest)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; ++z)
		{
			for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			{
				int index = chunk->LocalCoordsToIndex(0, y, z);
				BlockIterator blockIter(chunk, index);

				if (!blockIter.IsOpaque())
				{
					MarkLightingDirty(blockIter);
				}
			}
		}
	}

	// Check North face (y = CHUNK_MAX_Y)
	if (chunk->m_neighborNorth)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; ++z)
		{
			for (int x = 0; x < CHUNK_SIZE_X; ++x)
			{
				int index = chunk->LocalCoordsToIndex(x, CHUNK_MAX_Y, z);
				BlockIterator blockIter(chunk, index);

				if (!blockIter.IsOpaque())
				{
					MarkLightingDirty(blockIter);
				}
			}
		}
	}

	// Check South face (y = 0)
	if (chunk->m_neighborSouth)
	{
		for (int z = 0; z < CHUNK_SIZE_Z; ++z)
		{
			for (int x = 0; x < CHUNK_SIZE_X; ++x)
			{
				int index = chunk->LocalCoordsToIndex(x, 0, z);
				BlockIterator blockIter(chunk, index);

				if (!blockIter.IsOpaque())
				{
					MarkLightingDirty(blockIter);
				}
			}
		}
	}
}

void World::MarkSkyBlocks(Chunk* chunk)
{
	for (int y = 0; y < CHUNK_SIZE_Y; ++y)
	{
		for (int x = 0; x < CHUNK_SIZE_X; ++x)
		{
			// Descend from top (z = CHUNK_MAX_Z) downward
			for (int z = CHUNK_MAX_Z; z >= 0; --z)
			{
				int index = chunk->LocalCoordsToIndex(x, y, z);
				Block* block = &chunk->m_blocks[index];

				// If this block is opaque, stop descending this column
				// Everything below is underground
				if (block->IsFullOpaque())
				{
					break;
				}

				// This is a non-opaque block with no opaque blocks above it
				// Mark it as SKY
				block->SetIsSky(true);
			}
		}
	}
}

void World::SetSkyLightAndMarkNeighbors(Chunk* chunk)
{
	for (int y = 0; y < CHUNK_SIZE_Y; ++y)
	{
		for (int x = 0; x < CHUNK_SIZE_X; ++x)
		{
			// Descend from top until first opaque
			for (int z = CHUNK_MAX_Z; z >= 0; --z)
			{
				int index = chunk->LocalCoordsToIndex(x, y, z);
				Block* block = &chunk->m_blocks[index];
				
				// If opaque, stop descending this column
				if (block->IsFullOpaque())
				{
					break;
				}
				
				// This is a sky block, set outdoor light to maximum
				block->SetOutdoorLightInfluence(15);
				
				// Check the 4 horizontal neighbors and mark non-sky air blocks as dirty
				BlockIterator centerIter(chunk, index);
				
				// East neighbor (+X)
				BlockIterator eastNeighbor = centerIter.GetFwdX();
				if (eastNeighbor.IsValid())
				{
					Block* eastBlock = eastNeighbor.GetBlockPtr();
					// If non-opaque and NOT sky, it should receive light from this sky block
					if (eastBlock && !eastBlock->IsFullOpaque() && !eastBlock->IsSky())
					{
						MarkLightingDirty(eastNeighbor);
					}
				}
				
				// West neighbor (-X)
				BlockIterator westNeighbor = centerIter.GetNegX();
				if (westNeighbor.IsValid())
				{
					Block* westBlock = westNeighbor.GetBlockPtr();
					if (westBlock && !westBlock->IsFullOpaque() && !westBlock->IsSky())
					{
						MarkLightingDirty(westNeighbor);
					}
				}
				
				// North neighbor (+Y)
				BlockIterator northNeighbor = centerIter.GetFwdY();
				if (northNeighbor.IsValid())
				{
					Block* northBlock = northNeighbor.GetBlockPtr();
					if (northBlock && !northBlock->IsFullOpaque() && !northBlock->IsSky())
					{
						MarkLightingDirty(northNeighbor);
					}
				}
				
				// South neighbor (-Y)
				BlockIterator southNeighbor = centerIter.GetNegY();
				if (southNeighbor.IsValid())
				{
					Block* southBlock = southNeighbor.GetBlockPtr();
					if (southBlock && !southBlock->IsFullOpaque() && !southBlock->IsSky())
					{
						MarkLightingDirty(southNeighbor);
					}
				}
			}
		}
	}
}

void World::MarkLightEmittingBlocksAsDirty(Chunk* chunk)
{
	// Loop through every block in the chunk
	for (int index = 0; index < BLOCKS_PER_CHUNK; ++index)
	{
		Block const& block = chunk->m_blocks[index];
		uint8_t typeIndex = block.GetTypeIndex();

		// Check if this block type emits light
		BlockDefinition const& blockDef = BlockDefinition::s_blockDefs[typeIndex];

		if (blockDef.m_indoorLight > 0)
		{
			// This block emits light, mark it as dirty so it will be processed
			BlockIterator blockIter(chunk, index);
			MarkLightingDirty(blockIter);
		}
	}
}

void World::ProcessDirtyLighting()
{
	while (!m_dirtyLightingQueue.empty())
	{
		ProcessNextDirtyLightBlock();
	}
}

void World::ProcessNextDirtyLightBlock()
{
	if (m_dirtyLightingQueue.empty())
		return;

	// Pop front block from queue
	BlockIterator blockIter = m_dirtyLightingQueue.front();
	m_dirtyLightingQueue.pop_front();

	if (!blockIter.IsValid())
		return;

	Block* block = blockIter.GetBlockPtr();

	// Clear dirty flag
	block->SetIsLightDirty(false);

	// ===================================================
	// Step 2: Compute theoretically-correct light influences
	uint8_t correctIndoorLight = ComputeCorrectIndoorLight(block, blockIter);
	uint8_t correctOutdoorLight = ComputeCorrectOutdoorLight(block, blockIter);

	// Step 3: Get current light influences
	uint8_t currentIndoorLight = block->GetIndoorLightInfluence();
	uint8_t currentOutdoorLight = block->GetOutdoorLightInfluence();

	// Step 4: Compare - if either is incorrect, update
	if (correctIndoorLight != currentIndoorLight || correctOutdoorLight != currentOutdoorLight)
	{
		// Update the light influence values
		block->SetIndoorLightInfluence(correctIndoorLight);
		block->SetOutdoorLightInfluence(correctOutdoorLight);

		// Mark this chunk as needing mesh rebuild (vertex colors changed)
		Chunk* chunk = blockIter.GetChunk();
		if (chunk && !chunk->IsDirty())
		{
			chunk->SetDirty(true);
		}

		MarkNeighborsAsDirtyIfNotOpaque(blockIter);
	}
}

void World::MarkLightingDirty(BlockIterator& blockIter)
{
	if (!blockIter.IsValid())
		return;

	Block* block = blockIter.GetBlockPtr();

	if (block->IsLightDirty())
		return;

	m_dirtyLightingQueue.push_back(blockIter);
	block->SetIsLightDirty(true);
}

void World::UndirtyAllBlocksInChunk(Chunk* chunk)
{
	if (!chunk)
		return;

	// Remove all blocks from this chunk from the dirty queue
	auto it = m_dirtyLightingQueue.begin();
	while (it != m_dirtyLightingQueue.end())
	{
		if (it->GetChunk() == chunk)
		{
			Block* block = it->GetBlockPtr();
			if (block)
			{
				block->SetIsLightDirty(false);
			}
			it = m_dirtyLightingQueue.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void World::MarkLightingDirtyIfNotOpaque(BlockIterator& blockIter)
{
	if (!blockIter.IsValid())
		return;

	if (!blockIter.IsOpaque())
	{
		MarkLightingDirty(blockIter);
	}
}

void World::MarkNeighborsAsDirtyIfNotOpaque(BlockIterator& centerIter)
{
	BlockIterator neighbors[6] = {
	centerIter.GetFwdX(),  // East
	centerIter.GetNegX(),  // West
	centerIter.GetFwdY(),  // North
	centerIter.GetNegY(),  // South
	centerIter.GetFwdZ(),  // Up
	centerIter.GetNegZ()   // Down
	};

	for (int i = 0; i < 6; ++i)
	{
		if (neighbors[i].IsValid())
		{
			MarkLightingDirtyIfNotOpaque(neighbors[i]);
			neighbors[i].GetChunk()->SetDirty(true);
		}
	}
}

uint8_t World::ComputeCorrectIndoorLight(Block* block, BlockIterator& blockIter)
{
	int correctLight = 0;

	// Rule 2: Check if this block type emits indoor light
	uint8_t typeIndex = block->GetTypeIndex();
	BlockDefinition const& blockDef = BlockDefinition::s_blockDefs[typeIndex];
	correctLight = blockDef.m_indoorLight;

	// Rule 3: If non-opaque, get light from neighbors (with falloff)
	if (!block->IsFullOpaque())
	{
		// Check all 6 neighbors
		BlockIterator neighbors[6] = {
			blockIter.GetFwdX(),  // East
			blockIter.GetNegX(),  // West
			blockIter.GetFwdY(),  // North
			blockIter.GetNegY(),  // South
			blockIter.GetFwdZ(),  // Up
			blockIter.GetNegZ()   // Down
		};

		for (int i = 0; i < 6; ++i)
		{
			if (neighbors[i].IsValid())
			{
				Block neighborBlock = neighbors[i].GetBlock();
				int neighborLight = neighborBlock.GetIndoorLightInfluence();
				int propagatedLight = neighborLight - 1; // Light falloff
				if (propagatedLight > correctLight)
				{
					correctLight = propagatedLight;
				}	
			}
		}
	}

	// Clamp to valid range [0, 15]
	correctLight = GetClamped(correctLight, 0, 15);

	return (uint8_t)correctLight;
}

uint8_t World::ComputeCorrectOutdoorLight(Block* block, BlockIterator& blockIter)
{
	if (block->IsSky())
	{
		return 15;
	}

	int correctLight = 0;

	// Rule 2: Check if this block type emits outdoor light (usually 0)
	uint8_t typeIndex = block->GetTypeIndex();
	BlockDefinition const& blockDef = BlockDefinition::s_blockDefs[typeIndex];
	correctLight = blockDef.m_outdoorLight;

	// Rule 3: If non-opaque, get light from neighbors (with falloff)
	if (!block->IsFullOpaque())
	{
		// Check all 6 neighbors
		BlockIterator neighbors[6] = {
			blockIter.GetFwdX(),  // East
			blockIter.GetNegX(),  // West
			blockIter.GetFwdY(),  // North
			blockIter.GetNegY(),  // South
			blockIter.GetFwdZ(),  // Up
			blockIter.GetNegZ()   // Down
		};

		for (int i = 0; i < 6; ++i)
		{
			if (neighbors[i].IsValid())
			{
				Block neighborBlock = neighbors[i].GetBlock();
				int neighborLight = neighborBlock.GetOutdoorLightInfluence();
				int propagatedLight = neighborLight - 1; // Light falloff
				if (propagatedLight > correctLight)
				{
					correctLight = propagatedLight;
				}
			}
		}
	}

	// Clamp to valid range [0, 15]
	correctLight = GetClamped(correctLight, 0, 15);

	return (uint8_t)correctLight;
}

void World::DigBlock(Chunk* chunk, int blockIndex)
{
	if (!chunk)
		return;

	if (blockIndex < 0 || blockIndex >= BLOCKS_PER_CHUNK)
		return;

	Block* block = &chunk->m_blocks[blockIndex];

	// Step 1: Set new block type to Air
	uint8_t airTypeIndex = BlockDefinition::s_nameToIndexMap["Air"];
	block->SetTypeIndex(airTypeIndex);

	// Update block flags based on new type (Air is non-opaque, non-solid, visible)
	block->SetIsFullOpaque(false);
	block->SetIsSolid(false);
	block->SetIsVisible(true);

	// Step 2: Mark this block's light as dirty
	BlockIterator blockIter(chunk, blockIndex);
	MarkLightingDirty(blockIter);

	// Step 4: Check if block above is SKY
	BlockIterator aboveIter = blockIter.GetFwdZ();
	if (aboveIter.IsValid())
	{
		Block aboveBlock = aboveIter.GetBlock();
		if (aboveBlock.IsSky())
		{
			PropagateSkyFlagDownward(blockIter);
		}
	}

	if (!chunk->IsDirty())
	{
		chunk->SetDirty(true);
	}

	// Mark this chunk as needing to be saved
	chunk->m_needsSaving = true;
}

void World::PlaceBlock(Chunk* chunk, int blockIndex, std::string const& typeName)
{
	if (!chunk)
		return;

	if (blockIndex < 0 || blockIndex >= BLOCKS_PER_CHUNK)
		return;

	// Check if block type exists
	auto it = BlockDefinition::s_nameToIndexMap.find(typeName);
	if (it == BlockDefinition::s_nameToIndexMap.end())
	{
		// Block type not found
		return;
	}

	Block* block = &chunk->m_blocks[blockIndex];

	// Remember if the block being replaced was SKY
	bool wasReplacingSkyBlock = block->IsSky();

	// Step 1: Set new block type
	uint8_t newTypeIndex = it->second;
	block->SetTypeIndex(newTypeIndex);

	// Update block flags based on new type
	BlockDefinition const& blockDef = BlockDefinition::s_blockDefs[newTypeIndex];
	block->SetIsFullOpaque(blockDef.m_isOpaque);
	block->SetIsSolid(blockDef.m_isSolid);
	block->SetIsVisible(blockDef.m_isVisible);

	// Step 2: Mark this block's light as dirty
	BlockIterator blockIter(chunk, blockIndex);
	MarkLightingDirty(blockIter);

	// Step 4: Check if we replaced a SKY block with an opaque block
	if (wasReplacingSkyBlock && blockDef.m_isOpaque)
	{
		// Clear this block's SKY flag
		block->SetIsSky(false);

		// Descend downward, clearing all SKY flags and marking as dirty
		// This cuts off the sunlight beam
		ClearSkyFlagsDownward(blockIter);
	}

	// Mark chunk as needing mesh rebuild
	if (!chunk->IsDirty())
	{
		chunk->SetDirty(true);
	}

	// Mark this chunk as needing to be saved
	chunk->m_needsSaving = true;
}

void World::PropagateSkyFlagDownward(BlockIterator startIter)
{
	BlockIterator currentIter = startIter;

	while (currentIter.IsValid())
	{
		Block* currentBlock = currentIter.GetBlockPtr();
		if (!currentBlock)
			break;

		// If we hit an opaque block, stop
		if (currentBlock->IsFullOpaque())
		{
			break;
		}

		// This is a non-opaque block, mark it as SKY
		currentBlock->SetIsSky(true);

		// Set its outdoor light influence to maximum (direct sunlight)
		currentBlock->SetOutdoorLightInfluence(15);

		// Mark it as dirty so lighting propagates to neighbors
		MarkLightingDirty(currentIter);

		// Move down to next block
		currentIter = currentIter.GetNegZ();
	}
}

void World::ClearSkyFlagsDownward(BlockIterator startIter)
{
	BlockIterator currentIter = startIter.GetNegZ();

	while (currentIter.IsValid())
	{
		Block* currentBlock = currentIter.GetBlockPtr();
		if (!currentBlock)
			break;

		if (!currentBlock->IsSky())
		{
			break;
		}

		// Clear the SKY flag
		currentBlock->SetIsSky(false);

		// Mark as dirty - lighting will recalculate (should become much darker)
		MarkLightingDirty(currentIter);

		// If we hit an opaque block, stop
		if (currentBlock->IsFullOpaque())
		{
			break;
		}

		// Move down to next block
		currentIter = currentIter.GetNegZ();
	}
}

void World::UpdateWorldRenderConstants()
{
}

void World::UpdateDayNightCycle()
{
}

void World::SetWorldConstantsToGPU() const
{
	if (!m_player || !m_worldConstantBuffer)
		return;

	// ========== 组装常量数据 ==========
	WorldConstants constants = {};

	Vec3 cameraPos = m_player->m_position;
	constants.CameraPosition = cameraPos;
	constants.Padding1 = 0.0f;

	// 室内光颜色（归一化到 0-1）
	constants.IndoorLightColor[0] = m_indoorLightColor.r / 255.0f;
	constants.IndoorLightColor[1] = m_indoorLightColor.g / 255.0f;
	constants.IndoorLightColor[2] = m_indoorLightColor.b / 255.0f;
	constants.IndoorLightColor[3] = m_indoorLightColor.a / 255.0f;

	// 室外光颜色
	constants.OutdoorLightColor[0] = m_outdoorLightColor.r / 255.0f;
	constants.OutdoorLightColor[1] = m_outdoorLightColor.g / 255.0f;
	constants.OutdoorLightColor[2] = m_outdoorLightColor.b / 255.0f;
	constants.OutdoorLightColor[3] = m_outdoorLightColor.a / 255.0f;

	// 天空/雾颜色
	constants.SkyColor[0] = m_skyColor.r / 255.0f;
	constants.SkyColor[1] = m_skyColor.g / 255.0f;
	constants.SkyColor[2] = m_skyColor.b / 255.0f;
	constants.SkyColor[3] = m_skyColor.a / 255.0f;

	// 雾距离
	constants.FogNearDistance = m_fogNearDistance;
	constants.FogFarDistance = m_fogFarDistance;

	// ========== 使用 Renderer 的通用接口上传和绑定 ==========
	g_theRenderer->CopyConstantBufferToGPU(&constants, sizeof(WorldConstants), m_worldConstantBuffer);
	g_theRenderer->BindConstantBuffer(9, m_worldConstantBuffer);
}

void World::DigBlockByPlayerPos(Vec3 const& playerPos)
{
	if (playerPos.z >= 127.999f) return;
	IntVec2 chunkCoords = Chunk::GetChunkCoords(playerPos);
	auto it = m_activeChunks.find(chunkCoords);
	Chunk* curChunk = nullptr;
	if (it != m_activeChunks.end())
	{
		curChunk = it->second;
	}
	else return;

	curChunk->m_needsSaving = true;

	IntVec3 globalCoords=curChunk->GetGlobalCoords(playerPos);
	IntVec3 localCoords = curChunk->GlobalCoordsToLocalCoords(globalCoords);
	for (int h = std::min(localCoords.z,CHUNK_SIZE_Z-1); h >= 0; h--)
	{
		int blockIndex = curChunk->LocalCoordsToIndex(IntVec3(localCoords.x, localCoords.y, h));
		if (curChunk->GetBlock(blockIndex).GetTypeIndex()!=0)
		{
			DigBlock(curChunk, blockIndex);
			//--------------------------
			//Block block = BlockDefinition::s_nameToIndexMap["Air"];
			//curChunk->SetBlock(blockIndex, block);
			break;
		}
	}
}

void World::PlaceBlockByPlayerPos(std::string const& typeName, Vec3 const& playerPos)
{
	if (playerPos.z >= 127.999f) return;
	IntVec2 chunkCoords = Chunk::GetChunkCoords(playerPos);
	auto it = m_activeChunks.find(chunkCoords);
	Chunk* curChunk = nullptr;
	if (it != m_activeChunks.end())
	{
		curChunk = it->second;
	}
	else return;

	curChunk->m_needsSaving = true;

	IntVec3 globalCoords = curChunk->GetGlobalCoords(playerPos);
	IntVec3 localCoords = curChunk->GlobalCoordsToLocalCoords(globalCoords);
	for (int h = 0; h < CHUNK_SIZE_Z; h++)
	{   
		int blockIndex = curChunk->LocalCoordsToIndex(IntVec3(localCoords.x, localCoords.y, h));
		if (curChunk->GetBlock(blockIndex).GetTypeIndex() == 0)
		{
			PlaceBlock(curChunk, blockIndex, typeName);
			// ---------------------------
			//Block block = BlockDefinition::s_nameToIndexMap[typeName];
			//curChunk->SetBlock(blockIndex, block);
			break;
		}
	}
}

void World::HookUpChunkNeighbors(Chunk* chunk)
{
	if (!chunk) return;

	IntVec2 coords = chunk->GetChunkCoords();

	IntVec2 eastCoords = coords + IntVec2(1, 0);
	IntVec2 westCoords = coords + IntVec2(-1, 0);
	IntVec2 northCoords = coords + IntVec2(0, 1);
	IntVec2 southCoords = coords + IntVec2(0, -1);

	auto eastIt = m_activeChunks.find(eastCoords);
	if (eastIt != m_activeChunks.end()) {
		chunk->m_neighborEast = eastIt->second;
		eastIt->second->m_neighborWest = chunk; 
		eastIt->second->m_isDirty = true;
	}

	auto westIt = m_activeChunks.find(westCoords);
	if (westIt != m_activeChunks.end()) {
		chunk->m_neighborWest = westIt->second;
		westIt->second->m_neighborEast = chunk;
		westIt->second->m_isDirty = true;
	}

	auto northIt = m_activeChunks.find(northCoords);
	if (northIt != m_activeChunks.end()) {
		chunk->m_neighborNorth = northIt->second;
		northIt->second->m_neighborSouth = chunk;
		northIt->second->m_isDirty = true;
	}

	auto southIt = m_activeChunks.find(southCoords);
	if (southIt != m_activeChunks.end()) {
		chunk->m_neighborSouth = southIt->second;
		southIt->second->m_neighborNorth = chunk;
		southIt->second->m_isDirty = true;
	}
}

void World::UnhookChunkNeighbors(Chunk* chunk)
{
	if (!chunk) return;

	if (chunk->m_neighborEast) {
		chunk->m_neighborEast->m_neighborWest = nullptr;
		chunk->m_neighborEast = nullptr;
	}

	if (chunk->m_neighborWest) {
		chunk->m_neighborWest->m_neighborEast = nullptr;
		chunk->m_neighborWest = nullptr;
	}

	if (chunk->m_neighborNorth) {
		chunk->m_neighborNorth->m_neighborSouth = nullptr;
		chunk->m_neighborNorth = nullptr;
	}

	if (chunk->m_neighborSouth) {
		chunk->m_neighborSouth->m_neighborNorth = nullptr;
		chunk->m_neighborSouth = nullptr;
	}
}
