#pragma once
#include "Engine/Core/Rgba8.hpp"

// Continentalness 
enum class ContinentalnessLevel : int
{
	DEEP_OCEAN_1 = 0,   // [-1.20, -1.05)
	DEEP_OCEAN_2,       // [-1.05, -0.455)
	OCEAN,              // [-0.455, -0.19)
	COAST,              // [-0.19, -0.11)
	NEAR_INLAND,        // [-0.11, 0.03)
	MID_INLAND,         // [0.03, 0.30)
	FAR_INLAND,         // [0.30, 1.00]
	COUNT
};

// Erosion
enum class ErosionLevel : int
{
	E0 = 0,  // [-1.00, -0.78)
	E1,      // [-0.78, -0.375)
	E2,      // [-0.375, -0.2225)
	E3,      // [-0.2225, 0.05)
	E4,      // [0.05, 0.45)
	E5,      // [0.45, 0.55)
	E6,      // [0.55, 1.00]
	COUNT
};

// Peaks and Valleys
enum class PeaksValleysLevel : int
{
	VALLEYS = 0,  // [-1.00, -0.85)
	LOW,          // [-0.85, -0.2)
	MID,          // [-0.2, 0.2)
	HIGH,         // [0.2, 0.7)
	PEAKS,        // [0.7, 1.0]
	COUNT
};

// Temperature
enum class TemperatureLevel : int
{
	T0 = 0,  // [-1.00, -0.45)
	T1,      // [-0.45, -0.15)
	T2,      // [-0.15, 0.20)
	T3,      // [0.20, 0.55)
	T4,      // [0.55, 1.00]
	COUNT
};

// Humidity
enum class HumidityLevel : int
{
	H0 = 0,  // [-1.00, -0.35)
	H1,      // [-0.35, -0.10)
	H2,      // [-0.10, 0.10)
	H3,      // [0.10, 0.30)
	H4,      // [0.30, 1.00]
	COUNT
};

// ========================================
// Biome 
// ========================================
enum class BiomeType : int
{
	OCEAN = 0,
	DEEP_OCEAN,
	FROZEN_OCEAN,

	BEACH,
	SNOWY_BEACH,

	PLAINS,
	SNOWY_PLAINS,
	DESERT,
	SAVANNA,

	FOREST,
	JUNGLE,
	TAIGA,
	SNOWY_TAIGA,

	STONY_PEAKS,
	SNOWY_PEAKS,

	BADLANDS,

	COUNT
};

enum class BiomeCategory
{
	BEACH_BIOMES,
	BADLANDS,
	STONY_PEAKS,
	SNOWY_PEAKS,
	MIDDLE_BIOMES
};

struct NoiseRange
{
	float m_min;
	float m_max;

	bool Contains(float value) const
	{
		return value >= m_min && value < m_max;
	}
};

// the last range should contain the m_max
struct NoiseRangeInclusive
{
	float m_min;
	float m_max;

	bool Contains(float value) const
	{
		return value >= m_min && value <= m_max;
	}
};

Rgba8 GetLevelColor(int level, int maxLevels);

Rgba8 GetBiomeColor(BiomeType biomeType);