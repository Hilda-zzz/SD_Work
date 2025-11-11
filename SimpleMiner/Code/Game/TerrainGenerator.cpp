#include "TerrainGenerator.hpp"
#include "Game/Chunk.hpp"
#include "ThirdParty/Noise/SmoothNoise.hpp"
#include "ThirdParty/Noise/RawNoise.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Easing.hpp"
#include "BlockDefinition.hpp"
#include "TerrainConfig.hpp"
#include "Curve1D.hpp"
#include "BiomeRangeTables.hpp"






std::map<BiomeType, BiomeTreeConfig> TerrainGenerator::s_biomeTreeConfigs;
bool TerrainGenerator::s_biomeTreeConfigsInitialized = false;

std::map<TreeType, std::map<TreeSize, TreeStamp>> TerrainGenerator::s_treeStamps;
bool TerrainGenerator::s_treeStampsInitialized = false;

void TerrainGenerator::GenerateBlocksForChunk(Chunk* chunk)
{
	if (!chunk) return;

	// Establish world-space position and bounds of this chunk
	Vec3 minBound = Vec3((float)chunk->m_chunkCoords.x * (float)CHUNK_SIZE_X, (float)chunk->m_chunkCoords.y * (float)CHUNK_SIZE_Y, 0.f);
	Vec3 maxBound = minBound + Vec3((float)CHUNK_SIZE_X, (float)CHUNK_SIZE_Y, (float)CHUNK_SIZE_Z);
	chunk->m_worldBounds = AABB3(minBound, maxBound);

	// Derive deterministic seeds for each noise channel
	unsigned int terrainSeed = GAME_SEED;
	unsigned int humiditySeed = GAME_SEED + 1;
	unsigned int temperatureSeed = humiditySeed + 1;
	unsigned int hillSeed = temperatureSeed + 1;
	unsigned int oceanSeed = hillSeed + 1;
	unsigned int dirtSeed = oceanSeed + 1;

	// Allocate per-(x,y) maps
	int heightMapXY[MAP_SIZE];
	int dirtDepthXY[MAP_SIZE];
	float humidityMapXY[MAP_SIZE];
	float temperatureMapXY[MAP_SIZE];

	// --- Pass 1: compute surface & biome fields per (x,y) pillar ---
	for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
		for (int x = 0; x < CHUNK_SIZE_X; ++x) {


			IntVec2 globalCoords=chunk->GetGlobalCoords(x, y);
			int globalX = globalCoords.x;
			int globalY = globalCoords.y;

			// Humidity calculation
			float humidity = 0.5f + 0.5f * Compute2dPerlinNoise(
				static_cast<float>(globalX), static_cast<float>(globalY),
				HUMIDITY_NOISE_SCALE,
				HUMIDITY_NOISE_OCTAVES,
				DEFAULT_OCTAVE_PERSISTANCE,
				DEFAULT_NOISE_OCTAVE_SCALE,
				true, // wrap = TRUE
				humiditySeed
			);

			// Temperature calculation
			float temperature = Get2dNoiseNegOneToOne(
				globalX,
				globalY,
				temperatureSeed
			) * TEMPERATURE_RAW_NOISE_SCALE;

			temperature = temperature + 0.5f + 0.5f * Compute2dPerlinNoise(
				static_cast<float>(globalX), static_cast<float>(globalY),
				TEMPERATURE_NOISE_SCALE,
				TEMPERATURE_NOISE_OCTAVES,
				DEFAULT_OCTAVE_PERSISTANCE,
				DEFAULT_NOISE_OCTAVE_SCALE,
				true, // wrap = TRUE
				temperatureSeed
			);

			// Hill calculation
			float rawHill = Compute2dPerlinNoise(
				static_cast<float>(globalX), static_cast<float>(globalY),
				HILLINESS_NOISE_SCALE,
				HILLINESS_NOISE_OCTAVES,
				DEFAULT_OCTAVE_PERSISTANCE,
				DEFAULT_NOISE_OCTAVE_SCALE,
				true, // wrap = TRUE
				hillSeed
			);
			float hill = SmoothStep3(RangeMap(rawHill, -1.0f, 1.0f, 0.0f, 1.0f));

			// Ocean calculation
			float ocean = Compute2dPerlinNoise(
				static_cast<float>(globalX), static_cast<float>(globalY),
				OCEANESS_NOISE_SCALE,
				OCEANESS_NOISE_OCTAVES,
				DEFAULT_OCTAVE_PERSISTANCE,
				DEFAULT_NOISE_OCTAVE_SCALE,
				true, // wrap = TRUE
				oceanSeed
			);

			// Raw terrain calculation
			float rawTerrain = Compute2dPerlinNoise(
				static_cast<float>(globalX), static_cast<float>(globalY),
				TERRAIN_NOISE_SCALE,
				TERRAIN_NOISE_OCTAVES,
				DEFAULT_OCTAVE_PERSISTANCE,
				DEFAULT_NOISE_OCTAVE_SCALE,
				true, // wrap = TRUE
				terrainSeed
			);

			// Base terrain height with river/hill shaping
			float terrainHeightF = DEFAULT_TERRAIN_HEIGHT +
				hill * RangeMap(fabsf(rawTerrain), 0.0f, 1.0f, -RIVER_DEPTH, DEFAULT_TERRAIN_HEIGHT);

			// Ocean depressions
			if (ocean > OCEAN_START_THRESHOLD) {
				float oceanBlend = RangeMapClamped(
					ocean,
					OCEAN_START_THRESHOLD,
					OCEAN_END_THRESHOLD,
					0.0f, 1.0f
				);
				terrainHeightF = terrainHeightF - Interpolate(0.0f, OCEAN_DEPTH, oceanBlend);
			}

			// Dirt layer thickness driven by noise
			float dirtDepthPct = Get2dNoiseZeroToOne(
				globalX,
				globalY,
				dirtSeed
			);
			int dirtDepth = MIN_DIRT_OFFSET_Z +
				static_cast<int>(roundf(dirtDepthPct * (MAX_DIRT_OFFSET_Z - MIN_DIRT_OFFSET_Z)));

			// Store results in maps
			int idxXY = y * CHUNK_SIZE_X + x;
			humidityMapXY[idxXY] = humidity;
			temperatureMapXY[idxXY] = temperature;
			heightMapXY[idxXY] = static_cast<int>(floorf(terrainHeightF));
			dirtDepthXY[idxXY] = dirtDepth;
		}
	}

	// Pass 2: assign block types for every (x,y,z)
	for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
		for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
			for (int x = 0; x < CHUNK_SIZE_X; ++x) {
				IntVec3 local(x, y, z);
				IntVec3 global = chunk->LocalToGlobalCoords(local);
				int idx = chunk->LocalCoordsToIndex(local);
				int idxXY = y * CHUNK_SIZE_X + x;

				int terrainHeight = heightMapXY[idxXY];
				int dirtDepth = dirtDepthXY[idxXY];
				float humidity = humidityMapXY[idxXY];
				float temperature = temperatureMapXY[idxXY];

				// Temperature-driven ice ceiling depth
				int iceDepth = static_cast<int>(DEFAULT_TERRAIN_HEIGHT - 
					floorf(
					RangeMapClamped(temperature,
						ICE_TEMPERATURE_MAX,
						ICE_TEMPERATURE_MIN,
						ICE_DEPTH_MIN,
						ICE_DEPTH_MAX)
				));

				Block block = BlockDefinition::s_nameToIndexMap["Air"]; // Default to air

				// Water and ice between surface and sea level
				if (global.z > terrainHeight && global.z < SEA_LEVEL_Z) {
					block = BlockDefinition::s_nameToIndexMap["Water"];
					if (temperature < 0.38f && global.z > iceDepth) {
						block = BlockDefinition::s_nameToIndexMap["Ice"];
					}
					chunk->SetBlock(idx, block);
				}

				// Surface block (grass vs sand by humidity and elevation)
				else if (global.z == terrainHeight) {
					block = BlockDefinition::s_nameToIndexMap["Grass"];
					if (humidity < MIN_SAND_HUMIDITY) {
						block = BlockDefinition::s_nameToIndexMap["Sand"];
					}
					if (humidity < MAX_SAND_HUMIDITY && terrainHeight <= DEFAULT_TERRAIN_HEIGHT) {
						block = BlockDefinition::s_nameToIndexMap["Sand"];
					}
					chunk->SetBlock(idx, block);
				}

				// Subsurface: dirt or sand cap above stone/ores
				else if (global.z < terrainHeight) {
					int dirtTopZ = terrainHeight - dirtDepth;
					int sandTopZ = terrainHeight - static_cast<int>(floorf(
						RangeMapClamped(humidity,
							MIN_SAND_DEPTH_HUMIDITY,
							MAX_SAND_DEPTH_HUMIDITY,
							SAND_DEPTH_MIN,
							SAND_DEPTH_MAX)
					));

					if (global.z >= dirtTopZ) {
						block = BlockDefinition::s_nameToIndexMap["Dirt"];
						if (global.z >= sandTopZ) {
							block = BlockDefinition::s_nameToIndexMap["Sand"];
						}
						chunk->SetBlock(idx, block);
					}

					// Deep underground: special layers, lava/obsidian, ores, stone
					else {
						if (global.z == OBSIDIAN_Z) {
							block = BlockDefinition::s_nameToIndexMap["Obsidian"];
						}
						else if (global.z == LAVA_Z) {
							block = BlockDefinition::s_nameToIndexMap["Lava"];
						}
						else {
							float oreNoise = Get3dNoiseZeroToOne(
								global.x,
								global.y,
								global.z
							);

							if (oreNoise < DIAMOND_CHANCE) {
								block = BlockDefinition::s_nameToIndexMap["Diamond"];
							}
							else if (oreNoise < GOLD_CHANCE) {
								block = BlockDefinition::s_nameToIndexMap["Gold"];
							}
							else if (oreNoise < IRON_CHANCE) {
								block = BlockDefinition::s_nameToIndexMap["Iron"];
							}
							else if (oreNoise < COAL_CHANCE) {
								block = BlockDefinition::s_nameToIndexMap["Coal"];
							}
							else {
								block = BlockDefinition::s_nameToIndexMap["Stone"];
							}
						}
						chunk->SetBlock(idx, block);
					}
				}
			}
		}
	}

}

void TerrainGenerator::GenerateBlocksForChunk_New(Chunk* chunk)
{
	if (!chunk) return;

	// ===  Establish world-space position and bounds of this chunk
	Vec3 minBound = Vec3((float)chunk->m_chunkCoords.x * (float)CHUNK_SIZE_X, (float)chunk->m_chunkCoords.y * (float)CHUNK_SIZE_Y, 0.f);
	Vec3 maxBound = minBound + Vec3((float)CHUNK_SIZE_X, (float)CHUNK_SIZE_Y, (float)CHUNK_SIZE_Z);
	chunk->m_worldBounds = AABB3(minBound, maxBound);

	// =========== 2d noise ====================
	Generate2DNoise(chunk);


	// ============ density func ================
	// ============ Step 1: Calculate densities and find surface heights ================
	int extendedWidth = NOISE_EXTENDED_SIZE;  // 24
	int extendedHeight = NOISE_EXTENDED_SIZE; // 24
	int extendedBlocksPerChunk = extendedWidth * extendedHeight * CHUNK_SIZE_Z;
	std::vector<float> extendedDensities(extendedBlocksPerChunk, 0.0f);

	// 扩展的表面高度 (24×24)
	std::vector<int> extendedSurfaceHeights(NOISE_MAP_SIZE, -1);

	for (int noiseExtY = 0; noiseExtY < extendedHeight; ++noiseExtY)
	{
		for (int noiseExtX = 0; noiseExtX < extendedWidth; ++noiseExtX)
		{
			// 转换到本地坐标（可能为负或超出chunk）
			int localX = noiseExtX - MAX_TREE_RADIUS;
			int localY = noiseExtY - MAX_TREE_RADIUS;

			// 计算全局坐标
			IntVec3 globalPos = chunk->LocalToGlobalCoords(IntVec3(localX, localY, 0));
			int globalX = globalPos.x;
			int globalY = globalPos.y;

			// 获取噪声索引
			int noiseIdx = chunk->NoiseExtendedCoordsToIndex(noiseExtX, noiseExtY);

			// 获取2D噪声值
			float continentOffset = chunk->m_continentOffsetMapped[noiseIdx];
			float continentAmplitude = chunk->m_continentAmplitudeMapped[noiseIdx];
			float erosionOffset = chunk->m_erosionOffsetMapped[noiseIdx];
			float erosionAmplitude = chunk->m_erosionAmplitudeMapped[noiseIdx];
			float pvOffset = chunk->m_pvOffsetMapped[noiseIdx];
			float pvNoise = chunk->m_pvNoiseRaw[noiseIdx];

			int surfaceZ = -1;

			// 遍历Z轴
			for (int z = 0; z < CHUNK_SIZE_Z; ++z)
			{
				// 计算扩展数组的索引
				int extendedIdx = z * (extendedWidth * extendedHeight) + noiseExtY * extendedWidth + noiseExtX;

				float rawDensity = Compute3dPerlinNoise(
					(float)globalX, (float)globalY, (float)z,
					64, 8
				);

				// === Base Height ===
				float baseHeight = DEFAULT_TERRAIN_HEIGHT + (continentOffset * (CHUNK_SIZE_Z / 2.0f));

				// === Height Offset ===
				float heightOffset = ((float)z - baseHeight) / baseHeight;

				// === Calculate Density ===
				//float density = rawDensity + (-10.f / CHUNK_SIZE_Z) * ((float)z - baseHeight);
				float density = rawDensity;
				density += continentOffset;
				density += continentAmplitude * heightOffset;
				density -= erosionOffset;
				density -= erosionAmplitude * heightOffset;
				density += pvOffset * pvNoise * (1.f / CHUNK_SIZE_Z);

				// Store density in extended array
				extendedDensities[extendedIdx] = density;

				// Update surface height for this column
				if (density >= 0.f) // Solid block
				{
					surfaceZ = z;
				}
			}

			extendedSurfaceHeights[noiseIdx] = surfaceZ;
		}
	}


	ReplaceSurface(chunk, extendedDensities, extendedSurfaceHeights);
	GenerateTrees(chunk, extendedSurfaceHeights);
}

void TerrainGenerator::Generate2DNoise(Chunk* chunk)
{
	TerrainConfig& config = TerrainConfig::GetInstance();
	unsigned int continentSeed = config.m_seeds.m_continentSeed;
	unsigned int erosionSeed = config.m_seeds.m_erosionSeed;
	unsigned int pvSeed = config.m_seeds.m_weirdnessSeed;
	unsigned int temperatureSeed = config.m_seeds.m_temperatureSeed;
	unsigned int humiditySeed = config.m_seeds.m_humiditySeed;
	unsigned int treeSeed = config.m_seeds.m_treeSeed;

	for (int noiseExtY = 0; noiseExtY < NOISE_EXTENDED_SIZE; ++noiseExtY)
	{
		for (int noiseExtX = 0; noiseExtX < NOISE_EXTENDED_SIZE; ++noiseExtX)
		{
			// 转换到本地坐标
			int localX = noiseExtX - MAX_TREE_RADIUS;
			int localY = noiseExtY - MAX_TREE_RADIUS;

			// 计算全局坐标
			int globalX = chunk->m_chunkCoords.x * CHUNK_SIZE_X + localX;
			int globalY = chunk->m_chunkCoords.y * CHUNK_SIZE_Y + localY;

			int noiseIdx = chunk->NoiseExtendedCoordsToIndex(noiseExtX, noiseExtY);

			ContinentalnessLevel continentLevel = ContinentalnessLevel::NEAR_INLAND;
			ErosionLevel erosionLevel = ErosionLevel::E3;
			PeaksValleysLevel pvLevel = PeaksValleysLevel::MID;
			TemperatureLevel temperatureLevel = TemperatureLevel::T2;
			HumidityLevel humidityLevel = HumidityLevel::H2;

			// === 1. Continent Noise ===
			if (config.m_continent.m_enabled)
			{
				float continentNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_continent.m_scale,
					config.m_continent.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					continentSeed
				);

				chunk->m_continentNoiseRaw[noiseIdx] = continentNoise;
				chunk->m_continentOffsetMapped[noiseIdx] = config.m_curves.m_continentOffset->Evaluate(continentNoise);
				chunk->m_continentAmplitudeMapped[noiseIdx] = config.m_curves.m_continentAmplitude->Evaluate(continentNoise);
				continentLevel = BiomeRangeTables::GetContinentalnessLevel(continentNoise);
				chunk->m_continentLevel[noiseIdx] = continentLevel;
			}
			else
			{
				chunk->m_continentNoiseRaw[noiseIdx] = 0.0f;
				chunk->m_continentOffsetMapped[noiseIdx] = 0.0f;
				chunk->m_continentAmplitudeMapped[noiseIdx] = 1.0f;
				chunk->m_continentLevel[noiseIdx] = ContinentalnessLevel::NEAR_INLAND;
			}

			// === 2. Erosion Noise ===
			if (config.m_erosion.m_enabled)
			{
				float erosionNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_erosion.m_scale,
					config.m_erosion.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					erosionSeed
				);

				chunk->m_erosionNoiseRaw[noiseIdx] = erosionNoise;
				chunk->m_erosionOffsetMapped[noiseIdx] = config.m_curves.m_erosionOffset->Evaluate(erosionNoise);
				chunk->m_erosionAmplitudeMapped[noiseIdx] = config.m_curves.m_erosionAmplitude->Evaluate(erosionNoise);
				erosionLevel = BiomeRangeTables::GetErosionLevel(erosionNoise);
				chunk->m_erosionLevel[noiseIdx] = erosionLevel;
			}
			else
			{
				chunk->m_erosionNoiseRaw[noiseIdx] = 0.0f;
				chunk->m_erosionOffsetMapped[noiseIdx] = 0.0f;
				chunk->m_erosionAmplitudeMapped[noiseIdx] = 1.0f;
				chunk->m_erosionLevel[noiseIdx] = ErosionLevel::E3;
			}

			// === 3. Peaks & Valleys Noise ===
			if (config.m_peaksValleys.m_enabled)
			{
				float pvNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_peaksValleys.m_scale,
					config.m_peaksValleys.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					pvSeed
				);

				chunk->m_pvNoiseRaw[noiseIdx] = pvNoise;
				chunk->m_pvOffsetMapped[noiseIdx] = 1.f - abs(3.f * abs(pvNoise) - 2.f);
				pvLevel = BiomeRangeTables::GetPeaksValleysLevel(chunk->m_pvOffsetMapped[noiseIdx]);
				chunk->m_pvLevel[noiseIdx] = pvLevel;
			}
			else
			{
				chunk->m_pvNoiseRaw[noiseIdx] = 0.0f;
				chunk->m_pvOffsetMapped[noiseIdx] = 0.0f;
				chunk->m_pvLevel[noiseIdx] = PeaksValleysLevel::MID;
			}

			// === 4. Temperature Noise ===
			if (config.m_temperature.m_enabled)
			{
				float tempNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_temperature.m_scale,
					config.m_temperature.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					temperatureSeed
				);

				chunk->m_temperatureNoiseRaw[noiseIdx] = tempNoise;
				temperatureLevel = BiomeRangeTables::GetTemperatureLevel(tempNoise);
				chunk->m_temperatureLevel[noiseIdx] = temperatureLevel;
			}
			else
			{
				chunk->m_temperatureNoiseRaw[noiseIdx] = 0.0f;
				chunk->m_temperatureLevel[noiseIdx] = TemperatureLevel::T2;
			}

			// === 5. Humidity Noise ===
			if (config.m_humidity.m_enabled)
			{
				float humidNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_humidity.m_scale,
					config.m_humidity.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					humiditySeed
				);

				chunk->m_humidityNoiseRaw[noiseIdx] = humidNoise;
				humidityLevel = BiomeRangeTables::GetHumidityLevel(humidNoise);
				chunk->m_humidityLevel[noiseIdx] = humidityLevel;
			}
			else
			{
				chunk->m_humidityNoiseRaw[noiseIdx] = 0.0f;
				chunk->m_humidityLevel[noiseIdx] = HumidityLevel::H2;
			}

			// Lookup biome
			chunk->m_biomeType[noiseIdx] = DetermineBiome(
				continentLevel, erosionLevel, pvLevel,
				temperatureLevel, humidityLevel
			);
		}
	}

	if (config.m_tree.m_enabled)
	{
		for (int treeExtY = 0; treeExtY < TREE_EXTENDED_SIZE; ++treeExtY)
		{
			for (int treeExtX = 0; treeExtX < TREE_EXTENDED_SIZE; ++treeExtX)
			{
				// 转换到本地坐标 (注意额外的padding)
				int localX = treeExtX - MAX_TREE_RADIUS -TREE_CHECK_PADDING;
				int localY = treeExtY - MAX_TREE_RADIUS - TREE_CHECK_PADDING;

				// 计算全局坐标
				int globalX = chunk->m_chunkCoords.x * CHUNK_SIZE_X + localX;
				int globalY = chunk->m_chunkCoords.y * CHUNK_SIZE_Y + localY;

				// 生成树噪声
				float treeNoise = Compute2dPerlinNoise(
					static_cast<float>(globalX),
					static_cast<float>(globalY),
					config.m_tree.m_scale,
					config.m_tree.m_octaves,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					treeSeed
				);

				int treeIdx = chunk->TreeExtendedCoordsToIndex(treeExtX, treeExtY);
				chunk->m_treeNoiseRaw[treeIdx] = treeNoise;
			}
		}
	}
	else
	{
		for (int i = 0; i < TREE_MAP_SIZE; ++i)
		{
			chunk->m_treeNoiseRaw[i] = 0.0f;
		}
	}

}

void TerrainGenerator::ReplaceSurface(
	Chunk* chunk,
	std::vector<float> const& extendedDensities,      // 24×24×128
	std::vector<int> const& extendedSurfaceHeights)   // 24×24
{
	int extendedWidth = NOISE_EXTENDED_SIZE;   // 24
	int extendedHeight = NOISE_EXTENDED_SIZE;  // 24

	// 只遍历chunk内部的blocks (16×16×128)
	for (int z = 0; z < CHUNK_SIZE_Z; ++z)
	{
		for (int y = 0; y < CHUNK_SIZE_Y; ++y)
		{
			for (int x = 0; x < CHUNK_SIZE_X; ++x)
			{
				IntVec3 local(x, y, z);
				IntVec3 global = chunk->LocalToGlobalCoords(local);

				// Chunk内部的索引
				int chunkIdx = chunk->LocalCoordsToIndex(local);

				// 转换到扩展坐标
				int noiseExtX = x + MAX_TREE_RADIUS;
				int noiseExtY = y + MAX_TREE_RADIUS;

				// 扩展数组的2D索引（用于biome和surfaceHeight）
				int noiseIdx = chunk->NoiseExtendedCoordsToIndex(noiseExtX, noiseExtY);

				// 扩展密度数组的3D索引
				int extendedDensityIdx = z * (extendedWidth * extendedHeight) +
					noiseExtY * extendedWidth + noiseExtX;

				float density = extendedDensities[extendedDensityIdx];
				BiomeType biome = chunk->m_biomeType[noiseIdx];
				Block block = BlockDefinition::s_nameToIndexMap["Air"];

				// Non-solid (Air or Water)
				if (density < 0.f)
				{
					if (global.z > SEA_LEVEL_Z)
					{
						block = BlockDefinition::s_nameToIndexMap["Air"];
					}
					else
					{
						// Water or Ice based on biome
						if ((biome == BiomeType::FROZEN_OCEAN || biome == BiomeType::SNOWY_BEACH ||
							biome == BiomeType::SNOWY_PLAINS || biome == BiomeType::SNOWY_TAIGA) &&
							global.z == SEA_LEVEL_Z)
						{
							block = BlockDefinition::s_nameToIndexMap["Ice"];
						}
						else
						{
							block = BlockDefinition::s_nameToIndexMap["Water"];
						}
					}
					chunk->SetBlock(chunkIdx, block);
				}
				// Solid blocks
				else
				{
					// === Check if this is surface block (O(1) lookup!) ===
					bool isSurface = (z == extendedSurfaceHeights[noiseIdx]);

					// === Bedrock layers ===
					if (global.z == LAVA_Z)
					{
						block = BlockDefinition::s_nameToIndexMap["Lava"];
						chunk->SetBlock(chunkIdx, block);
					}
					else if (global.z == OBSIDIAN_Z)
					{
						block = BlockDefinition::s_nameToIndexMap["Obsidian"];
						chunk->SetBlock(chunkIdx, block);
					}
					// === Surface blocks ===
					else if (isSurface)
					{
						switch (biome)
						{
						case BiomeType::OCEAN:
							block = (global.z >= SEA_LEVEL_Z) ?
								BlockDefinition::s_nameToIndexMap["Sand"] :
								BlockDefinition::s_nameToIndexMap["Dirt"];
							break;

						case BiomeType::DEEP_OCEAN:
						case BiomeType::FROZEN_OCEAN:
							block = (global.z >= SEA_LEVEL_Z) ?
								((biome == BiomeType::FROZEN_OCEAN) ?
									BlockDefinition::s_nameToIndexMap["Snow"] :
									BlockDefinition::s_nameToIndexMap["Sand"]) :
								BlockDefinition::s_nameToIndexMap["Stone"];
							break;

						case BiomeType::BEACH:
							block = BlockDefinition::s_nameToIndexMap["Sand"];
							break;

						case BiomeType::SNOWY_BEACH:
						case BiomeType::SNOWY_PLAINS:
						case BiomeType::SNOWY_TAIGA:
						case BiomeType::SNOWY_PEAKS:
							block = BlockDefinition::s_nameToIndexMap["Snow"];
							break;

						case BiomeType::DESERT:
						case BiomeType::BADLANDS:
							block = BlockDefinition::s_nameToIndexMap["Sand"];
							break;

						case BiomeType::STONY_PEAKS:
							block = BlockDefinition::s_nameToIndexMap["Stone"];
							break;

						case BiomeType::PLAINS:
							block = BlockDefinition::s_nameToIndexMap["GrassLight"];
							break;
						case BiomeType::FOREST:
							block = BlockDefinition::s_nameToIndexMap["Grass"];
							break;
						case BiomeType::JUNGLE:
							block = BlockDefinition::s_nameToIndexMap["GrassDark"];
							break;
						case BiomeType::TAIGA:
							block = BlockDefinition::s_nameToIndexMap["GrassLight"];
							break;
						case BiomeType::SAVANNA:
							block = BlockDefinition::s_nameToIndexMap["GrassYellow"];
							break;
						default:
							block = BlockDefinition::s_nameToIndexMap["Grass"];
							break;
						}
						chunk->SetBlock(chunkIdx, block);
					}
					// === Subsurface blocks ===
					else
					{
						// Calculate depth (O(1) calculation!)
						int depthBelowSurface = extendedSurfaceHeights[noiseIdx] - z;

						// Determine subsurface layers based on biome
						bool hasDirtLayers = true;
						int dirtDepth = MIN_DIRT_OFFSET_Z;
						if (Get2dNoiseZeroToOne(global.x, global.y, GAME_SEED + 6) > 0.5f)
							dirtDepth = 4;
						int sandDepth = 0;

						// Biomes with no dirt layers
						if (biome == BiomeType::DEEP_OCEAN || biome == BiomeType::FROZEN_OCEAN ||
							biome == BiomeType::STONY_PEAKS || biome == BiomeType::SNOWY_PEAKS)
						{
							hasDirtLayers = false;
						}

						// Desert has extra sand layers
						if (biome == BiomeType::DESERT)
						{
							sandDepth = 4;
						}

						// Apply subsurface blocks
						if (biome == BiomeType::DESERT && depthBelowSurface < sandDepth)
						{
							block = BlockDefinition::s_nameToIndexMap["Sand"];
							chunk->SetBlock(chunkIdx, block);
						}
						else if (hasDirtLayers && depthBelowSurface < dirtDepth + sandDepth)
						{
							block = BlockDefinition::s_nameToIndexMap["Dirt"];
							chunk->SetBlock(chunkIdx, block);
						}
						else
						{
							// Stone with ores
							float oreNoise = Get3dNoiseZeroToOne(global.x, global.y, global.z);

							if (oreNoise < DIAMOND_CHANCE)
							{
								block = BlockDefinition::s_nameToIndexMap["Diamond"];
							}
							else if (oreNoise < GOLD_CHANCE)
							{
								block = BlockDefinition::s_nameToIndexMap["Gold"];
							}
							else if (oreNoise < IRON_CHANCE)
							{
								block = BlockDefinition::s_nameToIndexMap["Iron"];
							}
							else if (oreNoise < COAL_CHANCE)
							{
								block = BlockDefinition::s_nameToIndexMap["Coal"];
							}
							else
							{
								block = BlockDefinition::s_nameToIndexMap["Stone"];
							}
							chunk->SetBlock(chunkIdx, block);
						}
					}
				}
			}
		}
	}
}

void TerrainGenerator::GenerateTrees(Chunk* chunk, const std::vector<int>& extendedSurfaceHeights)
{
	if (!chunk) return;

	if (!s_biomeTreeConfigsInitialized)
	{
		InitializeBiomeTreeConfigs();
	}
	if (!s_treeStampsInitialized)
	{
		InitializeTreeStamps();
	}

	std::vector<TreeCandidate> treeCandidates;

	// 遍历树噪声的有效区域 (26×26，去掉最外层1格padding)
	// [1, 25) 对应实际可以放树的区域
	for (int treeExtY = 1; treeExtY < TREE_EXTENDED_SIZE - 1; ++treeExtY)
	{
		for (int treeExtX = 1; treeExtX < TREE_EXTENDED_SIZE - 1; ++treeExtX)
		{
			// 检查是否是局部最大值 (使用树噪声坐标)
			if (!IsLocalMaximum3x3_Tree(chunk, treeExtX, treeExtY))
			{
				continue;
			}

			// 转换到一般噪声坐标 (24×24)
			IntVec2 noiseCoords = chunk->TreeCoordsToNoiseCoords(treeExtX, treeExtY);
			int noiseExtX = noiseCoords.x;
			int noiseExtY = noiseCoords.y;

			// 边界检查
			if (noiseExtX < 0 || noiseExtX >= NOISE_EXTENDED_SIZE ||
				noiseExtY < 0 || noiseExtY >= NOISE_EXTENDED_SIZE)
			{
				continue;
			}

			// 获取噪声索引
			int noiseIdx = chunk->NoiseExtendedCoordsToIndex(noiseExtX, noiseExtY);

			// 获取表面高度
			int surfaceZ = extendedSurfaceHeights[noiseIdx];

			// 检查表面高度
			if (surfaceZ < SEA_LEVEL_Z)
			{
				continue;
			}

			// 获取生物群系和环境数据
			BiomeType biome = chunk->m_biomeType[noiseIdx];
			float temperature = chunk->m_temperatureNoiseRaw[noiseIdx];
			float humidity = chunk->m_humidityNoiseRaw[noiseIdx];

			// 获取树噪声
			int treeIdx = chunk->TreeExtendedCoordsToIndex(treeExtX, treeExtY);
			float treeNoise = chunk->m_treeNoiseRaw[treeIdx];

			// 判断树的大小
			TreeSize treeSize = DetermineTreeSize(treeNoise, biome, temperature, humidity);
			if (treeSize == TreeSize::NONE)
			{
				continue;
			}

			// 获取该生物群系的树配置
			BiomeTreeConfig config = GetBiomeTreeConfig(biome);
			if (config.m_availableTreeTypes.empty())
			{
				continue;
			}

			// 转换到本地坐标用于放置
			int localX = noiseExtX - MAX_TREE_RADIUS;
			int localY = noiseExtY - MAX_TREE_RADIUS;

			// 随机选择树类型
			IntVec3 globalPos = chunk->LocalToGlobalCoords(IntVec3(localX, localY, 0));
			int treeTypeIndex = GetDeterministicTreeType(
				globalPos.x,
				globalPos.y,
				(int)config.m_availableTreeTypes.size()
			);
			TreeType treeType = config.m_availableTreeTypes[treeTypeIndex];

			// 创建树候选 (使用本地坐标)
			TreeCandidate candidate(IntVec2(localX, localY), treeSize, treeType, surfaceZ);
			treeCandidates.push_back(candidate);
		}
	}

	// 放置所有树木
	for (const TreeCandidate& candidate : treeCandidates)
	{
		// 获取对应的树木印章
		TreeStamp stamp = GetTreeStamp(candidate.m_type, candidate.m_size);

		// 检查是否可以放置树木
		if (!CanPlaceTree(chunk, candidate, stamp))
		{
			continue;
		}

		// 放置树木
		PlaceTree(chunk, candidate, stamp);
	}
}

bool TerrainGenerator::IsLocalMaximum3x3_Tree(Chunk* chunk, int treeExtX, int treeExtY)
{
	// 边界检查 - 确保可以安全检查3x3
	if (treeExtX < 1 || treeExtX >= TREE_EXTENDED_SIZE - 1 ||
		treeExtY < 1 || treeExtY >= TREE_EXTENDED_SIZE - 1)
	{
		return false;
	}

	int currentIdx = chunk->TreeExtendedCoordsToIndex(treeExtX, treeExtY);
	float currentNoise = chunk->m_treeNoiseRaw[currentIdx];

	// 检查3x3邻域
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0) continue;

			int neighborIdx = chunk->TreeExtendedCoordsToIndex(treeExtX + dx, treeExtY + dy);
			float neighborNoise = chunk->m_treeNoiseRaw[neighborIdx];

			if (neighborNoise >= currentNoise)
				return false;
		}
	}

	return true;
}

void TerrainGenerator::InitializeBiomeTreeConfigs()
{
	if (s_biomeTreeConfigsInitialized)
		return;

	// 海洋类 - 不生成树
	BiomeTreeConfig oceanConfig;
	oceanConfig.m_availableTreeTypes = {};
	oceanConfig.m_threshold_Small = 999.f;
	s_biomeTreeConfigs[BiomeType::OCEAN] = oceanConfig;
	s_biomeTreeConfigs[BiomeType::DEEP_OCEAN] = oceanConfig;
	s_biomeTreeConfigs[BiomeType::FROZEN_OCEAN] = oceanConfig;

	// 沙滩类 - 少量树
	BiomeTreeConfig beachConfig;
	beachConfig.m_availableTreeTypes = {};
	beachConfig.m_threshold_Small = 0.3f;
	beachConfig.m_threshold_Small = 0.5f;
	beachConfig.m_threshold_Small = 0.7f;
	s_biomeTreeConfigs[BiomeType::BEACH] = beachConfig;

	BiomeTreeConfig snowyBeachConfig;
	snowyBeachConfig.m_availableTreeTypes = { TreeType::CACTUS };
	snowyBeachConfig.m_threshold_Small = 0.3f;
	s_biomeTreeConfigs[BiomeType::SNOWY_BEACH] = snowyBeachConfig;

	// 沙漠 - 仙人掌
	BiomeTreeConfig desertConfig;
	desertConfig.m_availableTreeTypes = { TreeType::CACTUS };
	desertConfig.m_threshold_Small = 0.3f;
	desertConfig.m_threshold_Medium = 0.45f;
	desertConfig.m_threshold_Large = 0.6f;
	desertConfig.m_humidityInfluence = -0.2f;
	s_biomeTreeConfigs[BiomeType::DESERT] = desertConfig;

	// 热带草原 - 金合欢和仙人掌
	BiomeTreeConfig savannaConfig;
	savannaConfig.m_availableTreeTypes = { TreeType::ACACIA, TreeType::CACTUS };
	savannaConfig.m_threshold_Small = 0.0f;
	savannaConfig.m_threshold_Medium = 0.3f;
	savannaConfig.m_threshold_Large = 0.5f;
	savannaConfig.m_humidityInfluence = 0.1f;
	s_biomeTreeConfigs[BiomeType::SAVANNA] = savannaConfig;

	// 平原 - 少量橡树
	BiomeTreeConfig plainsConfig;
	plainsConfig.m_availableTreeTypes = { TreeType::OAK };
	plainsConfig.m_threshold_Small = 0.0f;
	plainsConfig.m_threshold_Medium = 0.2f;
	plainsConfig.m_threshold_Large = 0.4f;
	s_biomeTreeConfigs[BiomeType::PLAINS] = plainsConfig;

	BiomeTreeConfig snowyPlainsConfig;
	snowyPlainsConfig.m_availableTreeTypes = {};
	snowyPlainsConfig.m_threshold_Small = 0.3f;
	s_biomeTreeConfigs[BiomeType::SNOWY_PLAINS] = snowyPlainsConfig;

	// 森林 - 大量橡树和白桦树
	BiomeTreeConfig forestConfig;
	forestConfig.m_availableTreeTypes = { TreeType::OAK, TreeType::BIRCH };
	forestConfig.m_threshold_Small = -0.6f;
	forestConfig.m_threshold_Medium = -0.4f;
	forestConfig.m_threshold_Large = 0.1f;
	forestConfig.m_tempInfluence = 0.05f;
	forestConfig.m_humidityInfluence = 0.1f;
	s_biomeTreeConfigs[BiomeType::FOREST] = forestConfig;

	// 丛林 - 大量丛林树
	BiomeTreeConfig jungleConfig;
	jungleConfig.m_availableTreeTypes = { TreeType::JUNGLE };
	jungleConfig.m_threshold_Small = -0.8f;
	jungleConfig.m_threshold_Medium = -0.6f;
	jungleConfig.m_threshold_Large = 0.0f;
	jungleConfig.m_tempInfluence = 0.1f;
	jungleConfig.m_humidityInfluence = 0.15f;
	s_biomeTreeConfigs[BiomeType::JUNGLE] = jungleConfig;

	// 针叶林 - 云杉
	BiomeTreeConfig taigaConfig;
	taigaConfig.m_availableTreeTypes = { TreeType::SPRUCE };
	taigaConfig.m_threshold_Small = 0.0f;
	taigaConfig.m_threshold_Medium = 0.2f;
	taigaConfig.m_threshold_Large = 0.4f;
	taigaConfig.m_tempInfluence = -0.1f;
	s_biomeTreeConfigs[BiomeType::TAIGA] = taigaConfig;

	BiomeTreeConfig snowyTaigaConfig;
	snowyTaigaConfig.m_availableTreeTypes = { TreeType::SPRUCE_SNOWY };
	snowyTaigaConfig.m_threshold_Small = 0.2f;
	snowyTaigaConfig.m_threshold_Medium = 0.3f;
	snowyTaigaConfig.m_threshold_Large = 0.4f;
	snowyTaigaConfig.m_tempInfluence = -0.15f;
	s_biomeTreeConfigs[BiomeType::SNOWY_TAIGA] = snowyTaigaConfig;

	// 山峰类
	BiomeTreeConfig peaksConfig;
	peaksConfig.m_availableTreeTypes = {};
	peaksConfig.m_threshold_Small = 0.2f;
	peaksConfig.m_threshold_Medium = 0.4f;
	peaksConfig.m_threshold_Medium = 0.7f;
	s_biomeTreeConfigs[BiomeType::STONY_PEAKS] = peaksConfig;
	s_biomeTreeConfigs[BiomeType::SNOWY_PEAKS] = peaksConfig;

	// 荒地 - 不生成树
	BiomeTreeConfig badlandsConfig;
	badlandsConfig.m_availableTreeTypes = {};
	badlandsConfig.m_threshold_Small = 999.f;
	s_biomeTreeConfigs[BiomeType::BADLANDS] = badlandsConfig;

	s_biomeTreeConfigsInitialized = true;
}

BiomeTreeConfig TerrainGenerator::GetBiomeTreeConfig(BiomeType biome)
{
	if (!s_biomeTreeConfigsInitialized)
	{
		InitializeBiomeTreeConfigs();
	}

	auto it = s_biomeTreeConfigs.find(biome);
	if (it != s_biomeTreeConfigs.end())
	{
		return it->second;
	}

	return BiomeTreeConfig();
}

TreeSize TerrainGenerator::DetermineTreeSize(float treeNoise, BiomeType biome, float temp, float humidity)
{
	BiomeTreeConfig config = GetBiomeTreeConfig(biome);

	// 如果该生物群系没有可用的树种，不生成
	if (config.m_availableTreeTypes.empty())
	{
		return TreeSize::NONE;
	}

	// 温湿度调整（将 0~1 归一化到 -0.5~0.5）
	float tempAdjust = (temp - 0.5f) * config.m_tempInfluence;
	float humidityAdjust = (humidity - 0.5f) * config.m_humidityInfluence;

	// 调整后的噪声值
	float adjustedNoise = treeNoise + tempAdjust + humidityAdjust;

	// 根据阈值判断尺寸
	if (adjustedNoise > config.m_threshold_Large)
	{
		return TreeSize::LARGE;
	}
	else if (adjustedNoise > config.m_threshold_Medium)
	{
		return TreeSize::MEDIUM;
	}
	else if (adjustedNoise > config.m_threshold_Small)
	{
		return TreeSize::SMALL;
	}

	return TreeSize::NONE;
}

bool TerrainGenerator::IsLocalMaximum3x3(Chunk* chunk, int localX, int localY)
{
	// ✅ 新的扩展宽度和高度
	int extendedWidth = CHUNK_SIZE_X + 2 * MAX_TREE_RADIUS + 2;
	int extendedHeight = CHUNK_SIZE_Y + 2 * MAX_TREE_RADIUS + 2;

	// ✅ 新的索引计算 - 偏移量改为 (MAX_TREE_RADIUS + 1)
	int extX = localX + MAX_TREE_RADIUS + 1;
	int extY = localY + MAX_TREE_RADIUS + 1;

	// 边界检查 - 确保可以检查周围8个邻居
	if (extX < 1 || extX >= extendedWidth - 1 ||
		extY < 1 || extY >= extendedHeight - 1)
	{
		return false; // 边界位置不生成树
	}

	// 获取当前位置的噪声值
	int currentIdx = extY * extendedWidth + extX;
	float currentNoise = chunk->m_treeNoiseRaw[currentIdx];

	// 检查 3×3 范围内的 8 个邻居
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			// 跳过自己
			if (dx == 0 && dy == 0)
				continue;

			int neighborExtX = extX + dx;
			int neighborExtY = extY + dy;
			int neighborIdx = neighborExtY * extendedWidth + neighborExtX;

			float neighborNoise = chunk->m_treeNoiseRaw[neighborIdx];

			// 如果有任何邻居的噪声值 >= 当前值,则不是局部最大
			if (neighborNoise >= currentNoise)
			{
				return false;
			}
		}
	}

	return true;
}

void TerrainGenerator::InitializeTreeStamps()
{
	if (s_treeStampsInitialized)
		return;

	InitializeOakTreeStamps();
	InitializeBirchStamps();
	InitializeSpruceStamps();
	InitializeCuctusStamps();
	InitializeJungleStamps();
	InitializeAcaciaStamps();

	s_treeStampsInitialized = true;
}

void TerrainGenerator::InitializeOakTreeStamps()
{
	const Block OAK_LOG = BlockDefinition::s_nameToIndexMap["OakLog"];
	const Block OAK_LEAVES = BlockDefinition::s_nameToIndexMap["OakLeaves"];
	// ===========================
	// Oak Tree (橡树)
	// ===========================

	//-------------------------------------------------------------------------------
	// Small Oak (4-6 blocks tall)
	TreeStamp oakSmall;
	oakSmall.m_type = TreeType::OAK;
	oakSmall.m_size = TreeSize::SMALL;
	oakSmall.m_radius = 5;

	// 树干 (4 blocks)
	for (int i = 0; i < 4; ++i)
	{
		oakSmall.m_blocks.push_back({ IntVec3(0, 0, i), OAK_LOG });
	}

	// 树叶层 - z=2 (5×5，去掉角落)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			// 去掉四个角
			if ((abs(dx) == 2 && abs(dy) == 2))
				continue;
			oakSmall.m_blocks.push_back({ IntVec3(dx, dy, 2), OAK_LEAVES });
		}
	}

	// 树叶层 - z=3 (5×5，去掉角落)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if ((abs(dx) == 2 && abs(dy) == 2))
				continue;
			oakSmall.m_blocks.push_back({ IntVec3(dx, dy, 3), OAK_LEAVES });
		}
	}

	// 树叶层 - z=4 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			oakSmall.m_blocks.push_back({ IntVec3(dx, dy, 4), OAK_LEAVES });
		}
	}

	s_treeStamps[TreeType::OAK][TreeSize::SMALL] = oakSmall;

	//-------------------------------------------------------------------------------
	// Medium Oak (5-7 blocks tall)
	TreeStamp oakMedium;
	oakMedium.m_type = TreeType::OAK;
	oakMedium.m_size = TreeSize::MEDIUM;
	oakMedium.m_radius = 7;

	// 树干 (5 blocks)
	for (int i = 0; i < 5; ++i)
	{
		oakMedium.m_blocks.push_back({ IntVec3(0, 0, i), OAK_LOG });
	}

	// 树叶层 - z=3 (5×5)
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if ((abs(dx) == 3 && abs(dy) == 3))
				continue;
			oakMedium.m_blocks.push_back({ IntVec3(dx, dy, 3), OAK_LEAVES });
		}
	}

	// 树叶层 - z=4 (5×5)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if ((abs(dx) == 2 && abs(dy) == 2))
				continue;
			oakMedium.m_blocks.push_back({ IntVec3(dx, dy, 4), OAK_LEAVES });
		}
	}

	// 树叶层 - z=5 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			oakMedium.m_blocks.push_back({ IntVec3(dx, dy, 5), OAK_LEAVES });
		}
	}
	oakMedium.m_blocks.push_back({ IntVec3(0, 0, 6), OAK_LEAVES });
	s_treeStamps[TreeType::OAK][TreeSize::MEDIUM] = oakMedium;

	//-------------------------------------------------------------------------------
	// Large Oak (6-8 blocks tall)
	TreeStamp oakLarge;
	oakLarge.m_type = TreeType::OAK;
	oakLarge.m_size = TreeSize::LARGE;
	oakLarge.m_radius = 4;

	// 树干 (6 blocks)
	for (int i = 0; i < 6; ++i)
	{
		oakLarge.m_blocks.push_back({ IntVec3(0, 0, i), OAK_LOG });
	}

	// 树叶层 - z=3 (7×7, 去角)
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if (abs(dx) == 3 && abs(dy) == 3)
				continue;
			if (abs(dx) == 3 || abs(dy) == 3)
			{
				if (rand() % 2 == 0) // 外圈随机
					oakLarge.m_blocks.push_back({ IntVec3(dx, dy, 3), OAK_LEAVES });
			}
			else
			{
				oakLarge.m_blocks.push_back({ IntVec3(dx, dy, 3), OAK_LEAVES });
			}
		}
	}

	// 树叶层 - z=4,5 (5×5)
	for (int z = 4; z <= 5; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if ((abs(dx) == 2 && abs(dy) == 2))
					continue;
				oakLarge.m_blocks.push_back({ IntVec3(dx, dy, z), OAK_LEAVES });
			}
		}
	}

	// 树叶层 - z=6 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			oakLarge.m_blocks.push_back({ IntVec3(dx, dy, 6), OAK_LEAVES });
		}
	}
	oakLarge.m_blocks.push_back({ IntVec3(0, 0, 7), OAK_LEAVES });
	s_treeStamps[TreeType::OAK][TreeSize::LARGE] = oakLarge;
}

void TerrainGenerator::InitializeBirchStamps()
{
	const Block BIRCH_LOG = BlockDefinition::s_nameToIndexMap["BirchLog"];
	const Block BIRCH_LEAVES = BlockDefinition::s_nameToIndexMap["BirchLeaves"];

	// ===========================
	// Birch Tree (白桦树) - 瘦高,树冠短且靠上
	// ===========================

	//-------------------------------------------------------------------------------
	// Small Birch (5-6 blocks tall, 紧凑树冠)
	TreeStamp birchSmall;
	birchSmall.m_type = TreeType::BIRCH;
	birchSmall.m_size = TreeSize::SMALL;
	birchSmall.m_radius = 3;

	// 树干 (5 blocks)
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 0), BIRCH_LOG });
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 1), BIRCH_LOG });
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 2), BIRCH_LOG });
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 3), BIRCH_LOG });
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 4), BIRCH_LOG });

	// 树叶层 - z=3 (3×3, 小树冠位置靠上)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			birchSmall.m_blocks.push_back({ IntVec3(dx, dy, 3), BIRCH_LEAVES });
		}
	}

	// 树叶层 - z=4 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			birchSmall.m_blocks.push_back({ IntVec3(dx, dy, 4), BIRCH_LEAVES });
		}
	}

	// 顶部 - z=5 (单点)
	birchSmall.m_blocks.push_back({ IntVec3(0, 0, 5), BIRCH_LEAVES });

	s_treeStamps[TreeType::BIRCH][TreeSize::SMALL] = birchSmall;

	//-------------------------------------------------------------------------------
	// Medium Birch (6-7 blocks tall, 中等树冠)
	TreeStamp birchMedium;
	birchMedium.m_type = TreeType::BIRCH;
	birchMedium.m_size = TreeSize::MEDIUM;
	birchMedium.m_radius = 4;

	// 树干 (6 blocks)
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 0), BIRCH_LOG });
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 1), BIRCH_LOG });
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 2), BIRCH_LOG });
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 3), BIRCH_LOG });
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 4), BIRCH_LOG });
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 5), BIRCH_LOG });

	// 树叶层 - z=4 (5×5去角, 树冠开始扩大)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if (abs(dx) == 2 && abs(dy) == 2)
				continue;
			birchMedium.m_blocks.push_back({ IntVec3(dx, dy, 4), BIRCH_LEAVES });
		}
	}

	// 树叶层 - z=5 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			birchMedium.m_blocks.push_back({ IntVec3(dx, dy, 5), BIRCH_LEAVES });
		}
	}

	// 顶部 - z=6 (十字形)
	birchMedium.m_blocks.push_back({ IntVec3(0, 0, 6), BIRCH_LEAVES });
	birchMedium.m_blocks.push_back({ IntVec3(1, 0, 6), BIRCH_LEAVES });
	birchMedium.m_blocks.push_back({ IntVec3(-1, 0, 6), BIRCH_LEAVES });
	birchMedium.m_blocks.push_back({ IntVec3(0, 1, 6), BIRCH_LEAVES });
	birchMedium.m_blocks.push_back({ IntVec3(0, -1, 6), BIRCH_LEAVES });

	s_treeStamps[TreeType::BIRCH][TreeSize::MEDIUM] = birchMedium;

	//-------------------------------------------------------------------------------
	// Large Birch (7-9 blocks tall, 明显更大的树冠)
	TreeStamp birchLarge;
	birchLarge.m_type = TreeType::BIRCH;
	birchLarge.m_size = TreeSize::LARGE;
	birchLarge.m_radius = 5;

	// 树干 (7 blocks - 很高)
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 0), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 1), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 2), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 3), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 4), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 5), BIRCH_LOG });
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 6), BIRCH_LOG });

	// 树叶层 - z=5 (7×7去角, 大树冠底层)
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if (abs(dx) == 3 && abs(dy) == 3)
				continue;
			// 外圈随机稀疏
			if (abs(dx) == 3 || abs(dy) == 3)
			{
				if (rand() % 2 == 0)
					birchLarge.m_blocks.push_back({ IntVec3(dx, dy, 5), BIRCH_LEAVES });
			}
			else
			{
				birchLarge.m_blocks.push_back({ IntVec3(dx, dy, 5), BIRCH_LEAVES });
			}
		}
	}

	// 树叶层 - z=6 (5×5去角)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if (abs(dx) == 2 && abs(dy) == 2)
				continue;
			birchLarge.m_blocks.push_back({ IntVec3(dx, dy, 6), BIRCH_LEAVES });
		}
	}

	// 树叶层 - z=7 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			birchLarge.m_blocks.push_back({ IntVec3(dx, dy, 7), BIRCH_LEAVES });
		}
	}

	// 顶部 - z=8 (十字形)
	birchLarge.m_blocks.push_back({ IntVec3(0, 0, 8), BIRCH_LEAVES });
	birchLarge.m_blocks.push_back({ IntVec3(1, 0, 8), BIRCH_LEAVES });
	birchLarge.m_blocks.push_back({ IntVec3(-1, 0, 8), BIRCH_LEAVES });
	birchLarge.m_blocks.push_back({ IntVec3(0, 1, 8), BIRCH_LEAVES });
	birchLarge.m_blocks.push_back({ IntVec3(0, -1, 8), BIRCH_LEAVES });

	s_treeStamps[TreeType::BIRCH][TreeSize::LARGE] = birchLarge;
}

void TerrainGenerator::InitializeSpruceStamps()
{
	const Block SPRUCE_LOG = BlockDefinition::s_nameToIndexMap["SpruceLog"];
	const Block SPRUCE_LEAVES = BlockDefinition::s_nameToIndexMap["SpruceLeaves"];
	const Block SPRUCE_LEAVES_SNOW = BlockDefinition::s_nameToIndexMap["SpruceLeavesSnow"];

	// ===========================
	// Spruce Tree (云杉) - 锥形,少叶版本
	// ===========================

	//-------------------------------------------------------------------------------
	// Small Spruce (6-7 blocks tall, 瘦锥形少叶)
	TreeStamp spruceSmall;
	spruceSmall.m_type = TreeType::SPRUCE;
	spruceSmall.m_size = TreeSize::SMALL;
	spruceSmall.m_radius = 3;

	// 树干 (6 blocks)
	for (int i = 0; i < 6; ++i)
	{
		spruceSmall.m_blocks.push_back({ IntVec3(0, 0, i), SPRUCE_LOG });
	}

	// 树叶 - 锥形从底部开始,少叶风格
	// z=2 - 3×3
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue; // 去掉角
			spruceSmall.m_blocks.push_back({ IntVec3(dx, dy, 2), SPRUCE_LEAVES });
		}
	}

	// z=3 - 3×3
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue;
			spruceSmall.m_blocks.push_back({ IntVec3(dx, dy, 3), SPRUCE_LEAVES });
		}
	}

	// z=4 - 3×3
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue;
			spruceSmall.m_blocks.push_back({ IntVec3(dx, dy, 4), SPRUCE_LEAVES });
		}
	}

	// z=5 - 十字形顶部
	spruceSmall.m_blocks.push_back({ IntVec3(0, 0, 5), SPRUCE_LEAVES });
	spruceSmall.m_blocks.push_back({ IntVec3(1, 0, 5), SPRUCE_LEAVES });
	spruceSmall.m_blocks.push_back({ IntVec3(-1, 0, 5), SPRUCE_LEAVES });
	spruceSmall.m_blocks.push_back({ IntVec3(0, 1, 5), SPRUCE_LEAVES });
	spruceSmall.m_blocks.push_back({ IntVec3(0, -1, 5), SPRUCE_LEAVES });

	// z=6 - 尖顶
	spruceSmall.m_blocks.push_back({ IntVec3(0, 0, 6), SPRUCE_LEAVES });

	s_treeStamps[TreeType::SPRUCE][TreeSize::SMALL] = spruceSmall;

	//-------------------------------------------------------------------------------
	// Medium Spruce (7-9 blocks tall, 标准锥形)
	TreeStamp spruceMedium;
	spruceMedium.m_type = TreeType::SPRUCE;
	spruceMedium.m_size = TreeSize::MEDIUM;
	spruceMedium.m_radius = 4;

	// 树干 (8 blocks)
	for (int i = 0; i < 8; ++i)
	{
		spruceMedium.m_blocks.push_back({ IntVec3(0, 0, i), SPRUCE_LOG });
	}

	// 树叶 - 标准锥形
	// z=2 - 5×5去角
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if (abs(dx) == 2 && abs(dy) == 2)
				continue;
			spruceMedium.m_blocks.push_back({ IntVec3(dx, dy, 2), SPRUCE_LEAVES });
		}
	}

	// z=3,4 - 5×5去角
	for (int z = 3; z <= 4; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (abs(dx) == 2 && abs(dy) == 2)
					continue;
				spruceMedium.m_blocks.push_back({ IntVec3(dx, dy, z), SPRUCE_LEAVES });
			}
		}
	}

	// z=5 - 3×3
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			spruceMedium.m_blocks.push_back({ IntVec3(dx, dy, 5), SPRUCE_LEAVES });
		}
	}

	// z=6 - 3×3去角
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue;
			spruceMedium.m_blocks.push_back({ IntVec3(dx, dy, 6), SPRUCE_LEAVES });
		}
	}

	// z=7 - 十字形
	spruceMedium.m_blocks.push_back({ IntVec3(0, 0, 7), SPRUCE_LEAVES });
	spruceMedium.m_blocks.push_back({ IntVec3(1, 0, 7), SPRUCE_LEAVES });
	spruceMedium.m_blocks.push_back({ IntVec3(-1, 0, 7), SPRUCE_LEAVES });
	spruceMedium.m_blocks.push_back({ IntVec3(0, 1, 7), SPRUCE_LEAVES });
	spruceMedium.m_blocks.push_back({ IntVec3(0, -1, 7), SPRUCE_LEAVES });

	// z=8 - 尖顶
	spruceMedium.m_blocks.push_back({ IntVec3(0, 0, 8), SPRUCE_LEAVES });

	s_treeStamps[TreeType::SPRUCE][TreeSize::MEDIUM] = spruceMedium;

	//-------------------------------------------------------------------------------
	// Large Spruce (9-11 blocks tall, 茂密锥形多叶)
	TreeStamp spruceLarge;
	spruceLarge.m_type = TreeType::SPRUCE;
	spruceLarge.m_size = TreeSize::LARGE;
	spruceLarge.m_radius = 5;

	// 树干 (10 blocks)
	for (int i = 0; i < 10; ++i)
	{
		spruceLarge.m_blocks.push_back({ IntVec3(0, 0, i), SPRUCE_LOG });
	}

	// 树叶 - 茂密锥形,多叶风格
	// z=2 - 7×7去角(底层很宽)
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if (abs(dx) == 3 && abs(dy) == 3)
				continue;
			spruceLarge.m_blocks.push_back({ IntVec3(dx, dy, 2), SPRUCE_LEAVES });
		}
	}

	// z=3 - 7×7去角
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if (abs(dx) == 3 && abs(dy) == 3)
				continue;
			spruceLarge.m_blocks.push_back({ IntVec3(dx, dy, 3), SPRUCE_LEAVES });
		}
	}

	// z=4,5 - 5×5去角
	for (int z = 4; z <= 5; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (abs(dx) == 2 && abs(dy) == 2)
					continue;
				spruceLarge.m_blocks.push_back({ IntVec3(dx, dy, z), SPRUCE_LEAVES });
			}
		}
	}

	// z=6,7 - 3×3
	for (int z = 6; z <= 7; ++z)
	{
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				spruceLarge.m_blocks.push_back({ IntVec3(dx, dy, z), SPRUCE_LEAVES });
			}
		}
	}

	// z=8 - 3×3去角
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue;
			spruceLarge.m_blocks.push_back({ IntVec3(dx, dy, 8), SPRUCE_LEAVES });
		}
	}

	// z=9 - 十字形
	spruceLarge.m_blocks.push_back({ IntVec3(0, 0, 9), SPRUCE_LEAVES });
	spruceLarge.m_blocks.push_back({ IntVec3(1, 0, 9), SPRUCE_LEAVES });
	spruceLarge.m_blocks.push_back({ IntVec3(-1, 0, 9), SPRUCE_LEAVES });
	spruceLarge.m_blocks.push_back({ IntVec3(0, 1, 9), SPRUCE_LEAVES });
	spruceLarge.m_blocks.push_back({ IntVec3(0, -1, 9), SPRUCE_LEAVES });

	// z=10 - 尖顶
	spruceLarge.m_blocks.push_back({ IntVec3(0, 0, 10), SPRUCE_LEAVES });

	s_treeStamps[TreeType::SPRUCE][TreeSize::LARGE] = spruceLarge;

	// ===========================
	// Snowy Spruce (雪地云杉) - 相同结构,使用雪叶子
	// ===========================

	// Small Snowy Spruce
	TreeStamp spruceSnowySmall = spruceSmall; // 复制结构
	spruceSnowySmall.m_type = TreeType::SPRUCE_SNOWY;
	// 替换所有树叶为雪叶子
	for (auto& blockData : spruceSnowySmall.m_blocks)
	{
		if (blockData.second.GetTypeIndex() == SPRUCE_LEAVES.GetTypeIndex())
		{
			blockData.second = SPRUCE_LEAVES_SNOW;
		}
	}
	s_treeStamps[TreeType::SPRUCE_SNOWY][TreeSize::SMALL] = spruceSnowySmall;

	// Medium Snowy Spruce
	TreeStamp spruceSnowyMedium = spruceMedium;
	spruceSnowyMedium.m_type = TreeType::SPRUCE_SNOWY;
	for (auto& blockData : spruceSnowyMedium.m_blocks)
	{
		if (blockData.second.GetTypeIndex() == SPRUCE_LEAVES.GetTypeIndex())
		{
			blockData.second = SPRUCE_LEAVES_SNOW;
		}
	}
	s_treeStamps[TreeType::SPRUCE_SNOWY][TreeSize::MEDIUM] = spruceSnowyMedium;

	// Large Snowy Spruce
	TreeStamp spruceSnowyLarge = spruceLarge;
	spruceSnowyLarge.m_type = TreeType::SPRUCE_SNOWY;
	for (auto& blockData : spruceSnowyLarge.m_blocks)
	{
		if (blockData.second.GetTypeIndex() == SPRUCE_LEAVES.GetTypeIndex())
		{
			blockData.second = SPRUCE_LEAVES_SNOW;
		}
	}
	s_treeStamps[TreeType::SPRUCE_SNOWY][TreeSize::LARGE] = spruceSnowyLarge;
}

void TerrainGenerator::InitializeCuctusStamps()
{
	const Block CACTUS = BlockDefinition::s_nameToIndexMap["CactusLog"];
	// ===========================
	// Cactus (仙人掌) - 简单
	// ===========================

	TreeStamp cactusSmall;
	cactusSmall.m_type = TreeType::CACTUS;
	cactusSmall.m_size = TreeSize::SMALL;
	cactusSmall.m_radius = 1;

	for (int i = 0; i < 3; ++i)
	{
		cactusSmall.m_blocks.push_back({ IntVec3(0, 0, i), CACTUS });
	}

	s_treeStamps[TreeType::CACTUS][TreeSize::SMALL] = cactusSmall;

	TreeStamp cactusMedium;
	cactusMedium.m_type = TreeType::CACTUS;
	cactusMedium.m_size = TreeSize::MEDIUM;
	cactusMedium.m_radius = 1;

	for (int i = 0; i < 4; ++i)
	{
		cactusMedium.m_blocks.push_back({ IntVec3(0, 0, i), CACTUS });
	}

	s_treeStamps[TreeType::CACTUS][TreeSize::MEDIUM] = cactusMedium;

	TreeStamp cactusLarge;
	cactusLarge.m_type = TreeType::CACTUS;
	cactusLarge.m_size = TreeSize::LARGE;
	cactusLarge.m_radius = 1;

	for (int i = 0; i < 5; ++i)
	{
		cactusLarge.m_blocks.push_back({ IntVec3(0, 0, i), CACTUS });
	}

	s_treeStamps[TreeType::CACTUS][TreeSize::LARGE] = cactusLarge;
}

void TerrainGenerator::InitializeJungleStamps()
{
	const Block JUNGLE_LOG = BlockDefinition::s_nameToIndexMap["JungleLog"];
	const Block JUNGLE_LEAVES = BlockDefinition::s_nameToIndexMap["JungleLeaves"];

	// ===========================
	// Jungle Tree (丛林树) - 高大,不规则,扁平大树冠
	// ===========================

	//-------------------------------------------------------------------------------
	// Small Jungle (7-8 blocks tall, 简单不规则)
	TreeStamp jungleSmall;
	jungleSmall.m_type = TreeType::JUNGLE;
	jungleSmall.m_size = TreeSize::SMALL;
	jungleSmall.m_radius = 4;

	// 树干 (7 blocks)
	for (int i = 0; i < 7; ++i)
	{
		jungleSmall.m_blocks.push_back({ IntVec3(0, 0, i), JUNGLE_LOG });
	}

	// 不规则树枝 - 四个方向各伸出一格
	jungleSmall.m_blocks.push_back({ IntVec3(1, 0, 5), JUNGLE_LOG });
	jungleSmall.m_blocks.push_back({ IntVec3(-1, 0, 5), JUNGLE_LOG });
	jungleSmall.m_blocks.push_back({ IntVec3(0, 1, 5), JUNGLE_LOG });
	jungleSmall.m_blocks.push_back({ IntVec3(0, -1, 5), JUNGLE_LOG });

	// 树冠 - z=5,6 扁平 (5×5去角)
	for (int z = 5; z <= 6; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (abs(dx) == 2 && abs(dy) == 2)
					continue;
				jungleSmall.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
			}
		}
	}

	// z=7 - 顶部 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			jungleSmall.m_blocks.push_back({ IntVec3(dx, dy, 7), JUNGLE_LEAVES });
		}
	}

	s_treeStamps[TreeType::JUNGLE][TreeSize::SMALL] = jungleSmall;

	//-------------------------------------------------------------------------------
	// Medium Jungle (12-14 blocks tall, 更高更大)
	TreeStamp jungleMedium;
	jungleMedium.m_type = TreeType::JUNGLE;
	jungleMedium.m_size = TreeSize::MEDIUM;
	jungleMedium.m_radius = 6;

	// 树干 (12 blocks - 更高)
	for (int i = 0; i < 12; ++i)
	{
		jungleMedium.m_blocks.push_back({ IntVec3(0, 0, i), JUNGLE_LOG });
	}

	// 不规则树枝层1 - z=8
	jungleMedium.m_blocks.push_back({ IntVec3(1, 0, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(2, 0, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(-1, 0, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(-2, 0, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, 1, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, 2, 8), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, -1, 8), JUNGLE_LOG });

	// 不规则树枝层2 - z=9
	jungleMedium.m_blocks.push_back({ IntVec3(1, 0, 9), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(2, 0, 9), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(-1, 0, 9), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, 1, 9), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, -1, 9), JUNGLE_LOG });
	jungleMedium.m_blocks.push_back({ IntVec3(0, -2, 9), JUNGLE_LOG });

	// 树冠 - z=9,10,11 扁平大范围 (9×9不规则)
	for (int z = 9; z <= 11; ++z)
	{
		for (int dy = -4; dy <= 4; ++dy)
		{
			for (int dx = -4; dx <= 4; ++dx)
			{
				// 去掉最外角
				if (abs(dx) == 4 && abs(dy) == 4)
					continue;
				if ((abs(dx) == 4 && abs(dy) >= 3) || (abs(dy) == 4 && abs(dx) >= 3))
					continue;

				// 外圈随机稀疏
				if (abs(dx) == 4 || abs(dy) == 4)
				{
					if (rand() % 3 != 0) // 66%概率
						jungleMedium.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
				else if (abs(dx) == 3 || abs(dy) == 3)
				{
					if (rand() % 4 != 0) // 75%概率
						jungleMedium.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
				else
				{
					jungleMedium.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
			}
		}
	}

	// z=12 - 顶部 (5×5去角)
	for (int dy = -2; dy <= 2; ++dy)
	{
		for (int dx = -2; dx <= 2; ++dx)
		{
			if (abs(dx) == 2 && abs(dy) == 2)
				continue;
			jungleMedium.m_blocks.push_back({ IntVec3(dx, dy, 12), JUNGLE_LEAVES });
		}
	}

	s_treeStamps[TreeType::JUNGLE][TreeSize::MEDIUM] = jungleMedium;

	//-------------------------------------------------------------------------------
	// Large Jungle (15-18 blocks tall, 双层树冠+巨大范围)
	TreeStamp jungleLarge;
	jungleLarge.m_type = TreeType::JUNGLE;
	jungleLarge.m_size = TreeSize::LARGE;
	jungleLarge.m_radius = 7;

	// 树干 (15 blocks - 非常高)
	for (int i = 0; i < 15; ++i)
	{
		jungleLarge.m_blocks.push_back({ IntVec3(0, 0, i), JUNGLE_LOG });
	}

	// === 下层树冠 (z=7-9) - 中等大小的扁平树冠 ===
	// 不规则树枝支撑下层树冠 - z=6
	jungleLarge.m_blocks.push_back({ IntVec3(2, 0, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(3, 0, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-2, 0, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-3, 0, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 2, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 3, 6), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -2, 6), JUNGLE_LOG });

	// 下层树冠叶子 - z=7,8,9 (7×7不规则)
	for (int z = 7; z <= 9; ++z)
	{
		for (int dy = -3; dy <= 3; ++dy)
		{
			for (int dx = -3; dx <= 3; ++dx)
			{
				if (abs(dx) == 3 && abs(dy) == 3)
					continue;
				// 随机稀疏效果
				if ((abs(dx) == 3 || abs(dy) == 3) && rand() % 2 == 0)
					continue;
				jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
			}
		}
	}

	// === 上层主树冠 (z=11-16) ===
	// 复杂不规则树枝层1 - z=10
	jungleLarge.m_blocks.push_back({ IntVec3(1, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(2, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(3, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(4, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-1, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-2, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-3, 0, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 1, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 2, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 3, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -1, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -2, 10), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -3, 10), JUNGLE_LOG });

	// 复杂不规则树枝层2 - z=11
	jungleLarge.m_blocks.push_back({ IntVec3(1, 0, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(2, 0, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(3, 0, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-1, 0, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(-2, 0, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 1, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 2, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, 3, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -1, 11), JUNGLE_LOG });
	jungleLarge.m_blocks.push_back({ IntVec3(0, -2, 11), JUNGLE_LOG });

	// 上层树冠 - z=11,12,13,14 巨大扁平 (11×11超超大不规则树冠)
	for (int z = 11; z <= 14; ++z)
	{
		for (int dy = -5; dy <= 5; ++dy)
		{
			for (int dx = -5; dx <= 5; ++dx)
			{
				// 去掉最外圈的角和远角
				if (abs(dx) == 5 && abs(dy) == 5)
					continue;
				if ((abs(dx) == 5 && abs(dy) >= 4) || (abs(dy) == 5 && abs(dx) >= 4))
					continue;

				// 外圈随机稀疏,增加不规则性
				if (abs(dx) == 5 || abs(dy) == 5)
				{
					if (rand() % 2 == 0) // 50%概率
						jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
				else if (abs(dx) == 4 || abs(dy) == 4)
				{
					if (rand() % 3 != 0) // 66%概率
						jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
				else if (abs(dx) == 3 || abs(dy) == 3)
				{
					if (rand() % 5 != 0) // 80%概率
						jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
				else
				{
					jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, z), JUNGLE_LEAVES });
				}
			}
		}
	}

	// z=15 - 顶部收窄 (7×7不规则)
	for (int dy = -3; dy <= 3; ++dy)
	{
		for (int dx = -3; dx <= 3; ++dx)
		{
			if (abs(dx) == 3 && abs(dy) == 3)
				continue;
			if ((abs(dx) == 3 || abs(dy) == 3) && rand() % 2 == 0)
				continue;
			jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, 15), JUNGLE_LEAVES });
		}
	}

	// z=16 - 尖顶 (3×3)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			if (abs(dx) == 1 && abs(dy) == 1)
				continue;
			jungleLarge.m_blocks.push_back({ IntVec3(dx, dy, 16), JUNGLE_LEAVES });
		}
	}

	s_treeStamps[TreeType::JUNGLE][TreeSize::LARGE] = jungleLarge;
}

void TerrainGenerator::InitializeAcaciaStamps()
{
	const Block ACACIA_LOG = BlockDefinition::s_nameToIndexMap["AcaciaLog"];
	const Block ACACIA_LEAVES = BlockDefinition::s_nameToIndexMap["AcaciaLeaves"];

	// ===========================
	// Acacia Tree (金合欢) - 不对称,斜向树枝,偏心树冠
	// ===========================

	//-------------------------------------------------------------------------------
	// Small Acacia (5-6 blocks tall, 简单不对称)
	TreeStamp acaciaSmall;
	acaciaSmall.m_type = TreeType::ACACIA;
	acaciaSmall.m_size = TreeSize::SMALL;
	acaciaSmall.m_radius = 4;

	// 树干 (5 blocks - 短而弯曲)
	acaciaSmall.m_blocks.push_back({ IntVec3(0, 0, 0), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(0, 0, 1), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(0, 0, 2), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(0, 0, 3), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(0, 0, 4), ACACIA_LOG });

	// 不对称斜向树枝 - z=3,4
	acaciaSmall.m_blocks.push_back({ IntVec3(1, 0, 3), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(1, 0, 4), ACACIA_LOG });
	acaciaSmall.m_blocks.push_back({ IntVec3(0, -1, 3), ACACIA_LOG });

	// 树冠 - z=4,5 扁平 (5×5去角,略微偏心)
	for (int z = 4; z <= 5; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (abs(dx) == 2 && abs(dy) == 2)
					continue;
				acaciaSmall.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
			}
		}
	}

	s_treeStamps[TreeType::ACACIA][TreeSize::SMALL] = acaciaSmall;

	//-------------------------------------------------------------------------------
	// Medium Acacia (7-9 blocks tall, 双层树冠,底层偏心)
	TreeStamp acaciaMedium;
	acaciaMedium.m_type = TreeType::ACACIA;
	acaciaMedium.m_size = TreeSize::MEDIUM;
	acaciaMedium.m_radius = 5;

	// 主树干 (7 blocks)
	for (int i = 0; i < 7; ++i)
	{
		acaciaMedium.m_blocks.push_back({ IntVec3(0, 0, i), ACACIA_LOG });
	}

	// === 底层树冠 (偏心向东南方向) ===
	// 斜向树枝支撑底层树冠 - z=3
	acaciaMedium.m_blocks.push_back({ IntVec3(1, 0, 3), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(2, 0, 3), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(0, -1, 3), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(0, -2, 3), ACACIA_LOG });

	// 底层树冠 - z=4,5 中心在(2,-1)偏离主干
	for (int z = 4; z <= 5; ++z)
	{
		for (int dy = -3; dy <= 1; ++dy) // 中心偏移到-1
		{
			for (int dx = 0; dx <= 4; ++dx) // 中心偏移到2
			{
				int offsetDy = dy + 1; // 相对于(2,-1)的偏移
				int offsetDx = dx - 2;

				if (abs(offsetDx) == 2 && abs(offsetDy) == 2)
					continue;
				acaciaMedium.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
			}
		}
	}

	// === 上层主树冠 (中心位置) ===
	// 上层树枝 - z=5,6
	acaciaMedium.m_blocks.push_back({ IntVec3(1, 0, 5), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(-1, 0, 5), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(0, 1, 5), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(1, 0, 6), ACACIA_LOG });
	acaciaMedium.m_blocks.push_back({ IntVec3(0, 1, 6), ACACIA_LOG });

	// 上层树冠 - z=6,7 (5×5不规则)
	for (int z = 6; z <= 7; ++z)
	{
		for (int dy = -2; dy <= 2; ++dy)
		{
			for (int dx = -2; dx <= 2; ++dx)
			{
				if (abs(dx) == 2 && abs(dy) == 2)
					continue;
				// 随机稀疏
				if ((abs(dx) == 2 || abs(dy) == 2) && rand() % 3 == 0)
					continue;
				acaciaMedium.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
			}
		}
	}

	s_treeStamps[TreeType::ACACIA][TreeSize::MEDIUM] = acaciaMedium;

	//-------------------------------------------------------------------------------
	// Large Acacia (9-11 blocks tall, 双层树冠,底层严重偏心)
	TreeStamp acaciaLarge;
	acaciaLarge.m_type = TreeType::ACACIA;
	acaciaLarge.m_size = TreeSize::LARGE;
	acaciaLarge.m_radius = 6;

	// 主树干 (9 blocks)
	for (int i = 0; i < 9; ++i)
	{
		acaciaLarge.m_blocks.push_back({ IntVec3(0, 0, i), ACACIA_LOG });
	}

	// === 底层树冠 (大幅偏心向东北方向) ===
	// 复杂斜向树枝支撑底层树冠 - z=4
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 0, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(2, 0, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(3, 0, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 1, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 2, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 1, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(2, 1, 4), ACACIA_LOG });

	// 斜向树枝层2 - z=5
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 0, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(2, 0, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(3, 0, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 1, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 2, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 1, 5), ACACIA_LOG });

	// 底层树冠 - z=5,6,7 中心在(3,2)大幅偏离主干
	for (int z = 5; z <= 7; ++z)
	{
		for (int dy = 0; dy <= 4; ++dy) // 中心偏移到2
		{
			for (int dx = 1; dx <= 5; ++dx) // 中心偏移到3
			{
				int offsetDy = dy - 2; // 相对于(3,2)的偏移
				int offsetDx = dx - 3;

				if (abs(offsetDx) == 2 && abs(offsetDy) == 2)
					continue;
				// 外圈随机稀疏
				if ((abs(offsetDx) == 2 || abs(offsetDy) == 2) && rand() % 2 == 0)
					continue;
				acaciaLarge.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
			}
		}
	}

	// === 第二个底层树冠 (偏心向西南) - 更不对称 ===
	// 西南方向树枝 - z=4,5
	acaciaLarge.m_blocks.push_back({ IntVec3(-1, 0, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(-2, 0, 4), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, -1, 5), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(-1, -1, 5), ACACIA_LOG });

	// 第二个底层树冠 - z=6 中心在(-2,-1)
	for (int dy = -3; dy <= 1; ++dy)
	{
		for (int dx = -4; dx <= 0; ++dx)
		{
			int offsetDy = dy + 1; // 相对于(-2,-1)
			int offsetDx = dx + 2;

			if (abs(offsetDx) == 2 && abs(offsetDy) == 2)
				continue;
			// 稀疏小树冠
			if ((abs(offsetDx) >= 2 || abs(offsetDy) >= 2) && rand() % 2 == 0)
				continue;
			acaciaLarge.m_blocks.push_back({ IntVec3(dx, dy, 6), ACACIA_LEAVES });
		}
	}

	// === 上层主树冠 (接近中心) ===
	// 上层复杂树枝 - z=7
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 0, 7), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(2, 0, 7), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(-1, 0, 7), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 1, 7), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 2, 7), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, -1, 7), ACACIA_LOG });

	// z=8
	acaciaLarge.m_blocks.push_back({ IntVec3(1, 0, 8), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(-1, 0, 8), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, 1, 8), ACACIA_LOG });
	acaciaLarge.m_blocks.push_back({ IntVec3(0, -1, 8), ACACIA_LOG });

	// 上层树冠 - z=8,9,10 (7×7不规则扁平)
	for (int z = 8; z <= 10; ++z)
	{
		for (int dy = -3; dy <= 3; ++dy)
		{
			for (int dx = -3; dx <= 3; ++dx)
			{
				if (abs(dx) == 3 && abs(dy) == 3)
					continue;
				// 外圈随机稀疏
				if (abs(dx) == 3 || abs(dy) == 3)
				{
					if (rand() % 2 == 0) // 50%概率
						acaciaLarge.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
				}
				else if ((abs(dx) == 2 || abs(dy) == 2) && rand() % 4 == 0)
				{
					continue; // 25%概率留空
				}
				else
				{
					acaciaLarge.m_blocks.push_back({ IntVec3(dx, dy, z), ACACIA_LEAVES });
				}
			}
		}
	}

	s_treeStamps[TreeType::ACACIA][TreeSize::LARGE] = acaciaLarge;
}

TreeStamp TerrainGenerator::GetTreeStamp(TreeType type, TreeSize size)
{
	if (!s_treeStampsInitialized)
	{
		InitializeTreeStamps();
	}

	auto typeIt = s_treeStamps.find(type);
	if (typeIt != s_treeStamps.end())
	{
		auto sizeIt = typeIt->second.find(size);
		if (sizeIt != typeIt->second.end())
		{
			return sizeIt->second;
		}
	}

	return TreeStamp();
}

bool TerrainGenerator::CanPlaceTree(Chunk* chunk, const TreeCandidate& candidate, const TreeStamp& stamp)
{
	if (!chunk) return false;

	int surfaceZ = candidate.m_surfaceZ;

	int maxTreeHeight = 0;
	for (const auto& block : stamp.m_blocks)
	{
		int blockZ = surfaceZ + 1 + block.first.z;
		if (blockZ > maxTreeHeight)
		{
			maxTreeHeight = blockZ;
		}
	}

	if (maxTreeHeight >= CHUNK_SIZE_Z)
	{
		return false;
	}

	return true;
}

void TerrainGenerator::PlaceTree(Chunk* chunk, const TreeCandidate& candidate, const TreeStamp& stamp)
{
	if (!chunk) return;

	// 树的基座位置 (可能在chunk外)
	int baseX = candidate.m_position.x;
	int baseY = candidate.m_position.y;
	int baseZ = candidate.m_surfaceZ + 1;

	// 遍历树木印章中的所有方块
	for (const auto& blockData : stamp.m_blocks)
	{
		IntVec3 offset = blockData.first;
		Block blockType = blockData.second;

		// 计算局部坐标 (可能超出chunk边界)
		int localX = baseX + offset.x;
		int localY = baseY + offset.y;
		int localZ = baseZ + offset.z;

		// ✅ 边界检查:只放置在chunk内的方块
		if (localX < 0 || localX >= CHUNK_SIZE_X ||
			localY < 0 || localY >= CHUNK_SIZE_Y ||
			localZ < 0 || localZ >= CHUNK_SIZE_Z)
		{
			continue; // 跳过chunk外的方块
		}

		// 转换为方块索引
		IntVec3 localPos(localX, localY, localZ);
		int blockIdx = chunk->LocalCoordsToIndex(localPos);

		// 获取当前方块类型
		Block currentBlockType = chunk->GetBlock(blockIdx);

		// 只替换空气方块
		if (currentBlockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["Air"])
		{
			chunk->SetBlock(blockIdx, blockType);
		}
		// 或者是树干可以覆盖树叶
		else if (blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["OakLog"]
			|| blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["BirchLog"]
			|| blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["SpruceLog"]
			|| blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["JungleLog"]
			|| blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["AcaciaLog"]
			|| blockType.GetTypeIndex() == BlockDefinition::s_nameToIndexMap["CactusLog"])
		{
			chunk->SetBlock(blockIdx, blockType);
		}
	}
}

int TerrainGenerator::GetDeterministicTreeType(int globalX, int globalY, int numTypes)
{
	unsigned int hash = static_cast<unsigned int>(globalX);
	hash = hash * 374761393 + static_cast<unsigned int>(globalY);
	hash = hash * 668265263;
	hash ^= (hash >> 13);
	hash = hash * 1274126177;
	hash ^= (hash >> 16);

	return hash % numTypes;
}

BiomeType TerrainGenerator::DetermineBiome(ContinentalnessLevel continent, ErosionLevel erosion, PeaksValleysLevel pv, TemperatureLevel temp, HumidityLevel humidity)
{
	if (continent <= ContinentalnessLevel::OCEAN)
		return LookupOceanBiome(continent, temp);

	// Step 2: Coast/Beach check
	if (continent == ContinentalnessLevel::COAST)
		return LookupBeachBiome(temp);

	// Step 3: Inland biomes
	BiomeCategory category = LookupInlandCategory(pv, erosion, continent, temp);

	// Step 4: Resolve category
	switch (category)
	{
	case BiomeCategory::BEACH_BIOMES: return LookupBeachBiome(temp);
	case BiomeCategory::BADLANDS: return BiomeType::BADLANDS;
	case BiomeCategory::STONY_PEAKS: return BiomeType::STONY_PEAKS;
	case BiomeCategory::SNOWY_PEAKS: return BiomeType::SNOWY_PEAKS;
	case BiomeCategory::MIDDLE_BIOMES: return LookupMiddleBiome(temp, humidity);
	}

	return BiomeType::BADLANDS;
}

BiomeType TerrainGenerator::LookupOceanBiome(ContinentalnessLevel continent, TemperatureLevel temp)
{
	if (continent == ContinentalnessLevel::DEEP_OCEAN_1 ||
		continent == ContinentalnessLevel::DEEP_OCEAN_2)
	{
		// T=0 → Frozen Ocean
		if (temp == TemperatureLevel::T0)
		{
			return BiomeType::FROZEN_OCEAN;
		}
		// T=1~4 → Deep Ocean
		else
		{
			return BiomeType::DEEP_OCEAN;
		}
	}
	// Ocean 区域
	else if (continent == ContinentalnessLevel::OCEAN)
	{
		// T=0 → Frozen Ocean
		if (temp == TemperatureLevel::T0)
		{
			return BiomeType::FROZEN_OCEAN;
		}
		// T=1~4 → Ocean
		else
		{
			return BiomeType::OCEAN;
		}
	}

	// 不应该到达这里，返回默认值
	return BiomeType::OCEAN;
}

BiomeType TerrainGenerator::LookupBeachBiome(TemperatureLevel temp)
{
	switch (temp)
	{
	case TemperatureLevel::T0:
		return BiomeType::SNOWY_BEACH;

	case TemperatureLevel::T1:
	case TemperatureLevel::T2:
	case TemperatureLevel::T3:
		return BiomeType::BEACH;

	case TemperatureLevel::T4:
		return BiomeType::DESERT;

	default:
		return BiomeType::BEACH;
	}
}
BiomeType TerrainGenerator::LookupMiddleBiome(TemperatureLevel temp, HumidityLevel humidity)
{
	static const BiomeType MIDDLE_BIOME_TABLE[5][5] =
	{
		// H=0						H=1							H=2						H=3						 H=4
		{ BiomeType::SNOWY_PLAINS, BiomeType::SNOWY_PLAINS, BiomeType::SNOWY_TAIGA, BiomeType::SNOWY_TAIGA, BiomeType::TAIGA },        // T=0
		{ BiomeType::PLAINS,       BiomeType::PLAINS,       BiomeType::FOREST,      BiomeType::TAIGA,       BiomeType::TAIGA },        // T=1
		{ BiomeType::PLAINS,       BiomeType::PLAINS,       BiomeType::FOREST,      BiomeType::FOREST,      BiomeType::JUNGLE },       // T=2
		{ BiomeType::SAVANNA,      BiomeType::SAVANNA,      BiomeType::PLAINS,      BiomeType::JUNGLE,      BiomeType::JUNGLE },       // T=3
		{ BiomeType::DESERT,       BiomeType::DESERT,       BiomeType::DESERT,      BiomeType::DESERT,      BiomeType::DESERT }        // T=4
	};

	int tempIdx = static_cast<int>(temp);
	int humidIdx = static_cast<int>(humidity);

	// 边界检查
	if (tempIdx < 0 || tempIdx >= 5 || humidIdx < 0 || humidIdx >= 5)
	{
		return BiomeType::PLAINS; // 默认值
	}

	return MIDDLE_BIOME_TABLE[tempIdx][humidIdx];

}

BiomeCategory TerrainGenerator::LookupInlandCategory(PeaksValleysLevel pv, ErosionLevel erosion, ContinentalnessLevel continent, TemperatureLevel temp)
{

	// ========================================
	// Peaks 特殊处理
	// ========================================
	if (pv == PeaksValleysLevel::PEAKS)
	{
		// Peaks + E=0 → 任何 Continent → Snowy/Stony Peaks
		if (erosion == ErosionLevel::E0)
		{
			return (temp <= TemperatureLevel::T2) ? BiomeCategory::SNOWY_PEAKS : BiomeCategory::STONY_PEAKS;
		}
		// Peaks + E=1 → Near-inland 或更内陆 → 可能是 Peaks
		else if (erosion == ErosionLevel::E1)
		{
			if (continent == ContinentalnessLevel::MID_INLAND || continent == ContinentalnessLevel::FAR_INLAND)
			{
				return (temp <= TemperatureLevel::T2) ? BiomeCategory::SNOWY_PEAKS : BiomeCategory::STONY_PEAKS;
			}
			// 否则，检查温度是否为 T4 (Badlands)
			return (temp == TemperatureLevel::T4) ? BiomeCategory::BADLANDS : BiomeCategory::MIDDLE_BIOMES;
		}
		// Peaks + E=2 → Middle biomes
		else if (erosion == ErosionLevel::E2)
		{
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Peaks + E=3 → Mid-inland 或 更远 + T4 → Badlands
		else if (erosion == ErosionLevel::E3)
		{
			if ((continent == ContinentalnessLevel::MID_INLAND || continent == ContinentalnessLevel::FAR_INLAND) &&
				temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Peaks + E=4~6 → Middle biomes
		else
		{
			return BiomeCategory::MIDDLE_BIOMES;
		}
	}

	// ========================================
	// High 特殊处理
	// ========================================
	else if (pv == PeaksValleysLevel::HIGH)
	{
		// High + E=0 → Mid/Far inland → Snowy/Stony Peaks
		if (erosion == ErosionLevel::E0)
		{
			if (continent == ContinentalnessLevel::MID_INLAND || continent == ContinentalnessLevel::FAR_INLAND)
			{
				return (temp <= TemperatureLevel::T2) ? BiomeCategory::SNOWY_PEAKS : BiomeCategory::STONY_PEAKS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// High + E=1 → Near-inland + T4 → Badlands
		else if (erosion == ErosionLevel::E1)
		{
			if (continent == ContinentalnessLevel::NEAR_INLAND && temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// High + E=3 → Mid-inland + T4 → Badlands
		else if (erosion == ErosionLevel::E3)
		{
			if (continent == ContinentalnessLevel::MID_INLAND && temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// High + E=2,4,5,6 → Middle biomes
		else
		{
			return BiomeCategory::MIDDLE_BIOMES;
		}
	}

	// ========================================
	// Mid 处理
	// ========================================
	else if (pv == PeaksValleysLevel::MID)
	{
		// Mid + E=0,1 → Near-inland+ → T4 → Badlands
		if (erosion == ErosionLevel::E0 || erosion == ErosionLevel::E1)
		{
			if (temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Mid + E=2 → Mid-inland + T4 → Badlands
		else if (erosion == ErosionLevel::E2)
		{
			if (continent == ContinentalnessLevel::MID_INLAND && temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Mid + E=3 → Near/Mid-inland + T4 → Badlands
		else if (erosion == ErosionLevel::E3)
		{
			if ((continent == ContinentalnessLevel::NEAR_INLAND || continent == ContinentalnessLevel::MID_INLAND) &&
				temp == TemperatureLevel::T4)
			{
				return BiomeCategory::BADLANDS;
			}
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Mid + E=4 → Middle biomes
		else if (erosion == ErosionLevel::E4)
		{
			return BiomeCategory::MIDDLE_BIOMES;
		}
		// Mid + E=5,6 → Beach biomes
		else
		{
			return BiomeCategory::BEACH_BIOMES;
		}
	}

	// ========================================
	// Low 处理
	// ========================================
	else if (pv == PeaksValleysLevel::LOW)
	{
		// Low + E=0,1 → T4 → Badlands
		if (erosion == ErosionLevel::E0 || erosion == ErosionLevel::E1)
		{
			return (temp == TemperatureLevel::T4) ? BiomeCategory::BADLANDS : BiomeCategory::MIDDLE_BIOMES;
		}
		// Low + E=2 → Near-inland → Middle, 否则可能 Badlands
		else if (erosion == ErosionLevel::E2)
		{
			if (continent == ContinentalnessLevel::NEAR_INLAND)
			{
				return BiomeCategory::MIDDLE_BIOMES;
			}
			return (temp == TemperatureLevel::T4) ? BiomeCategory::BADLANDS : BiomeCategory::MIDDLE_BIOMES;
		}
		// Low + E=3 → 类似 E=2
		else if (erosion == ErosionLevel::E3)
		{
			if (continent == ContinentalnessLevel::NEAR_INLAND)
			{
				return BiomeCategory::MIDDLE_BIOMES;
			}
			return (temp == TemperatureLevel::T4) ? BiomeCategory::BADLANDS : BiomeCategory::MIDDLE_BIOMES;
		}
		// Low + E=4,5,6 → Middle biomes
		else
		{
			return BiomeCategory::MIDDLE_BIOMES;
		}
	}

	// ========================================
	// Valleys 处理
	// ========================================
	else // pv == PeaksValleysLevel::VALLEYS
	{
		// Valleys → 所有情况都先检查 T4
		if (temp == TemperatureLevel::T4)
		{
			return BiomeCategory::BADLANDS;
		}
		return BiomeCategory::MIDDLE_BIOMES;
	}
}

