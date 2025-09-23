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

extern Game* g_theGame;

World::World(Player* player):m_player(player)
{
	//InitializeChunks();
	m_player->SetCurWorld(this);
}

World::~World()
{
	m_player = nullptr;
	for (Chunk* chunk : m_chunkUpdateList)
	{
		if (chunk && chunk->m_needsSaving)
		{
			chunk->SaveChunkToFile("Saves");
		}
	}

	for (Chunk* chunk : m_chunkUpdateList)
	{
		delete chunk;
	}

	m_chunkUpdateList.clear();
	m_activeChunks.clear();
}

void World::Update(float deltaTime)
{
	m_player->Update(deltaTime);

	// debug draw basis
	Vec3 basisPos = m_player->m_position + m_player->m_orientation.GetForward_IFwd() * 50.f;
	Mat44 basisTransform = Mat44::MakeTranslation3D(basisPos);
	DebugAddWorldBasis(basisTransform, 0.f, DebugRenderMode::ALWAYS, 1.5f);
	DebugAddWorldBasis(Mat44(), 0.f, DebugRenderMode::USE_DEPTH, 1.f);

	// rebuild nearest dirty mesh chunk
	Chunk* nearestDirtyChunk = FindNearestDirtyChunk(m_player->m_position);
	if (nearestDirtyChunk) {
		nearestDirtyChunk->RebuildMeshWithCulling();
	}

	// or activate a nearest missing chunk
	else if (m_chunkUpdateList.size() < MAX_ACTIVE_CHUNKS) {
		IntVec2 chunkCoords = IntVec2(-999999, -999999);
		if (FindNearestMissingChunk(m_player->m_position, chunkCoords))
		{
			ActivateChunk(chunkCoords);
		}
	}

	// or deactivate farthest active chunk if its center is outside the deactivation range
	else
	{
		Chunk* farestChunk = FindFarestOutrangeChunk(m_player->m_position);
		if (farestChunk)
		{
			DeactivateChunk(farestChunk);
		}
	}


	for (Chunk* chunk : m_chunkUpdateList)
	{
		chunk->Update();
	}

}

void World::Render() const
{
	for (Chunk* chunk : m_chunkUpdateList)
	{
		chunk->Render();
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
			"Chunk: %d  Vertices: %d  Indices: %d\n"
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
}

void World::ToggleDebugDraw()
{
	m_isDebugDraw = !m_isDebugDraw;
}

void World::InitializeChunks()
{

}

void World::ActivateChunk(IntVec2 const& chunkCoords)
{
	Chunk* newChunk = new Chunk(chunkCoords);
	m_activeChunks[chunkCoords] = newChunk;
	m_chunkUpdateList.push_back(newChunk);
}

void World::DeactivateChunk(Chunk* farChunk)
{
	if (farChunk)
	{
		if (farChunk->m_needsSaving)
		{
			farChunk->SaveChunkToFile("Saves");
		}

		// erase from list and map
		auto vecIt = std::find(m_chunkUpdateList.begin(), m_chunkUpdateList.end(), farChunk);
		if (vecIt != m_chunkUpdateList.end()) {
			m_chunkUpdateList.erase(vecIt);
		}

		auto it = m_activeChunks.find(farChunk->m_chunkCoords);
		if (it != m_activeChunks.end()) {
			m_activeChunks.erase(it);
		}
		delete farChunk;
		farChunk = nullptr;
	}
}

Chunk* World::FindNearestDirtyChunk(Vec3 const& playerPos)
{
	Chunk* nearest = nullptr;
	float nearestDistSq = FLT_MAX;

	for (Chunk* chunk : m_chunkUpdateList) {
		if (chunk->IsDirty()) {
			Vec2 playerPosXY =Vec2(playerPos.x,playerPos.y);
			IntVec2 chunkCenter = chunk->GetChunkCenter();
			float dist = GetDistanceSquared2D(playerPosXY, Vec2((float)chunkCenter.x, (float)chunkCenter.y));
			if (dist < nearestDistSq) {
				nearest = chunk;
				nearestDistSq = dist;
			}
		}
	}
	return nearest;
}

bool World::FindNearestMissingChunk(Vec3 const& playerPos, IntVec2& chunkCoords)
{
	IntVec2 playerChunkCoords = Chunk::GetChunkCoords(playerPos);

	float nearestDistSq = FLT_MAX;
	IntVec2 nearestMissing(-999999, -999999);
	bool found = false;

	for (int dx = -CHUNK_ACTIVATION_RADIUS_X; dx <= CHUNK_ACTIVATION_RADIUS_X; ++dx) {
		for (int dy = -CHUNK_ACTIVATION_RADIUS_Y; dy <= CHUNK_ACTIVATION_RADIUS_Y; ++dy) {
			IntVec2 candidateCoords = playerChunkCoords + IntVec2(dx, dy);

			if (!IsChunkInActivationRange(candidateCoords, playerPos)) {
				continue;
			}

			if (m_activeChunks.find(candidateCoords) != m_activeChunks.end()) {
				continue;
			}

			IntVec2 chunkCenter = Chunk::GetChunkCenter(candidateCoords);
			float dx_world = playerPos.x - static_cast<float>(chunkCenter.x);
			float dy_world = playerPos.y - static_cast<float>(chunkCenter.y);
			float distSq = dx_world * dx_world + dy_world * dy_world;

			if (distSq < nearestDistSq) {
				nearestDistSq = distSq;
				nearestMissing = candidateCoords;
				found = true;
			}
		}
	}

	if (found) {
		chunkCoords = nearestMissing;
		return true;
	}
	return false;
}

Chunk* World::FindFarestOutrangeChunk(Vec3 const& playerPos)
{
	Chunk* farthestChunk = nullptr;
	float farthestDistSq = 0.0f;

	static float deactivationRangeSq = static_cast<float>(CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE);

	for (Chunk* chunk : m_chunkUpdateList) {
		IntVec2 chunkCenter = Chunk::GetChunkCenter(chunk->GetChunkCoords());

		float dx = playerPos.x - static_cast<float>(chunkCenter.x);
		float dy = playerPos.y - static_cast<float>(chunkCenter.y);
		float distSq = dx * dx + dy * dy;

		if (distSq > deactivationRangeSq && distSq > farthestDistSq) {
			farthestChunk = chunk;
			farthestDistSq = distSq;
		}
	}

	return farthestChunk;
}

bool World::IsChunkInActivationRange(const IntVec2& chunkCoords, const Vec3& playerPos) const
{
	IntVec2 chunkCenter = Chunk::GetChunkCenter(chunkCoords);

	float dx = playerPos.x - static_cast<float>(chunkCenter.x);
	float dy = playerPos.y - static_cast<float>(chunkCenter.y);
	float distanceSquared = dx * dx + dy * dy;

	float activationRangeSquared = static_cast<float>(CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE);
	return distanceSquared <= activationRangeSquared;
}

void World::DigBlock(Vec3 const& playerPos)
{
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
			Block block = BlockDefinition::s_nameToIndexMap["Air"];
			curChunk->SetBlock(blockIndex, block);
			break;
		}
	}
}

void World::PlaceBlock(std::string const& typeName, Vec3 const& playerPos)
{
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
			Block block = BlockDefinition::s_nameToIndexMap[typeName];
			curChunk->SetBlock(blockIndex, block);
			break;
		}
	}
}
