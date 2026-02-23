#include "Game/RegionDefinitionLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <algorithm>

RegionDefinitionLoader::RegionDefinitionLoader()
{
}

RegionDefinitionLoader::~RegionDefinitionLoader()
{
}

bool RegionDefinitionLoader::Load(const std::string& xmlPath, RegionDefinition& outDefinition)
{
	XmlDocument doc;
	XmlResult result = doc.LoadFile(xmlPath.c_str());

	if (result != tinyxml2::XML_SUCCESS) {
		ERROR_RECOVERABLE(Stringf("Failed to load XML file: %s", xmlPath.c_str()));
		return false;
	}

	XmlElement const* root = doc.RootElement();
	if (!root) {
		ERROR_RECOVERABLE("XML file has no root element");
		return false;
	}

	return ParseRegionConfig(root, outDefinition);
}

bool RegionDefinitionLoader::ParseRegionConfig(XmlElement const* root, RegionDefinition& outDefinition)
{
	// 基本信息
	std::string name = ParseXmlAttribute(root, "name", "Untitled");
	std::string biomeType = ParseXmlAttribute(root, "biomeType", "Unknown");

	outDefinition.SetName(name);
	outDefinition.SetBiomeType(biomeType);

	// 解析 BaseLayer
	XmlElement const* baseLayerElem = root->FirstChildElement("BaseLayer");
	if (baseLayerElem) {
		BaseLayerConfig baseLayerConfig;
		if (ParseBaseLayer(baseLayerElem, baseLayerConfig)) 
		{
			outDefinition.SetBaseLayerConfig(baseLayerConfig);
		}
	}

	// 解析 ColoredLayers
	XmlElement const* coloredLayersElem = root->FirstChildElement("ColoredLayers");
	if (coloredLayersElem) {
		outDefinition.m_coloredLayers = ParseColoredLayers(coloredLayersElem);
	}

	// 解析 DetailScript
	XmlElement const* detailScriptElem = root->FirstChildElement("DetailScript");
	if (detailScriptElem) {
		outDefinition.m_detailScriptPath = ParseXmlAttribute(detailScriptElem, "path", "");
		outDefinition.m_detailScriptEnabled = ParseXmlAttribute(detailScriptElem, "enabled", false);
	}

	return true;
}

// ============================================
// BaseLayer 解析
// ============================================

bool RegionDefinitionLoader::ParseBaseLayer(XmlElement const* element, BaseLayerConfig& outConfig)
{
	// 密度来源
	XmlElement const* densitySourceElem = element->FirstChildElement("DensitySource");
	if (densitySourceElem) {
		std::string typeStr = ParseXmlAttribute(densitySourceElem, "type", "herringbone_grayscale");
		outConfig.densitySource = ParseDensitySourceType(typeStr);
	}

	// 主噪声
	XmlElement const* noiseElem = element->FirstChildElement("Noise");
	if (noiseElem) {
		outConfig.mainNoise = ParseNoiseConfig(noiseElem);
	}

	// 材质层
	XmlElement const* materialLayersElem = element->FirstChildElement("MaterialLayers");
	if (materialLayersElem) {
		outConfig.materialLayers = ParseMaterialLayers(materialLayersElem);
	}

	// 装饰层
	XmlElement const* decoLayersElem = element->FirstChildElement("DecorationLayers");
	if (decoLayersElem) {
		outConfig.decorationLayers = ParseDecorationLayers(decoLayersElem);
	}

	// 边缘贴纸
	XmlElement const* edgeStickersElem = element->FirstChildElement("EdgeStickers");
	if (edgeStickersElem) {
		outConfig.edgeStickers = ParseEdgeStickers(edgeStickersElem);
	}

	return true;
}

NoiseConfig RegionDefinitionLoader::ParseNoiseConfig(XmlElement const* element)
{
	NoiseConfig config;

	config.scale = ParseXmlAttribute(element, "scale", 64.0f);
	config.octaves = ParseXmlAttribute(element, "octaves", 2);
	config.persistence = ParseXmlAttribute(element, "persistence", 0.5f);
	config.octaveScale = ParseXmlAttribute(element, "octaveScale", 2.0f);
	config.seedOffset = ParseXmlAttribute(element, "seedOffset", 0);

	// 噪声效果
	XmlElement const* effectElem = element->FirstChildElement("Effect");
	if (effectElem) {
		std::string typeStr = ParseXmlAttribute(effectElem, "type", "SUBTRACT_ABS");
		config.effectType = ParseNoiseEffectType(typeStr);
		config.effectStrength = ParseXmlAttribute(effectElem, "strength", 0.3f);
	}

	return config;
}

std::vector<MaterialLayerConfig> RegionDefinitionLoader::ParseMaterialLayers(XmlElement const* element)
{
	std::vector<MaterialLayerConfig> layers;

	XmlElement const* layerElem = element->FirstChildElement("Layer");
	while (layerElem) {
		layers.push_back(ParseMaterialLayer(layerElem));
		layerElem = layerElem->NextSiblingElement("Layer");
	}

	return layers;
}

MaterialLayerConfig RegionDefinitionLoader::ParseMaterialLayer(XmlElement const* element)
{
	MaterialLayerConfig config;

	std::string matStr = ParseXmlAttribute(element, "material", "MAT_AIR");
	config.material = ParseMaterialType(matStr);

	// 密度范围
	XmlElement const* densityRangeElem = element->FirstChildElement("DensityRange");
	if (densityRangeElem) {
		config.densityMin = ParseXmlAttribute(densityRangeElem, "min", 0.0f);
		config.densityMax = ParseXmlAttribute(densityRangeElem, "max", 1.0f);
	}

	// 纹理
	XmlElement const* textureElem = element->FirstChildElement("Texture");
	if (textureElem) {
		config.texturePath = ParseXmlAttribute(textureElem, "path", "");
		config.textureEnabled = ParseXmlAttribute(textureElem, "enabled", true);

		XmlElement const* colorAdjustElem = textureElem->FirstChildElement("ColorAdjust");
		if (colorAdjustElem) {
			config.colorAdjust = ParseColorAdjustment(colorAdjustElem);
		}
	}

	return config;
}

std::vector<DecorationConfig> RegionDefinitionLoader::ParseDecorationLayers(XmlElement const* element)
{
	std::vector<DecorationConfig> layers;

	XmlElement const* decoElem = element->FirstChildElement("Decoration");
	while (decoElem) {
		layers.push_back(ParseDecorationLayer(decoElem));
		decoElem = decoElem->NextSiblingElement("Decoration");
	}

	return layers;
}

DecorationConfig RegionDefinitionLoader::ParseDecorationLayer(XmlElement const* element)
{
	DecorationConfig config;

	std::string matStr = ParseXmlAttribute(element, "material", "MAT_AIR");
	config.material = ParseMaterialType(matStr);

	// 基础密度条件
	XmlElement const* conditionElem = element->FirstChildElement("BaseDensityCondition");
	if (conditionElem) {
		config.baseDensityMin = ParseXmlAttribute(conditionElem, "min", 0.0f);
		config.baseDensityMax = ParseXmlAttribute(conditionElem, "max", 1.0f);
	}

	// 装饰噪声
	XmlElement const* decoNoiseElem = element->FirstChildElement("DecorationNoise");
	if (decoNoiseElem) {
		config.decoNoise = ParseDecorationNoise(decoNoiseElem);
	}

	// 纹理
	XmlElement const* textureElem = element->FirstChildElement("Texture");
	if (textureElem) {
		config.texturePath = ParseXmlAttribute(textureElem, "path", "");
		config.textureEnabled = ParseXmlAttribute(textureElem, "enabled", false);
	}

	return config;
}

DecorationNoiseConfig RegionDefinitionLoader::ParseDecorationNoise(XmlElement const* element)
{
	DecorationNoiseConfig config;

	config.scale = ParseXmlAttribute(element, "scale", 16.0f);
	config.octaves = ParseXmlAttribute(element, "octaves", 2);
	config.seedOffset = ParseXmlAttribute(element, "seedOffset", 1);

	// 强度调制
	XmlElement const* modulationElem = element->FirstChildElement("StrengthModulation");
	if (modulationElem) {
		std::string typeStr = ParseXmlAttribute(modulationElem, "type", "INVERSE_DENSITY");
		config.modulationType = ParseStrengthModulationType(typeStr);
		config.modulationMultiplier = ParseXmlAttribute(modulationElem, "multiplier", 2.0f);
	}

	// 阈值
	XmlElement const* thresholdElem = element->FirstChildElement("Threshold");
	if (thresholdElem) {
		config.threshold = ParseXmlAttribute(thresholdElem, "min", 0.2f);
	}

	return config;
}

EdgeStickersConfig RegionDefinitionLoader::ParseEdgeStickers(XmlElement const* element)
{
	EdgeStickersConfig config;

	// 应用条件
	XmlElement const* applyConditionElem = element->FirstChildElement("ApplyCondition");
	if (applyConditionElem) {
		XmlElement const* densityRangeElem = applyConditionElem->FirstChildElement("DensityRange");
		if (densityRangeElem) {
			config.applyDensityMin = ParseXmlAttribute(densityRangeElem, "min", 0.0f);
			config.applyDensityMax = ParseXmlAttribute(densityRangeElem, "max", 1.0f);
		}
	}

	// 贴纸列表
	XmlElement const* stickersElem = element->FirstChildElement("Stickers");
	if (stickersElem) {
		XmlElement const* stickerElem = stickersElem->FirstChildElement("Sticker");
		while (stickerElem) {
			config.stickers.push_back(ParseSticker(stickerElem));
			stickerElem = stickerElem->NextSiblingElement("Sticker");
		}

		// 按 order 排序
		std::sort(config.stickers.begin(), config.stickers.end());
	}

	return config;
}

// ============================================
// ColoredLayers 解析
// ============================================

std::vector<ColoredLayerConfig> RegionDefinitionLoader::ParseColoredLayers(XmlElement const* element)
{
	std::vector<ColoredLayerConfig> layers;

	XmlElement const* layerElem = element->FirstChildElement("ColoredLayer");
	while (layerElem) {
		layers.push_back(ParseColoredLayer(layerElem));
		layerElem = layerElem->NextSiblingElement("ColoredLayer");
	}

	return layers;
}

ColoredLayerConfig RegionDefinitionLoader::ParseColoredLayer(XmlElement const* element)
{
	ColoredLayerConfig config;

	config.name = ParseXmlAttribute(element, "name", "Unnamed");

	// 触发颜色
	XmlElement const* triggerColorElem = element->FirstChildElement("TriggerColor");
	if (triggerColorElem) {
		int r = ParseXmlAttribute(triggerColorElem, "r", 255);
		int g = ParseXmlAttribute(triggerColorElem, "g", 0);
		int b = ParseXmlAttribute(triggerColorElem, "b", 0);
		int a = ParseXmlAttribute(triggerColorElem, "a", 255);
		config.triggerColor = Rgba8((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
	}

	// 密度来源
	XmlElement const* densitySourceElem = element->FirstChildElement("DensitySource");
	if (densitySourceElem) {
		std::string typeStr = ParseXmlAttribute(densitySourceElem, "type", "colored_pixel");
		config.densitySource = ParseDensitySourceType(typeStr);
	}

	// 主噪声
	XmlElement const* noiseElem = element->FirstChildElement("Noise");
	if (noiseElem) {
		config.mainNoise = ParseNoiseConfig(noiseElem);
	}

	// 材质层
	XmlElement const* materialLayersElem = element->FirstChildElement("MaterialLayers");
	if (materialLayersElem) {
		config.materialLayers = ParseMaterialLayers(materialLayersElem);
	}

	// 边缘检测
	XmlElement const* edgeDetectionElem = element->FirstChildElement("EdgeDetection");
	if (edgeDetectionElem) {
		config.edgeDetection = ParseEdgeDetection(edgeDetectionElem);
	}

	return config;
}

EdgeDetectionConfig RegionDefinitionLoader::ParseEdgeDetection(XmlElement const* element)
{
	EdgeDetectionConfig config;

	config.enabled = ParseXmlAttribute(element, "enabled", false);

	if (!config.enabled) {
		return config;
	}

	// 动态阈值
	XmlElement const* dynamicThresholdElem = element->FirstChildElement("DynamicThreshold");
	if (dynamicThresholdElem) {
		config.dynamicThreshold = ParseDynamicThreshold(dynamicThresholdElem);
	}

	// 边缘贴纸
	XmlElement const* edgeStickersElem = element->FirstChildElement("EdgeStickers");
	if (edgeStickersElem) {
		XmlElement const* stickerElem = edgeStickersElem->FirstChildElement("Sticker");
		while (stickerElem) {
			config.edgeStickers.push_back(ParseSticker(stickerElem));
			stickerElem = stickerElem->NextSiblingElement("Sticker");
		}

		// 按 order 排序
		std::sort(config.edgeStickers.begin(), config.edgeStickers.end());
	}

	return config;
}

DynamicThresholdConfig RegionDefinitionLoader::ParseDynamicThreshold(XmlElement const* element)
{
	DynamicThresholdConfig config;

	std::string typeStr = ParseXmlAttribute(element, "type", "NOISED_OFFSET");
	config.type = ParseDynamicThresholdType(typeStr);

	// 基础阈值
	XmlElement const* baseThresholdElem = element->FirstChildElement("BaseThreshold");
	if (baseThresholdElem) {
		config.baseThreshold = std::atof(baseThresholdElem->GetText());
	}

	// 阈值噪声
	XmlElement const* thresholdNoiseElem = element->FirstChildElement("ThresholdNoise");
	if (thresholdNoiseElem) {
		config.thresholdNoise = ParseThresholdNoise(thresholdNoiseElem);
	}

	return config;
}

ThresholdNoiseConfig RegionDefinitionLoader::ParseThresholdNoise(XmlElement const* element)
{
	ThresholdNoiseConfig config;

	config.scale = ParseXmlAttribute(element, "scale", 32.0f);
	config.octaves = ParseXmlAttribute(element, "octaves", 3);
	config.seedOffset = ParseXmlAttribute(element, "seedOffset", 100);

	XmlElement const* noiseRangeElem = element->FirstChildElement("NoiseRange");
	if (noiseRangeElem) {
		config.noiseRange = std::atof(noiseRangeElem->GetText());
	}

	return config;
}

// ============================================
// 通用解析
// ============================================

StickerConfig RegionDefinitionLoader::ParseSticker(XmlElement const* element)
{
	StickerConfig config;

	config.order = ParseXmlAttribute(element, "order", 0);
	config.path = ParseXmlAttribute(element, "path", "");
	config.offsetX = ParseXmlAttribute(element, "offsetX", 0);
	config.offsetY = ParseXmlAttribute(element, "offsetY", 0);
	config.scale = ParseXmlAttribute(element, "scale", 1.0f);

	// 色彩调整
	XmlElement const* colorAdjustElem = element->FirstChildElement("ColorAdjust");
	if (colorAdjustElem) {
		config.colorAdjust = ParseColorAdjustment(colorAdjustElem);
	}

	return config;
}

ColorAdjustment RegionDefinitionLoader::ParseColorAdjustment(XmlElement const* element)
{
	ColorAdjustment adjustment;

	adjustment.hueShift = ParseXmlAttribute(element, "hueShift", 0.0f);
	adjustment.saturation = ParseXmlAttribute(element, "saturation", 1.0f);
	adjustment.brightness = ParseXmlAttribute(element, "brightness", 1.0f);

	return adjustment;
}

BaseTextureConfig RegionDefinitionLoader::ParseBaseTexture(XmlElement const* element)
{
	BaseTextureConfig config;

	config.path = ParseXmlAttribute(element, "path", "");

	// 颜色乘法器
	XmlElement const* colorMultElem = element->FirstChildElement("ColorMultiplier");
	if (colorMultElem) {
		float r = ParseXmlAttribute(colorMultElem, "r", 1.0f);
		float g = ParseXmlAttribute(colorMultElem, "g", 1.0f);
		float b = ParseXmlAttribute(colorMultElem, "b", 1.0f);

		config.colorMultiplier.r = (unsigned char)(r * 255.0f);
		config.colorMultiplier.g = (unsigned char)(g * 255.0f);
		config.colorMultiplier.b = (unsigned char)(b * 255.0f);
	}

	return config;
}

// ============================================
// 枚举解析
// ============================================

NoiseEffectType RegionDefinitionLoader::ParseNoiseEffectType(const std::string& str)
{
	if (str == "SUBTRACT_ABS") return NoiseEffectType::SUBTRACT_ABS;
	if (str == "ADD_ABS") return NoiseEffectType::ADD_ABS;
	if (str == "SUBTRACT") return NoiseEffectType::SUBTRACT;
	if (str == "ADD") return NoiseEffectType::ADD;
	if (str == "MULTIPLY") return NoiseEffectType::MULTIPLY;

	return NoiseEffectType::SUBTRACT_ABS; // 默认值
}

StrengthModulationType RegionDefinitionLoader::ParseStrengthModulationType(const std::string& str)
{
	if (str == "INVERSE_DENSITY") return StrengthModulationType::INVERSE_DENSITY;
	if (str == "LINEAR_DENSITY") return StrengthModulationType::LINEAR_DENSITY;
	if (str == "CONSTANT") return StrengthModulationType::CONSTANT;

	return StrengthModulationType::INVERSE_DENSITY; // 默认值
}

DynamicThresholdType RegionDefinitionLoader::ParseDynamicThresholdType(const std::string& str)
{
	if (str == "NOISED_OFFSET") return DynamicThresholdType::NOISED_OFFSET;
	if (str == "SIMPLE_OFFSET") return DynamicThresholdType::SIMPLE_OFFSET;
	if (str == "DENSITY_SCALED") return DynamicThresholdType::DENSITY_SCALED;

	return DynamicThresholdType::NOISED_OFFSET; // 默认值
}

DensitySourceType RegionDefinitionLoader::ParseDensitySourceType(const std::string& str)
{
	if (str == "herringbone_grayscale") return DensitySourceType::HERRINGBONE_GRAYSCALE;
	if (str == "colored_pixel") return DensitySourceType::COLORED_PIXEL;

	return DensitySourceType::HERRINGBONE_GRAYSCALE; // 默认值
}

CellMatType RegionDefinitionLoader::ParseMaterialType(const std::string& str)
{
	// TODO: 实现材质类型字符串到枚举的映射
	// 这里需要根据你的 CellMatType 定义来实现

	if (str == "MAT_AIR") return CellMatType::MAT_EMPTY;
	if (str == "MAT_STONE") return CellMatType::MAT_STONE;
	if (str == "MAT_WOOD") return CellMatType::MAT_WOOD;
	if (str == "MAT_SAND") return CellMatType::MAT_SAND;
	if (str == "MAT_WATER") return CellMatType::MAT_WATER;
	//if (str == "MAT_GRASS") return CellMatType::MAT_GRASS;
	// ... 添加更多材质类型

	return CellMatType::MAT_EMPTY; // 默认值
}