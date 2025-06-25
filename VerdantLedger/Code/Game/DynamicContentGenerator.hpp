#pragma once
class TileMap;
struct IntVec2;

class DynamicContentGenerator
{
public:
	DynamicContentGenerator() {};
	~DynamicContentGenerator() {};

	void GenerateAllDynamicContentForTheMap(TileMap* curMap);
	void GenerateObstacleAtPosition(TileMap* curMap, IntVec2 const& gridPos);

private:

};