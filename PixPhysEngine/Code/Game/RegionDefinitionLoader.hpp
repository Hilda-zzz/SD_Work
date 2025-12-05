#pragma once
#include "RegionDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"

class RegionDefinitionLoader 
{
public:
	RegionDefinitionLoader();
	~RegionDefinitionLoader();

	// 加载 XML 到 RegionDefinition
	bool Load(const std::string& xmlPath, RegionDefinition& outDefinition);

private:
	// === 顶层解析 ===
	bool ParseRegionConfig(XmlElement const* root, RegionDefinition& outDefinition);

	// === BaseLayer 解析 ===
	bool ParseBaseLayer(XmlElement const* element, BaseLayerConfig& outConfig);
	NoiseConfig ParseNoiseConfig(XmlElement const* element);
	std::vector<MaterialLayerConfig> ParseMaterialLayers(XmlElement const* element);
	MaterialLayerConfig ParseMaterialLayer(XmlElement const* element);
	std::vector<DecorationConfig> ParseDecorationLayers(XmlElement const* element);
	DecorationConfig ParseDecorationLayer(XmlElement const* element);
	DecorationNoiseConfig ParseDecorationNoise(XmlElement const* element);
	EdgeStickersConfig ParseEdgeStickers(XmlElement const* element);

	// === ColoredLayers 解析 ===
	std::vector<ColoredLayerConfig> ParseColoredLayers(XmlElement const* element);
	ColoredLayerConfig ParseColoredLayer(XmlElement const* element);
	EdgeDetectionConfig ParseEdgeDetection(XmlElement const* element);
	DynamicThresholdConfig ParseDynamicThreshold(XmlElement const* element);
	ThresholdNoiseConfig ParseThresholdNoise(XmlElement const* element);

	// === 通用解析 ===
	StickerConfig ParseSticker(XmlElement const* element);
	ColorAdjustment ParseColorAdjustment(XmlElement const* element);
	BaseTextureConfig ParseBaseTexture(XmlElement const* element);

	// === 枚举解析 ===
	NoiseEffectType ParseNoiseEffectType(const std::string& str);
	StrengthModulationType ParseStrengthModulationType(const std::string& str);
	DynamicThresholdType ParseDynamicThresholdType(const std::string& str);
	DensitySourceType ParseDensitySourceType(const std::string& str);
	CellMatType ParseMaterialType(const std::string& str);
};