#include "Game/ConfigDrivenCellGenerator.hpp"
#include "Game/RegionGenerationData_v2.hpp"
#include "ThirdParty/Noise/SmoothNoise.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cmath>

// 噪声常量（从 WangTileMap 复制）
constexpr float DEFAULT_OCTAVE_PERSISTANCE = 0.5f;
constexpr float DEFAULT_NOISE_OCTAVE_SCALE = 2.0f;

// ============================================
// 主入口：生成 Chunk 的所有 cells
// ============================================

void ConfigDrivenCellGenerator::GenerateChunkCells(
	CellChunk* chunk,
	const RegionDefinition& regionDef,
	RegionGenerationData const* regionData)
{
	if (!chunk || !regionData) return;

	// === 1. 生成 BaseLayer ===
	GenerateBaseLayer(chunk, regionDef.GetBaseLayer(), regionData);

	// === 2. 生成 ColoredLayers ===
	GenerateColoredLayers(chunk, regionDef.GetColoredLayers(), regionData);
}

// ============================================
// BaseLayer 生成
// ============================================

void ConfigDrivenCellGenerator::GenerateBaseLayer(
	CellChunk* chunk,
	const BaseLayerConfig& config,
	RegionGenerationData const* regionData)
{
	IntVec2 chunkCoords = chunk->GetChunkIndex();

	// 遍历 Chunk 中的每个 cell
	for (int localY = 0; localY < CHUNK_SIZE; ++localY) 
	{
		for (int localX = 0; localX < CHUNK_SIZE; ++localX) 
		{
			int worldX = chunkCoords.x * CHUNK_SIZE + localX;
			int worldY = chunkCoords.y * CHUNK_SIZE + localY;

			// === 1. 计算基础密度 ===
			float baseDensity = CalculateDensity(
				worldX, worldY,
				config.densitySource,
				regionData,
				nullptr  // BaseLayer 不需要 targetColor
			);

			//Cell& cell = chunk->GetLocalCell(localX, localY);

			//if (baseDensity > 0.6f)
			//{
			//	cell.SetToType(CellMatType::MAT_STONE);
			//	cell.m_color = Rgba8::HILDA;
			//}

			// ----------------------------------------------------------------
			// === 2. 应用主噪声 ===
			float advancedDensity = ApplyNoiseEffect(baseDensity, worldX, worldY, config.mainNoise);

			// === 3. Clamp 密度 ===
			advancedDensity = ClampDensity(advancedDensity);

			Cell& cell = chunk->GetLocalCell(localX, localY);

			// === 4. 应用材质层（互斥判断）===
			ApplyMaterialLayers(cell, advancedDensity, config.materialLayers, worldX, worldY);

			// === 5. 应用装饰层（叠加判断）===
			ApplyDecorationLayers(cell, baseDensity, advancedDensity, config.decorationLayers, worldX, worldY);

			// === 6. 应用边缘贴纸 ===
			ApplyBaseEdgeStickers(cell, advancedDensity, config.edgeStickers, worldX, worldY);
		}
	}
}

void ConfigDrivenCellGenerator::ApplyMaterialLayers(
	Cell& cell,
	float density,
	const std::vector<MaterialLayerConfig>& layers,
	int worldX, int worldY)
{
	// 按顺序检查每个材质层（第一个匹配的生效）
	for (auto const& layer : layers) {
		if (density >= layer.densityMin && density <= layer.densityMax) {
			cell.SetToType(layer.material);

			// 如果启用纹理，采样纹理
			if (layer.textureEnabled && !layer.texturePath.empty()) {
				Rgba8 texColor = SampleTexture(layer.texturePath, worldX, worldY);
				cell.m_color = texColor;
			}

			break;  // 只应用第一个匹配的材质层
		}
	}
}

void ConfigDrivenCellGenerator::ApplyDecorationLayers(
	Cell& cell,
	float baseDensity,
	float advancedDensity,
	const std::vector<DecorationConfig>& decorations,
	int worldX, int worldY)
{
	// 装饰层可以叠加，依次检查
	for (auto const& deco : decorations) {
		// 检查基础密度条件
		if (advancedDensity < deco.baseDensityMin || advancedDensity > deco.baseDensityMax) {
			continue;
		}

		// 计算装饰噪声
		float decoNoise = ComputeDecorationNoise(worldX, worldY, baseDensity, deco.decoNoise);

		// 检查阈值
		if (decoNoise > deco.decoNoise.threshold) {
			// 覆盖材质
			cell.SetToType(deco.material);

			// 采样纹理
			if (deco.textureEnabled && !deco.texturePath.empty()) {
				Rgba8 texColor = SampleTexture(deco.texturePath, worldX, worldY);
				cell.m_color = texColor;
			}
		}
	}
}

void ConfigDrivenCellGenerator::ApplyBaseEdgeStickers(
	Cell& cell,
	float density,
	const EdgeStickersConfig& edgeConfig,
	int worldX, int worldY)
{
	// 检查密度条件
	if (density < edgeConfig.applyDensityMin || density > edgeConfig.applyDensityMax) {
		return;
	}

	// 按顺序叠加贴纸（已按 order 排序）
	for (auto const& sticker : edgeConfig.stickers) {
		ApplySticker(cell, sticker, worldX, worldY);
	}
}

// ============================================
// ColoredLayers 生成
// ============================================

void ConfigDrivenCellGenerator::GenerateColoredLayers(
	CellChunk* chunk,
	const std::vector<ColoredLayerConfig>& layers,
	RegionGenerationData const* regionData)
{
	// 为每个彩色层生成 cells
	for (auto const& layerConfig : layers) 
	{
		GenerateColoredLayer(chunk, layerConfig, regionData);
	}
}

void ConfigDrivenCellGenerator::GenerateColoredLayer(
	CellChunk* chunk,
	const ColoredLayerConfig& layerConfig,
	RegionGenerationData const* regionData)
{
	IntVec2 chunkCoords = chunk->GetChunkIndex();

	for (int localY = 0; localY < CHUNK_SIZE; ++localY) 
	{
		for (int localX = 0; localX < CHUNK_SIZE; ++localX) 
		{
			int worldX = chunkCoords.x * CHUNK_SIZE + localX;
			int worldY = chunkCoords.y * CHUNK_SIZE + localY;

			// === 1. 计算彩色密度 ===
			float coloredDensity = CalculateDensity(
				worldX, worldY,
				layerConfig.densitySource,
				regionData,
				&layerConfig.triggerColor
			);

			// === 2. 应用噪声 ===
			float advancedDensity = ApplyNoiseEffect(coloredDensity, worldX, worldY, layerConfig.mainNoise);
			advancedDensity = ClampDensity(advancedDensity);

			// === 3. 应用材质层 ===
			Cell& cell = chunk->GetLocalCell(localX, localY);

			for (auto const& matLayer : layerConfig.materialLayers) 
			{
				if (advancedDensity >= matLayer.densityMin && advancedDensity <= matLayer.densityMax) 
				{
					cell.SetToType(matLayer.material);

					// 采样纹理
					if (!matLayer.texturePath.empty()) {
						Rgba8 texColor = SampleTexture(matLayer.texturePath, worldX, worldY);

						// TODO: 应用 ColorMultiplier（如果有 BaseTextureConfig）

						cell.m_color = texColor;

						if (layerConfig.edgeDetection.enabled)
						{
							ApplyEdgeDetection(cell, advancedDensity, layerConfig.edgeDetection, worldX, worldY);
						}
					}
					else
					{
						CellMatDef const& def = CellMatManager::GetMaterialDef(cell.m_type);
						cell.m_color = Rgba8::GetRandomColorInRange(def.m_colorMin, def.m_colorMax, &Game::s_rng);
					}

					break;
				}
			}
		}
	}
}

void ConfigDrivenCellGenerator::ApplyEdgeDetection(
	Cell& cell,
	float density,
	const EdgeDetectionConfig& edgeConfig,
	int worldX, int worldY)
{
	// 计算动态阈值
	float dynamicThreshold = ComputeDynamicThreshold(
		worldX, worldY,
		density,
		edgeConfig.dynamicThreshold
	);

	// 检查是否在边缘范围内
	if (density < dynamicThreshold) {
		// 按顺序叠加边缘贴纸
		for (auto const& sticker : edgeConfig.edgeStickers) 
		{
			ApplySticker(cell, sticker, worldX, worldY);
		}
	}
}

// ============================================
// 密度计算
// ============================================

float ConfigDrivenCellGenerator::CalculateDensity(
	int worldX, int worldY,
	DensitySourceType sourceType,
	RegionGenerationData const* regionData,
	const Rgba8* targetColor)
{
	if (!regionData) return 0.0f;

	if (sourceType == DensitySourceType::HERRINGBONE_GRAYSCALE) 
	{
		// 从灰度 pixel 插值计算密度
		return regionData->CalculateBaseDensity(worldX, worldY);
	}
	else if (sourceType == DensitySourceType::COLORED_PIXEL) {
		// 从彩色 pixel 颜色匹配计算密度
		if (!targetColor) return 0.0f;

		return regionData->CalculateColoredDensity(worldX, worldY, *targetColor);
	}

	return 0.0f;
}

// ============================================
// 噪声系统
// ============================================

float ConfigDrivenCellGenerator::ApplyNoiseEffect(
	float baseDensity,
	int worldX, int worldY,
	const NoiseConfig& noiseConfig)
{
	float result = baseDensity;
	if (baseDensity > 0.0001f)
	{
		// 计算 Perlin 噪声
		float noise = Compute2dPerlinNoise(
			static_cast<float>(worldX),
			static_cast<float>(worldY),
			noiseConfig.scale,
			noiseConfig.octaves,
			noiseConfig.persistence,
			noiseConfig.octaveScale,
			true,  // renormalize
			noiseConfig.seedOffset
		);

		switch (noiseConfig.effectType) {
		case NoiseEffectType::SUBTRACT_ABS:
			result -= abs(noise) * noiseConfig.effectStrength;
			break;

		case NoiseEffectType::ADD_ABS:
			result += abs(noise) * noiseConfig.effectStrength;
			break;

		case NoiseEffectType::SUBTRACT:
			result -= noise * noiseConfig.effectStrength;
			break;

		case NoiseEffectType::ADD:
			result += noise * noiseConfig.effectStrength;
			break;

		case NoiseEffectType::MULTIPLY:
			result *= (1.0f + noise * noiseConfig.effectStrength);
			break;
		}
	}

	return result;
}

float ConfigDrivenCellGenerator::ComputeDecorationNoise(
	int worldX, int worldY,
	float baseDensity,
	const DecorationNoiseConfig& noiseConfig)
{
	// 计算基础噪声
	float noise = Compute2dPerlinNoise(
		static_cast<float>(worldX),
		static_cast<float>(worldY),
		noiseConfig.scale,
		noiseConfig.octaves,
		DEFAULT_OCTAVE_PERSISTANCE,
		DEFAULT_NOISE_OCTAVE_SCALE,
		true,
		noiseConfig.seedOffset
	);

	// 应用强度调制
	noise = ApplyStrengthModulation(
		noise,
		baseDensity,
		noiseConfig.modulationType,
		noiseConfig.modulationMultiplier
	);

	return noise;
}

float ConfigDrivenCellGenerator::ComputeDynamicThreshold(
	int worldX, int worldY,
	float density,
	const DynamicThresholdConfig& thresholdConfig)
{
	// 计算阈值噪声
	float thresholdNoise = Compute2dPerlinNoise(
		static_cast<float>(worldX),
		static_cast<float>(worldY),
		thresholdConfig.thresholdNoise.scale,
		thresholdConfig.thresholdNoise.octaves,
		DEFAULT_OCTAVE_PERSISTANCE,
		DEFAULT_NOISE_OCTAVE_SCALE,
		true,
		thresholdConfig.thresholdNoise.seedOffset
	);

	// 应用动态阈值公式
	float result = 0.0f;

	switch (thresholdConfig.type) {
	case DynamicThresholdType::NOISED_OFFSET:
		// -density + base + abs(noise) * range
		result = -density + thresholdConfig.baseThreshold
			+ abs(thresholdNoise) * thresholdConfig.thresholdNoise.noiseRange;
		break;

	case DynamicThresholdType::SIMPLE_OFFSET:
		// base + abs(noise) * range
		result = thresholdConfig.baseThreshold
			+ abs(thresholdNoise) * thresholdConfig.thresholdNoise.noiseRange;
		break;

	case DynamicThresholdType::DENSITY_SCALED:
		// base * (1.0 + density) + abs(noise) * range
		result = thresholdConfig.baseThreshold * (1.0f + density)
			+ abs(thresholdNoise) * thresholdConfig.thresholdNoise.noiseRange;
		break;
	}

	return result;
}

// ============================================
// 强度调制
// ============================================

float ConfigDrivenCellGenerator::ApplyStrengthModulation(
	float noise,
	float baseDensity,
	StrengthModulationType type,
	float multiplier)
{
	switch (type) {
	case StrengthModulationType::INVERSE_DENSITY:
		// noise *= (1.0 + (1.0 - density) * multiplier)
		return noise * (1.0f + (1.0f - baseDensity) * multiplier);

	case StrengthModulationType::LINEAR_DENSITY:
		// noise *= (1.0 + density * multiplier)
		return noise * (1.0f + baseDensity * multiplier);

	case StrengthModulationType::CONSTANT:
		// noise *= multiplier
		return noise * multiplier;
	}

	return noise;
}

// ============================================
// 贴纸系统
// ============================================

void ConfigDrivenCellGenerator::ApplySticker(
	Cell& cell,
	const StickerConfig& sticker,
	int worldX, int worldY)
{
	// 采样贴纸纹理
	Rgba8 stickerColor = SampleTexture(
		sticker.path,
		worldX + sticker.offsetX,
		worldY + sticker.offsetY,
		sticker.scale
	);

	// 如果 alpha 为 0，跳过
	if (stickerColor.a == 0) {
		return;
	}

	// 应用色彩调整
	stickerColor = ApplyColorAdjustment(stickerColor, sticker.colorAdjust);

	// 叠加到 cell
	cell.m_color = stickerColor;
}

Rgba8 ConfigDrivenCellGenerator::ApplyColorAdjustment(
	const Rgba8& color,
	const ColorAdjustment& adjustment)
{
	// TODO: 实现 HSV 色彩调整
	// 1. RGB -> HSV
	// 2. 调整 H, S, V
	// 3. HSV -> RGB

	// 暂时返回原色
	return color;
}

// ============================================
// 纹理采样
// ============================================

Rgba8 ConfigDrivenCellGenerator::SampleTexture(
	const std::string& texturePath,
	int worldX, int worldY,
	float scale)
{
	// TODO: 实现纹理缓存系统，避免重复加载

	static std::map<std::string, Image*> textureCache;

	Image* texture = nullptr;

	// 查找缓存
	auto it = textureCache.find(texturePath);
	if (it != textureCache.end()) {
		texture = it->second;
	}
	else {
		// 加载纹理
		texture = new Image(texturePath.c_str());
		textureCache[texturePath] = texture;
	}

	if (!texture) {
		return Rgba8(255, 0, 255, 255);  // 缺失纹理：洋红色
	}

	// 平铺采样
	IntVec2 dims = texture->GetDimensions();
	int sampledX = static_cast<int>(worldX * scale) % dims.x;
	int sampledY = static_cast<int>(worldY * scale) % dims.y;

	// 处理负数
	if (sampledX < 0) sampledX += dims.x;
	if (sampledY < 0) sampledY += dims.y;

	return texture->GetTexelColor(IntVec2(sampledX, sampledY));
}

// ============================================
// 辅助函数
// ============================================

float ConfigDrivenCellGenerator::ClampDensity(float density)
{
	return GetClamped(density, 0.0f, 1.0f);
}

Rgba8 ConfigDrivenCellGenerator::GetPixelColor(int worldX, int worldY, RegionGenerationData_v2 const* regionData)
{
	if (!regionData) return Rgba8::BLACK;

	// TODO: 从 regionData 获取 pixel 颜色
	return Rgba8::BLACK;
}