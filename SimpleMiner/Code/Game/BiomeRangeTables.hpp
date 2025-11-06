#pragma once
#include "BiomeTypes.hpp"

class BiomeRangeTables
{
public:
	// ========================================
	// Continentalness 
	// ========================================
	static constexpr NoiseRange CONTINENT_RANGES[] = {
		{ -1.20f, -1.05f },   // DEEP_OCEAN_1
		{ -1.05f, -0.455f },  // DEEP_OCEAN_2
		{ -0.455f, -0.19f },  // OCEAN
		{ -0.19f, -0.11f },   // COAST
		{ -0.11f, 0.03f },    // NEAR_INLAND
		{ 0.03f, 0.30f }      // MID_INLAND
	};
	static constexpr NoiseRangeInclusive CONTINENT_RANGE_LAST =
	{ 0.30f, 1.00f };     // FAR_INLAND 

	// ========================================
	// Erosion
	// ========================================
	static constexpr NoiseRange EROSION_RANGES[] = {
		{ -1.00f, -0.78f },   // E0
		{ -0.78f, -0.375f },  // E1
		{ -0.375f, -0.2225f },// E2
		{ -0.2225f, 0.05f },  // E3
		{ 0.05f, 0.45f },     // E4
		{ 0.45f, 0.55f }      // E5
	};
	static constexpr NoiseRangeInclusive EROSION_RANGE_LAST =
	{ 0.55f, 1.00f };     // E6 

	// ========================================
	// Peaks and Valleys
	// ========================================
	static constexpr NoiseRange PV_RANGES[] = {
		{ -1.00f, -0.85f },   // VALLEYS
		{ -0.85f, -0.2f },    // LOW
		{ -0.2f, 0.2f },      // MID
		{ 0.2f, 0.7f }        // HIGH
	};
	static constexpr NoiseRangeInclusive PV_RANGE_LAST =
	{ 0.7f, 1.0f };       // PEAKS 

	// ========================================
	// Temperature
	// ========================================
	static constexpr NoiseRange TEMP_RANGES[] = {
		{ -1.00f, -0.45f },   // T0
		{ -0.45f, -0.15f },   // T1
		{ -0.15f, 0.20f },    // T2
		{ 0.20f, 0.55f }      // T3
	};
	static constexpr NoiseRangeInclusive TEMP_RANGE_LAST =
	{ 0.55f, 1.00f };     // T4 

	// ========================================
	// Humidity 
	// ========================================
	static constexpr NoiseRange HUMIDITY_RANGES[] = {
		{ -1.00f, -0.35f },   // H0
		{ -0.35f, -0.10f },   // H1
		{ -0.10f, 0.10f },    // H2
		{ 0.10f, 0.30f }      // H3
	};
	static constexpr NoiseRangeInclusive HUMIDITY_RANGE_LAST =
	{ 0.30f, 1.00f };     // H4 

	// ========================================
	// Lookup level funcs
	// ========================================

	static ContinentalnessLevel GetContinentalnessLevel(float noiseValue)
	{
		for (int i = 0; i < 6; ++i) {
			if (CONTINENT_RANGES[i].Contains(noiseValue)) {
				return static_cast<ContinentalnessLevel>(i);
			}
		}
		if (CONTINENT_RANGE_LAST.Contains(noiseValue)) {
			return ContinentalnessLevel::FAR_INLAND;
		}
		return ContinentalnessLevel::DEEP_OCEAN_1;
	}

	static ErosionLevel GetErosionLevel(float noiseValue)
	{
		for (int i = 0; i < 6; ++i) {
			if (EROSION_RANGES[i].Contains(noiseValue)) {
				return static_cast<ErosionLevel>(i);
			}
		}
		if (EROSION_RANGE_LAST.Contains(noiseValue)) {
			return ErosionLevel::E6;
		}
		return ErosionLevel::E0;
	}

	static PeaksValleysLevel GetPeaksValleysLevel(float noiseValue)
	{
		for (int i = 0; i < 4; ++i) {
			if (PV_RANGES[i].Contains(noiseValue)) {
				return static_cast<PeaksValleysLevel>(i);
			}
		}
		if (PV_RANGE_LAST.Contains(noiseValue)) {
			return PeaksValleysLevel::PEAKS;
		}
		return PeaksValleysLevel::VALLEYS;
	}

	static TemperatureLevel GetTemperatureLevel(float noiseValue)
	{
		for (int i = 0; i < 4; ++i) {
			if (TEMP_RANGES[i].Contains(noiseValue)) {
				return static_cast<TemperatureLevel>(i);
			}
		}
		if (TEMP_RANGE_LAST.Contains(noiseValue)) {
			return TemperatureLevel::T4;
		}
		return TemperatureLevel::T0;
	}

	static HumidityLevel GetHumidityLevel(float noiseValue)
	{
		for (int i = 0; i < 4; ++i) {
			if (HUMIDITY_RANGES[i].Contains(noiseValue)) {
				return static_cast<HumidityLevel>(i);
			}
		}
		if (HUMIDITY_RANGE_LAST.Contains(noiseValue)) {
			return HumidityLevel::H4;
		}
		return HumidityLevel::H0;
	}
};