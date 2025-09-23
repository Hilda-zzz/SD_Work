#pragma once
#include <vector>
#include <unordered_map>
#include "Engine/Math/IntVec2.hpp"
#include <string>
class Chunk;
class Player;
struct Vec3;

struct IntVec2Hash {
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

	void ToggleDebugDraw();

private:
	void InitializeChunks();
	void ActivateChunk(IntVec2 const& chunkCoords);
	void DeactivateChunk(Chunk* farChunk);

	Chunk* FindNearestDirtyChunk(Vec3 const& playerPos);
	bool FindNearestMissingChunk(Vec3 const& playerPos,IntVec2& chunkCoords);
	Chunk* FindFarestOutrangeChunk(Vec3 const& playerPos);

	bool IsChunkInActivationRange(const IntVec2& chunkCoords, const Vec3& playerPos) const;
public:
	void DigBlock(Vec3 const& playerPos);
	void PlaceBlock(std::string const& typeName, Vec3 const& playerPos);

private:
	std::vector<Chunk*> m_chunkUpdateList;
	std::unordered_map< IntVec2, Chunk*,IntVec2Hash > m_activeChunks;

	Player* m_player = nullptr;
	bool m_isDebugDraw = false;
};