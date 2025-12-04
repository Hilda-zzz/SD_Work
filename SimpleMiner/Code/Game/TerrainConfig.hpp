#pragma once
#include "TerrainSpline.hpp"
class PiecewiseCurve1D;

// ========================================
// Debug Visualization
// ========================================
enum class NoiseDebugMode {
	NONE = 0,

	// Raw noise values 
	CONTINENT_RAW,
	EROSION_RAW,
	PEAKS_VALLEYS_RAW,
	TEMPERATURE_RAW,
	HUMIDITY_RAW,

	// Spline-mapped values
	CONTINENT_OFFSET_MAPPED,
	EROSION_OFFSET_MAPPED,
	PV_OFFSET_MAPPED,
	CONTINENT_AMPLITUDE_MAPPED,
	EROSION_AMPLITUDE_MAPPED,

	CONTINENT_LEVEL,
	EROSION_LEVEL,
	PV_LEVEL,
	TEMPERATURE_LEVEL,
	HUMIDITY_LEVEL,
	BIOME_TYPE
};


struct TerrainCurves {
	PiecewiseCurve1D* m_continentOffset;      // Continent → 高度偏移
	PiecewiseCurve1D* m_erosionOffset;        // Erosion → 高度偏移
	PiecewiseCurve1D* m_pvOffset;             // PV → 高度偏移

	// 3D 噪声振幅曲线 (2D Noise → 3D Noise 振幅)
	PiecewiseCurve1D* m_continentAmplitude;   // Continent → 3D 振幅
	PiecewiseCurve1D* m_erosionAmplitude;     // Erosion → 3D 振幅
	PiecewiseCurve1D* m_pvAmplitude;          // PV → 3D 振幅 (新增)

	// 垂直压缩曲线 (高度 → 压缩因子)
	PiecewiseCurve1D* m_heightSqueeze;        // Height (Z) → 压缩因子

	void InitializeDefaults();
	void Cleanup();
};

class TerrainConfig
{
public:
	// === Singleton ===
	static TerrainConfig& GetInstance();
	TerrainConfig(const TerrainConfig&) = delete;
	TerrainConfig& operator=(const TerrainConfig&) = delete;

	void ResetToDefaults();
	void SaveToFile(const char* filename) const;
	void LoadFromFile(const char* filename);

public:
	// ========================================
	// 2D Noise Parameters
	// ========================================

	// Continent 
	struct ContinentParams {
		float m_scale = 256.f;
		int m_octaves = 8;
		bool m_enabled = true;
	} m_continent;

	// Erosion
	struct ErosionParams {
		float m_scale = 128.0f;
		int m_octaves = 4;
		bool m_enabled = true;
	} m_erosion;

	// Peaks and Valleys
	struct PVParams {
		float m_scale = 256.0f;
		int m_octaves = 8;
		float m_influence = 15.0f;
		bool m_enabled = true;
	} m_peaksValleys;

	// Temperature
	struct TemperatureParams {
		float m_scale = 256.0f;
		int m_octaves = 4;
		bool m_enabled = true;
	} m_temperature;

	// Humidity
	struct HumidityParams {
		float m_scale = 256.0f;
		int m_octaves = 4;
		bool m_enabled = true;
	} m_humidity;

	// Tree
	struct TreeParams {
		float m_scale = 32.0f;
		int m_octaves = 8;
		bool m_enabled = true;
	} m_tree;

	struct DebugParams {
		NoiseDebugMode m_activeDebugMode = NoiseDebugMode::NONE;
		bool m_showNoiseDebug = false;
	} m_debug;

	// Cheese Cave 参数
	struct CheeseParams {
		float m_scale = 40.0f;           // 3D噪声尺度
		int m_octaves = 4;
		float m_hollowness = 0.3f;       // 空洞密度 (0-1)
		float m_threshold = 0.0f;        // 密度阈值
		bool m_enabled = true;
	} m_cheeseCave;

	// Spaghetti Cave 参数
	struct SpaghettiParams {
		float m_scale = 80.0f;           // 3D噪声尺度
		int m_octaves = 3;
		float m_thickness = 0.05f;       // 隧道厚度
		float m_ridgeThreshold = 0.5f;   // Ridge噪声阈值
		bool m_enabled = true;

		int m_baseLevel = 20;
		int m_highLevel = 70;
	} m_spaghettiCave;

	// ========================================
	// Seeds (种子 - 运行时可改)
	// ========================================

	struct SeedParams {
		unsigned int m_gameSeed = 0u;
		unsigned int m_continentSeed = 0u+1;
		unsigned int m_erosionSeed = 0u+2;
		unsigned int m_weirdnessSeed = 0u+3;
		unsigned int m_temperatureSeed = 0u+4;
		unsigned int m_humiditySeed = 0u+5;
		unsigned int m_treeSeed = 0u + 6;

		unsigned int m_cheeseCaveSeed = 0u + 7;
		unsigned int m_spaghettiCaveSeed = 0u + 8;
		unsigned int m_noodleCaveSeed = 0u + 9;

		void RegenerateSeeds();  // 重新生成所有种子
	} m_seeds;

	// Splines
	TerrainCurves m_curves;

private:
	TerrainConfig(); 
	~TerrainConfig();

	static const ContinentParams DEFAULT_CONTINENT;
	static const ErosionParams DEFAULT_EROSION;
	static const PVParams DEFAULT_PV;
	static const TemperatureParams DEFAULT_TEMPERATURE;
	static const HumidityParams DEFAULT_HUMIDITY;
	static const TreeParams DEFAULT_TREE;
};