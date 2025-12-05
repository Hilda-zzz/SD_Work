#include "Game/RegionDefinition.hpp"
#include "Game/RegionDefinitionLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

RegionDefinition::RegionDefinition()
	: m_name("Untitled")
	, m_biomeType("Unknown")
	, m_detailScriptEnabled(false)
{
}

RegionDefinition::~RegionDefinition()
{
}

bool RegionDefinition::LoadFromXML(const std::string& xmlPath)
{
	// 使用 RegionDefinitionLoader 解析 XML
	RegionDefinitionLoader loader;

	if (!loader.Load(xmlPath, *this)) {
		ERROR_AND_DIE(Stringf("Failed to load region definition from: %s", xmlPath.c_str()));
		return false;
	}

	// 构建颜色查找表
	BuildColorLookupTable();

	DebuggerPrintf("Successfully loaded region definition: %s\n", m_name.c_str());
	return true;
}

bool RegionDefinition::SaveToXML(const std::string& xmlPath) const
{
	// TODO: 实现保存功能（可选）
	UNUSED(xmlPath);
	return false;
}

ColoredLayerConfig const* RegionDefinition::FindColoredLayer(const Rgba8& triggerColor) const
{
	unsigned int key = ColorToKey(triggerColor);
	auto it = m_colorToLayerMap.find(key);

	if (it != m_colorToLayerMap.end()) {
		return it->second;
	}

	return nullptr;
}

bool RegionDefinition::HasColoredLayer(const Rgba8& triggerColor) const
{
	return FindColoredLayer(triggerColor) != nullptr;
}

void RegionDefinition::BuildColorLookupTable()
{
	m_colorToLayerMap.clear();

	for (auto const& layer : m_coloredLayers) {
		unsigned int key = ColorToKey(layer.triggerColor);
		m_colorToLayerMap[key] = &layer;
	}

	DebuggerPrintf("Built color lookup table with %d entries\n",
		(int)m_colorToLayerMap.size());
}

unsigned int RegionDefinition::ColorToKey(const Rgba8& color) const
{
	// 将 RGBA 打包成 32-bit key
	return (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
}

void RegionDefinition::PrintDebugInfo() const
{
	DebuggerPrintf("\n=== Region Definition: %s ===\n", m_name.c_str());
	DebuggerPrintf("Biome Type: %s\n", m_biomeType.c_str());

	// BaseLayer 信息
	DebuggerPrintf("\n[BaseLayer]\n");
	DebuggerPrintf("  Density Source: %s\n",
		m_baseLayer.densitySource == DensitySourceType::HERRINGBONE_GRAYSCALE ?
		"Herringbone Grayscale" : "Colored Pixel");
	DebuggerPrintf("  Material Layers: %d\n", (int)m_baseLayer.materialLayers.size());
	DebuggerPrintf("  Decoration Layers: %d\n", (int)m_baseLayer.decorationLayers.size());

	// ColoredLayers 信息
	DebuggerPrintf("\n[ColoredLayers]\n");
	DebuggerPrintf("  Total Layers: %d\n", (int)m_coloredLayers.size());

	for (auto const& layer : m_coloredLayers) {
		DebuggerPrintf("  - %s: Trigger Color (%d,%d,%d,%d)\n",
			layer.name.c_str(),
			layer.triggerColor.r, layer.triggerColor.g,
			layer.triggerColor.b, layer.triggerColor.a);
	}

	// Lua 脚本
	if (m_detailScriptEnabled) {
		DebuggerPrintf("\n[Detail Script]\n");
		DebuggerPrintf("  Path: %s\n", m_detailScriptPath.c_str());
	}

	DebuggerPrintf("=== End Region Definition ===\n\n");
}