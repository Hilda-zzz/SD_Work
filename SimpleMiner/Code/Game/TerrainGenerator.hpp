#pragma once
#include "BiomeTypes.hpp"
#include <vector>
#include "TreeGenerator.hpp"
#include <map>
class Chunk;

class TerrainGenerator
{
	friend Chunk;
public:
	static void GenerateBlocksForChunk(Chunk* chunk);
	static void GenerateBlocksForChunk_New(Chunk* chunk);

private:
// 	static int CalculateTerrainHeight(int globalX, int globalY);
// 	static uint8_t GetRandomOreType();
// 	static void GenerateCaves(/* params */);
// 	static void GenerateStructures(/* params */);

	static void Generate2DNoise(Chunk* chunk);
	static void ReplaceSurface(
		Chunk* chunk,
		std::vector<float> const& extendedDensities,      // 24×24×128
		std::vector<int> const& extendedSurfaceHeights);   // 24×24

	//====================================
	static void GenerateTrees(Chunk* chunk, const std::vector<int>& surfaceHeights);

	static void InitializeBiomeTreeConfigs();
	static BiomeTreeConfig GetBiomeTreeConfig(BiomeType biome);
	static TreeSize DetermineTreeSize(float treeNoise, BiomeType biome, float temp, float humidity);
	static bool IsLocalMaximum3x3(Chunk* chunk, int localX, int localY);

	static bool IsLocalMaximum3x3_Tree(Chunk* chunk, int treeExtX, int treeExtY);

	static std::map<BiomeType, BiomeTreeConfig> s_biomeTreeConfigs;
	static bool s_biomeTreeConfigsInitialized;

	static void InitializeTreeStamps();
	static void InitializeOakTreeStamps();
	static void InitializeBirchStamps();
	static void InitializeSpruceStamps();
	static void InitializeCuctusStamps();
	static void InitializeJungleStamps();
	static void InitializeAcaciaStamps();

	static TreeStamp GetTreeStamp(TreeType type, TreeSize size);
	static bool CanPlaceTree(Chunk* chunk, const TreeCandidate& candidate, const TreeStamp& stamp);
	static void PlaceTree(Chunk* chunk, const TreeCandidate& candidate, const TreeStamp& stamp);
	static int GetDeterministicTreeType(int globalX, int globalY, int numTypes);

	static std::map<TreeType, std::map<TreeSize, TreeStamp>> s_treeStamps;
	static bool s_treeStampsInitialized;

	//====================================

	// Cheese Cave 生成 (使用3D Perlin噪声)
	static float GenerateCheeseCaveNoise(int globalX, int globalY, int globalZ);

	// Spaghetti/Noodle Cave 生成 (使用Ridged噪声交集)
	static float GenerateSpaghettiCaveNoise(int globalX, int globalY, int globalZ);

	// Ridge 噪声辅助函数
	static float ComputeRidgedNoise3D(
		float x, float y, float z,
		float scale, int octaves,
		unsigned int seed);

	// 洞穴密度组合函数
	static bool IsCaveAt(int globalX, int globalY, int globalZ);

	//====================================

	static BiomeType DetermineBiome(
		ContinentalnessLevel continent,
		ErosionLevel erosion,
		PeaksValleysLevel pv,
		TemperatureLevel temp,
		HumidityLevel humidity);

	static BiomeType LookupOceanBiome(ContinentalnessLevel continent, TemperatureLevel temp);
	static BiomeType LookupBeachBiome(TemperatureLevel temp);
	static BiomeType LookupMiddleBiome(TemperatureLevel temp, HumidityLevel humidity);
	static BiomeCategory LookupInlandCategory(
		PeaksValleysLevel pv,
		ErosionLevel erosion,
		ContinentalnessLevel continent,
		TemperatureLevel temp);

	// Terrain Generation
	static bool m_useRaw3DNoise;
	static bool m_useHeightBias;
};