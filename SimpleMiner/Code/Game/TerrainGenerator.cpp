#include "TerrainGenerator.hpp"
#include "Game/Chunk.hpp"
#include "ThirdParty/Noise/SmoothNoise.hpp"
#include "ThirdParty/Noise/RawNoise.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Easing.hpp"
#include "BlockDefinition.hpp"

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
