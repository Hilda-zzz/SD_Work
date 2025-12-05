#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/CellMatDef.hpp"
#include <string>
#include <vector>
#include <map>

// ============================================
// 枚举类型
// ============================================

// 噪声效果类型
enum class NoiseEffectType {
	SUBTRACT_ABS,  // density -= abs(noise) * strength
	ADD_ABS,       // density += abs(noise) * strength
	SUBTRACT,      // density -= noise * strength
	ADD,           // density += noise * strength
	MULTIPLY       // density *= (1.0 + noise * strength)
};

// 强度调制类型
enum class StrengthModulationType {
	INVERSE_DENSITY,  // noise *= (1.0 + (1.0 - density) * multiplier)
	LINEAR_DENSITY,   // noise *= (1.0 + density * multiplier)
	CONSTANT          // noise *= multiplier
};

// 动态阈值类型
enum class DynamicThresholdType {
	NOISED_OFFSET,     // -density + base + abs(noise) * range
	SIMPLE_OFFSET,     // base + abs(noise) * range
	DENSITY_SCALED     // base * (1.0 + density) + abs(noise) * range
};

// 密度来源类型
enum class DensitySourceType {
	HERRINGBONE_GRAYSCALE,  // 从灰度 herringbone pixel 插值
	COLORED_PIXEL           // 从彩色 pixel 颜色匹配
};

// ============================================
// 配置结构体
// ============================================

// 色彩调整
struct ColorAdjustment {
	float hueShift = 0.0f;      // 色相偏移（度数，0-360）
	float saturation = 1.0f;    // 饱和度（0-2，1为原始）
	float brightness = 1.0f;    // 亮度（0-2，1为原始）

	ColorAdjustment() = default;
	ColorAdjustment(float h, float s, float b)
		: hueShift(h), saturation(s), brightness(b) {}
};

// 噪声配置
struct NoiseConfig {
	float scale = 64.0f;
	int octaves = 2;
	float persistence = 0.5f;
	float octaveScale = 2.0f;
	unsigned int seedOffset = 0;

	// 噪声效果
	NoiseEffectType effectType = NoiseEffectType::SUBTRACT_ABS;
	float effectStrength = 0.3f;

	NoiseConfig() = default;
};

// 材质层配置
struct MaterialLayerConfig {
	CellMatType material = CellMatType::MAT_EMPTY;
	float densityMin = 0.0f;
	float densityMax = 1.0f;
	std::string texturePath;
	bool textureEnabled = false;
	ColorAdjustment colorAdjust;

	MaterialLayerConfig() = default;
};

// 装饰噪声配置
struct DecorationNoiseConfig {
	float scale = 16.0f;
	int octaves = 2;
	unsigned int seedOffset = 1;

	// 强度调制
	StrengthModulationType modulationType = StrengthModulationType::INVERSE_DENSITY;
	float modulationMultiplier = 2.0f;

	// 阈值
	float threshold = 0.2f;

	DecorationNoiseConfig() = default;
};

// 装饰层配置
struct DecorationConfig {
	CellMatType material = CellMatType::MAT_EMPTY;
	float baseDensityMin = 0.0f;
	float baseDensityMax = 1.0f;

	DecorationNoiseConfig decoNoise;

	std::string texturePath;
	bool textureEnabled = false;

	DecorationConfig() = default;
};

// 贴纸配置
struct StickerConfig {
	int order = 0;
	std::string path;
	int offsetX = 0;
	int offsetY = 0;
	float scale = 1.0f;
	ColorAdjustment colorAdjust;

	StickerConfig() = default;

	// 按 order 排序用
	bool operator<(const StickerConfig& other) const {
		return order < other.order;
	}
};

// 边缘贴纸配置
struct EdgeStickersConfig {
	float applyDensityMin = 0.0f;
	float applyDensityMax = 1.0f;
	std::vector<StickerConfig> stickers;

	EdgeStickersConfig() = default;
};

// 基础纹理配置
struct BaseTextureConfig {
	std::string path;
	Rgba8 colorMultiplier = Rgba8(255, 255, 255, 255);

	BaseTextureConfig() = default;
};

// 动态阈值噪声配置
struct ThresholdNoiseConfig {
	float scale = 32.0f;
	int octaves = 3;
	unsigned int seedOffset = 100;
	float noiseRange = 0.5f;

	ThresholdNoiseConfig() = default;
};

// 动态阈值配置
struct DynamicThresholdConfig {
	DynamicThresholdType type = DynamicThresholdType::NOISED_OFFSET;
	float baseThreshold = 1.75f;
	ThresholdNoiseConfig thresholdNoise;

	DynamicThresholdConfig() = default;
};

// 边缘检测配置
struct EdgeDetectionConfig {
	bool enabled = false;
	DynamicThresholdConfig dynamicThreshold;
	std::vector<StickerConfig> edgeStickers;

	EdgeDetectionConfig() = default;
};

// 基础层配置
struct BaseLayerConfig {
	DensitySourceType densitySource = DensitySourceType::HERRINGBONE_GRAYSCALE;
	NoiseConfig mainNoise;
	std::vector<MaterialLayerConfig> materialLayers;
	std::vector<DecorationConfig> decorationLayers;
	EdgeStickersConfig edgeStickers;

	BaseLayerConfig() = default;
};

// 彩色层配置
struct ColoredLayerConfig {
	std::string name;
	Rgba8 triggerColor;
	DensitySourceType densitySource = DensitySourceType::COLORED_PIXEL;
	NoiseConfig mainNoise;
	std::vector<MaterialLayerConfig> materialLayers;

	// 边缘检测
	EdgeDetectionConfig edgeDetection;

	ColoredLayerConfig() = default;
};

// ============================================
// RegionDefinition 主类
// ============================================

class RegionDefinition 
{
public:
	RegionDefinition();
	~RegionDefinition();

	// === 加载与保存 ===
	bool LoadFromXML(const std::string& xmlPath);
	bool SaveToXML(const std::string& xmlPath) const;

	// === 访问器 ===
	std::string GetName() const { return m_name; }
	std::string GetBiomeType() const { return m_biomeType; }

	BaseLayerConfig const& GetBaseLayer() const { return m_baseLayer; }
	std::vector<ColoredLayerConfig> const& GetColoredLayers() const { return m_coloredLayers; }

	std::string GetDetailScriptPath() const { return m_detailScriptPath; }
	bool IsDetailScriptEnabled() const { return m_detailScriptEnabled; }

	// === 查询彩色层 ===
	ColoredLayerConfig const* FindColoredLayer(const Rgba8& triggerColor) const;
	bool HasColoredLayer(const Rgba8& triggerColor) const;

	// === 设置器（用于运行时修改）===
	void SetName(const std::string& name) { m_name = name; }
	void SetBiomeType(const std::string& biomeType) { m_biomeType = biomeType; }
	void SetBaseLayerConfig(BaseLayerConfig const& config) { m_baseLayer = config; }

	// === 调试输出 ===
	void PrintDebugInfo() const;

private:
	// 辅助函数
	void BuildColorLookupTable();
	unsigned int ColorToKey(const Rgba8& color) const;

public:
	// Region 基本信息
	std::string m_name;
	std::string m_biomeType;

	// 生成配置
	BaseLayerConfig m_baseLayer;
	std::vector<ColoredLayerConfig> m_coloredLayers;

	// Lua 脚本（预留）
	std::string m_detailScriptPath;
	bool m_detailScriptEnabled = false;

	// 快速查找表（triggerColor -> ColoredLayerConfig）
	std::map<unsigned int, ColoredLayerConfig const*> m_colorToLayerMap;

};