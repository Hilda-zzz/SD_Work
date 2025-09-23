#pragma once
class Chunk;

class TerrainGenerator
{
	friend Chunk;
public:
	static void GenerateBlocksForChunk(Chunk* chunk);

private:
// 	static int CalculateTerrainHeight(int globalX, int globalY);
// 	static uint8_t GetRandomOreType();
// 	static void GenerateCaves(/* params */);
// 	static void GenerateStructures(/* params */);
};