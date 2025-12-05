#pragma once
#include "Game/RegionDefinition.hpp"
#include "Game/CellChunk.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <map>

class Image;
class RegionGenerationData_v2;

// === Cell 生成器：基于 RegionDefinition 配置驱动生成 ===
class ConfigDrivenCellGenerator {
public:
	// === 主入口：生成 Chunk 的所有 cells ===
	static void GenerateChunkCells(
		CellChunk* chunk,
		const RegionDefinition& regionDef,
		RegionGenerationData const* regionData  // 包含 herringbone pixels
	);

private:
	// === BaseLayer 生成 ===
	static void GenerateBaseLayer(
		CellChunk* chunk,
		const BaseLayerConfig& config,
		RegionGenerationData const* regionData
	);

	static void ApplyMaterialLayers(
		Cell& cell,
		float density,
		const std::vector<MaterialLayerConfig>& layers,
		int worldX, int worldY
	);

	static void ApplyDecorationLayers(
		Cell& cell,
		float baseDensity,
		float advancedDensity,
		const std::vector<DecorationConfig>& decorations,
		int worldX, int worldY
	);

	static void ApplyBaseEdgeStickers(
		Cell& cell,
		float density,
		const EdgeStickersConfig& edgeConfig,
		int worldX, int worldY
	);

	// === ColoredLayers 生成 ===
	static void GenerateColoredLayers(
		CellChunk* chunk,
		const std::vector<ColoredLayerConfig>& layers,
		RegionGenerationData const* regionData
	);

	static void GenerateColoredLayer(
		CellChunk* chunk,
		const ColoredLayerConfig& layerConfig,
		RegionGenerationData const* regionData
	);

	static void ApplyEdgeDetection(
		Cell& cell,
		float density,
		const EdgeDetectionConfig& edgeConfig,
		int worldX, int worldY
	);

	// === 密度计算 ===
	static float CalculateDensity(
		int worldX, int worldY,
		DensitySourceType sourceType,
		RegionGenerationData const* regionData,
		const Rgba8* targetColor = nullptr  // 用于 colored_pixel
	);

	// === 噪声系统 ===
	static float ApplyNoiseEffect(
		float baseDensity,
		int worldX, int worldY,
		const NoiseConfig& noiseConfig
	);

	static float ComputeDecorationNoise(
		int worldX, int worldY,
		float baseDensity,
		const DecorationNoiseConfig& noiseConfig
	);

	static float ComputeDynamicThreshold(
		int worldX, int worldY,
		float density,
		const DynamicThresholdConfig& thresholdConfig
	);

	// === 强度调制 ===
	static float ApplyStrengthModulation(
		float noise,
		float baseDensity,
		StrengthModulationType type,
		float multiplier
	);

	// === 贴纸系统 ===
	static void ApplySticker(
		Cell& cell,
		const StickerConfig& sticker,
		int worldX, int worldY
	);

	static Rgba8 ApplyColorAdjustment(
		const Rgba8& color,
		const ColorAdjustment& adjustment
	);

	// === 纹理采样 ===
	static Rgba8 SampleTexture(
		const std::string& texturePath,
		int worldX, int worldY,
		float scale = 1.0f
	);

	// === 辅助函数 ===
	static float ClampDensity(float density);
	static Rgba8 GetPixelColor(int worldX, int worldY, RegionGenerationData_v2 const* regionData);
};