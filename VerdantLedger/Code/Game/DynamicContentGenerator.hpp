#pragma once
#include "Game/TileTypesInGame.hpp"

class TileMap;
struct IntVec2;
class TileChunk;

class DynamicContentGenerator
{
public:
	DynamicContentGenerator() {};
	~DynamicContentGenerator() {};

	void GenerateAllDynamicContentForTheMap(TileMap* curMap);
	void GenerateDynamicContentForEachChunk(TileChunk* curChunk);
	void GenerateObstacleAtPosition(TileMap* curMap, IntVec2 const& gridPos);
	void GenerateObstacleAtPosition(TileChunk* curChunk, IntVec2 const& gridPos);

	void GenerateSpecificObstacle(TileChunk* curChunk, IntVec2 const& gridPos,
		uint64_t tilePosKey, ObstacleType obstacleType);

private:

};