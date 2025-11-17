#include "WangTileMap.hpp"
#include "CellMatManager.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/SandboxPlayer.hpp"
#include <cmath>
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "ThirdParty/Noise/SmoothNoise.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "SampleImageUtils.hpp"
#include "Nova2D/Nova2DSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"

extern Renderer* g_theRenderer;
extern Nova2DSystem* g_nova2D;
extern InputSystem* g_theInput;
extern Window* g_theWindow;

constexpr float DEFAULT_OCTAVE_PERSISTANCE = 0.5f;
constexpr float DEFAULT_NOISE_OCTAVE_SCALE = 2.0f;

WangTileMap::WangTileMap(SandboxPlayer* player)
	: BaseMap(IntVec2(WANG_MAP_CHUNKS_X* WANG_CHUNK_SIZE, WANG_MAP_CHUNKS_Y* WANG_CHUNK_SIZE), WANG_CHUNK_SIZE)
{
	m_player = player;
	//m_player->SetCurMap(this);
	Initialize();
	InitializeColorMaterialMappings();
	LoadMaterialTextures();
	m_edgeSticker = new Image("Data/Images/edge_soil_lush_1.png");
	m_sand = new Image("Data/Images/moss.png");
	m_sandEdgeSticker = new Image("Data/Images/edge_soil_dead_1.png");

	m_stickerImageA = new Image("Data/Images/edge_earth_rainforest_ver.png");           // RED区域贴画

	LoadTemplateFromFile("Data/Images/XShapeWithColor.png");
	GenerateMapFromTemplate();
}

WangTileMap::~WangTileMap()
{
	BaseMap::~BaseMap();
	if (m_templateImage) {
		delete m_templateImage;
		m_templateImage = nullptr;
	}

	delete m_edgeSticker;

	for (auto& pair : m_materialTextures)
	{
		delete pair.second;
	}
	m_materialTextures.clear();

	delete m_stickerImageA;
}

void WangTileMap::Initialize()
{
	CreateChunkGrid();
}

void WangTileMap::LoadTemplateFromFile(const std::string& filename)
{
	LoadMultiLayerTemplate(filename);
	DetectTemplateEdges();
}

Rgba8 WangTileMap::GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const
{
	return Rgba8();
}

void WangTileMap::DetectTemplateEdges()
{
	for (int y = 0; y < m_templateSize.y; ++y)
	{
		for (int x = 0; x < m_templateSize.x; ++x)
		{
			if (m_templateData[y][x].m_layerType == PixelLayerType::EMPTY) continue;
			PixelLayerType curType = m_templateData[y][x].m_layerType;

			// 检查上方pixel (x, y+1)
			if (y + 1 < m_templateSize.y)
			{
				if (curType != m_templateData[y + 1][x].m_layerType)
				{
					m_templateData[y][x].m_isEdge = true;
					continue;
				}
			}

			// 检查右方pixel (x+1, y)
			if (x + 1 < m_templateSize.x)
			{
				if (curType != m_templateData[y][x+1].m_layerType)
				{
					m_templateData[y][x].m_isEdge = true;
					continue;
				}
			}

			if (y - 1 >= 0)
			{
				if (curType != m_templateData[y - 1][x].m_layerType)
				{
					m_templateData[y][x].m_isEdge = true;
					continue;
				}
			}

			if (x - 1 >= 0)
			{
				if (curType != m_templateData[y][x - 1].m_layerType)
				{
					m_templateData[y][x].m_isEdge = true;
					continue;
				}
			}
		}
	}
}

void WangTileMap::LoadMultiLayerTemplate(const std::string& filename)
{
	m_templateImage = new Image(filename.c_str());
	IntVec2 dimensions = m_templateImage->GetDimensions();
	m_templateSize = dimensions;

	m_templateData.resize(dimensions.y);
	for (int y = 0; y < dimensions.y; ++y) 
	{
		m_templateData[y].resize(dimensions.x);
	}

	for (int y = 0; y < dimensions.y; ++y) 
	{
		for (int x = 0; x < dimensions.x; ++x) 
		{
			Rgba8 pixelColor = m_templateImage->GetTexelColor(IntVec2(x, y));
			TemplatePixel& pixel = m_templateData[y][x];

			// 分类像素类型
			pixel.m_layerType = ClassifyPixel(pixelColor);
			pixel.m_color = pixelColor;

			if (pixel.m_layerType == PixelLayerType::GRAYSCALE||
				pixel.m_layerType == PixelLayerType::EMPTY)
			{
				// 计算灰度密度
				float luminance = (0.299f * pixelColor.r + 0.587f * pixelColor.g + 0.114f * pixelColor.b) / 255.0f;
				pixel.m_density = luminance;
				pixel.m_matType = CellMatType::MAT_STONE;  // 基础层默认石头
			}
			else 
			{
				// 彩色层：找到对应的材质类型
				pixel.m_density = 1.0f;  // 彩色像素视为完全实心
				pixel.m_matType = GetMaterialTypeForColor(pixelColor);
			}
		}
	}
}

void WangTileMap::InitializeColorMaterialMappings()
{
	m_colorMaterialMappings.clear();
	m_colorMaterialMappings.push_back(ColorMaterialMapping(Rgba8(112, 128, 64), CellMatType::MAT_WOOD, 30.0f));
}

PixelLayerType WangTileMap::ClassifyPixel(const Rgba8& color) const
{
	if (IsGrayscale(color, 10.0f)) 
	{
		if (color.r == 0) return PixelLayerType::EMPTY;
		return PixelLayerType::GRAYSCALE;
	}
	return PixelLayerType::COLORED;
}

CellMatType WangTileMap::GetMaterialTypeForColor(const Rgba8& color) const
{
	float minDistance = FLT_MAX;
	CellMatType closestMat = CellMatType::MAT_STONE;  // 默认值

	for (const ColorMaterialMapping& mapping : m_colorMaterialMappings) 
	{
		float distance = ColorDistance(color, mapping.m_color);

		if (distance < mapping.m_colorTolerance && distance < minDistance) 
		{
			minDistance = distance;
			closestMat = mapping.m_matType;
		}
	}

	return closestMat;
}

float WangTileMap::ColorDistance(const Rgba8& c1, const Rgba8& c2) const
{
	int dr = (int)c1.r - (int)c2.r;
	int dg = (int)c1.g - (int)c2.g;
	int db = (int)c1.b - (int)c2.b;
	return std::sqrt((float)(dr * dr + dg * dg + db * db));
}

bool WangTileMap::IsGrayscale(const Rgba8& color, float tolerance) const
{
	float maxDiff = std::max({
		std::abs((int)color.r - (int)color.g),
		std::abs((int)color.g - (int)color.b),
		std::abs((int)color.r - (int)color.b)
		});

	return maxDiff == 0;
}

//float WangTileMap::CalculateBaseDensity(int cellX, int cellY) const
//{
//	float templateX = cellX / static_cast<float>(WANG_CELLS_PER_PIXEL);
//	float templateY = cellY / static_cast<float>(WANG_CELLS_PER_PIXEL);
//
//	int x0 = static_cast<int>(std::floor(templateX));
//	int y0 = static_cast<int>(std::floor(templateY));
//	int x1 = std::min(x0 + 1, m_templateSize.x - 1);
//	int y1 = std::min(y0 + 1, m_templateSize.y - 1);
//
//	x0 = std::clamp(x0, 0, m_templateSize.x - 1);
//	y0 = std::clamp(y0, 0, m_templateSize.y - 1);
//
//	float fx = templateX - x0;
//	float fy = templateY - y0;
//
//	TemplatePixel p00 = GetTemplatePixelAt(x0, y0);
//	TemplatePixel p10 = GetTemplatePixelAt(x1, y0);
//	TemplatePixel p11 = GetTemplatePixelAt(x1, y1);
//	TemplatePixel p01 = GetTemplatePixelAt(x0, y1);
//
//	auto getDensityForBase = [](const TemplatePixel& p) -> float {
//		if (p.m_layerType == PixelLayerType::COLORED) {
//			return 1.0f;  
//		}
//		return p.m_density;
//		};
//
//	float v00 = getDensityForBase(p00);
//	float v10 = getDensityForBase(p10);
//	float v11 = getDensityForBase(p11);
//	float v01 = getDensityForBase(p01);
//
//	float v0 = v00 * (1.0f - fx) + v10 * fx;
//	float v1 = v01 * (1.0f - fx) + v11 * fx;
//	float density = v0 * (1.0f - fy) + v1 * fy;
//
//	return density;
//}

float WangTileMap::CalculateBaseDensity(int cellX, int cellY) const
{
	// 关键修正：让 cell 中心映射到以 pixel 中心为单位的坐标系
	// 使得每个 pixel 的中心 cell 能够正确采样该 pixel
	// 计算公式：(cellX - (CELLS_PER_PIXEL - 1) / 2) / CELLS_PER_PIXEL
	// 这样 cell [7.5] 会映射到 templateX = 0 (Pixel 0 的中心)

	float halfCells = (WANG_CELLS_PER_PIXEL - 1) * 0.5f;  // 对于 16，这是 7.5
	float templateX = (cellX + 0.5f - halfCells) / static_cast<float>(WANG_CELLS_PER_PIXEL);
	float templateY = (cellY + 0.5f - halfCells) / static_cast<float>(WANG_CELLS_PER_PIXEL);

	// 找到周围4个像素
	int x0 = static_cast<int>(std::floor(templateX));
	int y0 = static_cast<int>(std::floor(templateY));
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// 边界检查
	x0 = std::clamp(x0, 0, m_templateSize.x - 1);
	y0 = std::clamp(y0, 0, m_templateSize.y - 1);
	x1 = std::clamp(x1, 0, m_templateSize.x - 1);
	y1 = std::clamp(y1, 0, m_templateSize.y - 1);

	float fx = templateX - std::floor(templateX);
	float fy = templateY - std::floor(templateY);

	// 获取4个像素
	TemplatePixel p00 = GetTemplatePixelAt(x0, y0);
	TemplatePixel p10 = GetTemplatePixelAt(x1, y0);
	TemplatePixel p11 = GetTemplatePixelAt(x1, y1);
	TemplatePixel p01 = GetTemplatePixelAt(x0, y1);

	// 计算密度（彩色像素视为完全实心 = 1.0）
	auto getDensityForBase = [](const TemplatePixel& p) -> float {
		if (p.m_layerType == PixelLayerType::COLORED) {
			return 1.0f;
		}
		return p.m_density;
		};

	float v00 = getDensityForBase(p00);
	float v10 = getDensityForBase(p10);
	float v11 = getDensityForBase(p11);
	float v01 = getDensityForBase(p01);

	// 双线性插值
	float v0 = v00 * (1.0f - fx) + v10 * fx;
	float v1 = v01 * (1.0f - fx) + v11 * fx;
	float density = v0 * (1.0f - fy) + v1 * fy;

	return density;
}

float WangTileMap::CalculateColoredDensity(int cellX, int cellY, const Rgba8& targetColor) const
{
	// 使用与 CalculateBaseDensity 相同的映射逻辑
	// 让 cell 中心映射到以 pixel 中心为单位的坐标系
	float halfCells = (WANG_CELLS_PER_PIXEL - 1) * 0.5f;  // 对于 16，这是 7.5
	float templateX = (cellX + 0.5f - halfCells) / static_cast<float>(WANG_CELLS_PER_PIXEL);
	float templateY = (cellY + 0.5f - halfCells) / static_cast<float>(WANG_CELLS_PER_PIXEL);

	// 找到周围4个像素
	int x0 = static_cast<int>(std::floor(templateX));
	int y0 = static_cast<int>(std::floor(templateY));
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// 边界检查
	x0 = std::clamp(x0, 0, m_templateSize.x - 1);
	y0 = std::clamp(y0, 0, m_templateSize.y - 1);
	x1 = std::clamp(x1, 0, m_templateSize.x - 1);
	y1 = std::clamp(y1, 0, m_templateSize.y - 1);

	// 计算插值参数
	float fx = templateX - std::floor(templateX);
	float fy = templateY - std::floor(templateY);

	// 获取4个像素
	TemplatePixel p00 = GetTemplatePixelAt(x0, y0);
	TemplatePixel p10 = GetTemplatePixelAt(x1, y0);
	TemplatePixel p11 = GetTemplatePixelAt(x1, y1);
	TemplatePixel p01 = GetTemplatePixelAt(x0, y1);

	// 计算密度（只有目标颜色视为实心，黑白灰视为空）
	auto getDensityForColored = [&](const TemplatePixel& p) -> float {
		if (p.m_layerType == PixelLayerType::COLORED) {
			// 检查是否是目标颜色
			float colorDist = ColorDistance(p.m_color, targetColor);
			if (colorDist < 30.0f) {  // 容差范围内
				return 1.0f;
			}
		}
		return 0.0f;  // 黑白灰和其他彩色都视为空
		};

	float v00 = getDensityForColored(p00);
	float v10 = getDensityForColored(p10);
	float v11 = getDensityForColored(p11);
	float v01 = getDensityForColored(p01);

	// 双线性插值
	float v0 = v00 * (1.0f - fx) + v10 * fx;
	float v1 = v01 * (1.0f - fx) + v11 * fx;
	float density = v0 * (1.0f - fy) + v1 * fy;

	return density;
}

TemplatePixel WangTileMap::GetTemplatePixelAt(int templateX, int templateY) const
{
	if (templateX < 0 || templateX >= m_templateSize.x ||
		templateY < 0 || templateY >= m_templateSize.y) 
	{
		return TemplatePixel();  
	}
	return m_templateData[templateY][templateX];
}

void WangTileMap::GenerateBaseLayer()
{
	for (int chunkY = 0; chunkY < m_chunkGridSize.y; ++chunkY) 
	{
		for (int chunkX = 0; chunkX < m_chunkGridSize.x; ++chunkX) 
		{
			CellChunk* chunk = m_chunks[chunkY][chunkX];
			GenerateBaseCellsInChunk(chunk);
		}
	}
}

void WangTileMap::GenerateColoredLayers()
{
	std::vector<Rgba8> uniqueColors = GetUniqueColoredLayers();

	// 为每种颜色生成对应的层
	for (const Rgba8& color : uniqueColors) 
	{
		CellMatType matType = GetMaterialTypeForColor(color);

		for (int chunkY = 0; chunkY < m_chunkGridSize.y; ++chunkY) 
		{
			for (int chunkX = 0; chunkX < m_chunkGridSize.x; ++chunkX) 
			{
				CellChunk* chunk = m_chunks[chunkY][chunkX];
				GenerateColoredCellsInChunk(chunk, color, matType);
			}
		}
	}
}

//void WangTileMap::GenerateBaseCellsInChunk(CellChunk* chunk)
//{
//	IntVec2 chunkCoords = chunk->GetChunkIndex();
//
//	for (int localY = 0; localY < WANG_CHUNK_SIZE; ++localY) 
//	{
//		for (int localX = 0; localX < WANG_CHUNK_SIZE; ++localX) 
//		{
//			int worldX = chunkCoords.x * WANG_CHUNK_SIZE + localX;
//			int worldY = chunkCoords.y * WANG_CHUNK_SIZE + localY;
//
//			float baseDensity = CalculateBaseDensity(worldX, worldY);
//			float advancedDensity = baseDensity;
//			// 添加Perlin噪声
//			if (baseDensity > 0.0001f && baseDensity < 0.999999f) {
//				float noise = Compute2dPerlinNoise(
//					static_cast<float>(worldX),
//					static_cast<float>(worldY),
//					64.0f,      // scale
//					2,          // octaves
//					DEFAULT_OCTAVE_PERSISTANCE,
//					DEFAULT_NOISE_OCTAVE_SCALE,
//					true,
//					0U
//				);
//				advancedDensity -= abs(noise) * 0.3f;
//				advancedDensity = RangeMapClamped(advancedDensity, FloatRange(-1.f, 1.f), FloatRange(0.f, 1.f));
//			}
//
//			Cell& cell = chunk->GetLocalCell(localX, localY);
//			// 根据密度阈值设置cell
//			if (advancedDensity > 0.6f)
//			{
//				cell.SetToType(CellMatType::MAT_STONE);
//				cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);
//				//Decoration noise
//				float decoNoise = Compute2dPerlinNoise(
//					static_cast<float>(worldX),
//					static_cast<float>(worldY),
//					16.0f,      // scale
//					2,          // octaves
//					DEFAULT_OCTAVE_PERSISTANCE,
//					DEFAULT_NOISE_OCTAVE_SCALE,
//					true,
//					0U + 1
//				);
//
//				float densityFactor = 1.0f - baseDensity; // 0.4 到 0.0
//				decoNoise *= (1.0f + densityFactor * 2.0f);
//
//				if (decoNoise > 0.2f)
//				{
//					cell.SetToType(CellMatType::MAT_WOOD);
//					//cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);
//					cell.m_color = Rgba8::CYAN;
//				}
//				if (decoNoise > -0.2f && decoNoise < 0.2f)
//				{
//					cell.m_color = Rgba8::RED;
//				}
//
//				if (advancedDensity < 0.8f)
//				{
//					cell.SetToType(CellMatType::MAT_WOOD);
//					cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);
//
//					Rgba8 texColor2 = SampleImage(m_edgeSticker, m_edgeSticker->GetDimensions().x/2, m_edgeSticker->GetDimensions().y / 2, 1.f);
//					if (texColor2.a != 0)
//					{
//						cell.m_color = texColor2;
//					}
//
//					Rgba8 texColor3 = SampleImage(m_edgeSticker, worldX+10, worldY+10, 0.5f);
//					if (texColor3.a != 0)
//					{
//						cell.m_color = texColor3;
//					}
//
//					Rgba8 texColor1= SampleImage(m_edgeSticker, worldX, worldY, 0.5f);
//					if (texColor1.a!= 0)
//					{
//						cell.m_color = texColor1;
//					}
//				}
//			}
//		}
//	}
//}

void WangTileMap::GenerateBaseCellsInChunk(CellChunk* chunk)
{
	IntVec2 chunkCoords = chunk->GetChunkIndex();
	for (int localY = 0; localY < WANG_CHUNK_SIZE; ++localY)
	{
		for (int localX = 0; localX < WANG_CHUNK_SIZE; ++localX)
		{
			int worldX = chunkCoords.x * WANG_CHUNK_SIZE + localX;
			int worldY = chunkCoords.y * WANG_CHUNK_SIZE + localY;
			float baseDensity = CalculateBaseDensity(worldX, worldY);
			float advancedDensity = baseDensity;

			// 添加Perlin噪声
			if (baseDensity > 0.0001f && baseDensity < 0.999999f) {
				float noise = Compute2dPerlinNoise(
					static_cast<float>(worldX),
					static_cast<float>(worldY),
					64.0f,      // scale
					2,          // octaves
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					0U
				);
				advancedDensity -= abs(noise) * 0.3f;
				advancedDensity = RangeMapClamped(advancedDensity, FloatRange(-1.f, 1.f), FloatRange(0.f, 1.f));
			}

			Cell& cell = chunk->GetLocalCell(localX, localY);

			// 根据密度阈值设置cell
			if (advancedDensity > 0.6f)
			{
				cell.SetToType(CellMatType::MAT_STONE);
				cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);

				// Decoration noise
				float decoNoise = Compute2dPerlinNoise(
					static_cast<float>(worldX),
					static_cast<float>(worldY),
					16.0f,      // scale
					2,          // octaves
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					0U + 1
				);
				float densityFactor = 1.0f - baseDensity; // 0.4 到 0.0
				decoNoise *= (1.0f + densityFactor * 2.0f);

				// RED区域 - 采样imgA作为贴画
// RED区域 - 采样imgA作为贴画
				if (decoNoise > -0.2f && decoNoise < 0.2f)
				{
					if (m_stickerImageA) {
						// 第一次采样 - 原始方向
						Rgba8 stickerColor = SampleImage(
							m_stickerImageA,
							worldX,
							worldY,
							1.0f,                    // tilingMultiplier
							Rotation::ROTATE_0,      // 原始方向
							Symmetry::NONE           // 无对称
						);

						// 只有alpha不为0才贴上去
						if (stickerColor.a != 0) {
							cell.m_color = stickerColor;
						}

						// 第二次采样 - 旋转90度
						Rgba8 stickerColor90 = SampleImage(
							m_stickerImageA,
							worldX,
							worldY,
							1.0f,                    // tilingMultiplier
							Rotation::ROTATE_90,     // 旋转90度
							Symmetry::NONE           // 无对称
						);

						// 只有alpha不为0才贴上去
						if (stickerColor90.a != 0) {
							cell.m_color = stickerColor90;
						}
					}
				}
				// CYAN区域 - 采样imgB作为贴画
				else if (decoNoise > 0.2f)
				{
					cell.SetToType(CellMatType::MAT_WOOD);
					cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);
				}

				// 边缘过渡区域
				if (advancedDensity < 0.8f)
				{
					cell.SetToType(CellMatType::MAT_WOOD);
					cell.m_color = SampleMaterialTexture(cell.m_type, worldX, worldY);

					// 使用m_edgeSticker进行边缘装饰
					if (m_edgeSticker) {
						// 中心采样
						Rgba8 texColor2 = SampleImage(
							m_edgeSticker,
							m_edgeSticker->GetDimensions().x / 2,
							m_edgeSticker->GetDimensions().y / 2,
							1.f,
							Rotation::ROTATE_0,
							Symmetry::NONE
						);
						if (texColor2.a != 0) {
							cell.m_color = texColor2;
						}

						// 偏移采样
						Rgba8 texColor3 = SampleImage(
							m_edgeSticker,
							worldX + 10,
							worldY + 10,
							0.5f,
							Rotation::ROTATE_0,
							Symmetry::NONE
						);
						if (texColor3.a != 0) {
							cell.m_color = texColor3;
						}

						// 世界坐标采样
						Rgba8 texColor1 = SampleImage(
							m_edgeSticker,
							worldX,
							worldY,
							0.5f,
							Rotation::ROTATE_0,
							Symmetry::NONE
						);
						if (texColor1.a != 0) {
							cell.m_color = texColor1;
						}
					}
				}
			}
		}
	}
}

void WangTileMap::GenerateColoredCellsInChunk(CellChunk* chunk, const Rgba8& layerColor, CellMatType matType)
{
	IntVec2 chunkCoords = chunk->GetChunkIndex();

	for (int localY = 0; localY < WANG_CHUNK_SIZE; ++localY) {
		for (int localX = 0; localX < WANG_CHUNK_SIZE; ++localX) {
			int worldX = chunkCoords.x * WANG_CHUNK_SIZE + localX;
			int worldY = chunkCoords.y * WANG_CHUNK_SIZE + localY;

			float coloredDensity = CalculateColoredDensity(worldX, worldY, layerColor);

			// 添加Perlin噪声
			if (coloredDensity > 0.0001f && coloredDensity < 1.f) {
				float noise = Compute2dPerlinNoise(
					static_cast<float>(worldX),
					static_cast<float>(worldY),
					32.0f,      // scale（略小于base layer，制造更多细节）
					2,
					DEFAULT_OCTAVE_PERSISTANCE,
					DEFAULT_NOISE_OCTAVE_SCALE,
					true,
					1U          // 不同的seed
				);
				coloredDensity += abs(noise) * 0.35f;  // 更强的噪声影响
				coloredDensity = RangeMapClamped(coloredDensity, FloatRange(-1.f, 1.f), FloatRange(0.f, 1.f));
			}

			if (coloredDensity > 0.6f)
			{
				Cell& cell = chunk->GetLocalCell(localX, localY);
				cell.SetToType(matType);

				Rgba8 color = SampleImage(m_sand, worldX, worldY);
				color.r = static_cast<unsigned char>(GetClamped(color.r * 0.9f, 0.0f, 255.0f));
				color.g = static_cast<unsigned char>(GetClamped(color.g * 0.7f, 0.0f, 255.0f));
				color.b = static_cast<unsigned char>(GetClamped(color.b * 0.5f, 0.0f, 255.0f));
				cell.m_color = color;

				// === 使用噪声动态调整边界范围 ===

				// 生成边界扰动噪声
				float edgeNoise = Compute2dPerlinNoise(
					static_cast<float>(worldX),
					static_cast<float>(worldY),
					32.0f,  // scale - 控制边界变化的频率
					3,      // octaves - 增加复杂度
					0.5f, 2.0f, true, 100U
				);

				// 计算动态的边界阈值
				// 基础阈值 + 噪声扰动
				float baseThreshold = 1.75f;  // 基础阈值（可以调整这个值）
				float noiseRange = 0.5f;       // 噪声影响范围
				float dynamicThreshold =-coloredDensity+ baseThreshold + abs(edgeNoise) * noiseRange;

				// 现在边界范围是：0.6 到 (0.75 ± 0.2) = 0.6 到 0.55-0.95
				if (coloredDensity < dynamicThreshold)
				{
					// 使用噪声选择变体
					int variantIndex = static_cast<int>(fabsf(edgeNoise * 10.0f)) % 6;

					// 依次叠加多个边缘纹理
					Rgba8 edgeColor1 = SampleImage(m_sandEdgeSticker, worldX, worldY, 0.5f);
					if (edgeColor1.a != 0)
						cell.m_color = edgeColor1;

					Rgba8 edgeColor2 = SampleImage(m_sandEdgeSticker, worldX + 5, worldY+5);
					if (edgeColor2.a != 0)
						cell.m_color = edgeColor2;

					Rgba8 edgeColor3 = SampleImage(m_sandEdgeSticker, worldX + 10, worldY+10);
					if (edgeColor3.a != 0)
						cell.m_color = edgeColor3;

					Rgba8 edgeColor4 = SampleImage(m_sandEdgeSticker, worldX, worldY);
					if (edgeColor4.a != 0)
						cell.m_color = edgeColor4;

					Rgba8 edgeColor5 = SampleImage(m_sandEdgeSticker, worldX + 15, worldY);
					if (edgeColor5.a != 0)
						cell.m_color = edgeColor5;

					Rgba8 edgeColor6 = SampleImage(m_sandEdgeSticker, worldX, worldY, 2.0f);
					if (edgeColor6.a != 0)
						cell.m_color = edgeColor6;
				}
			}
		}
	}
}

std::vector<Rgba8> WangTileMap::GetUniqueColoredLayers() const
{
	std::vector<Rgba8> uniqueColors;

	for (int y = 0; y < m_templateSize.y; ++y) {
		for (int x = 0; x < m_templateSize.x; ++x) {
			const TemplatePixel& pixel = m_templateData[y][x];

			if (pixel.m_layerType == PixelLayerType::COLORED) {
				// 检查是否已经在列表中
				bool found = false;
				for (const Rgba8& existingColor : uniqueColors) {
					if (ColorDistance(pixel.m_color, existingColor) < 30.0f) {
						found = true;
						break;
					}
				}

				if (!found) {
					uniqueColors.push_back(pixel.m_color);
				}
			}
		}
	}

	return uniqueColors;
}

void WangTileMap::LoadMaterialTextures()
{
	Image* stoneTexture = new Image("Data/Images/earth_rainforest.png");
	m_materialTextures[CellMatType::MAT_STONE] = stoneTexture;

	Image* woodTexture = new Image("Data/Images/soil_lush.png");
	m_materialTextures[CellMatType::MAT_WOOD] = woodTexture;
}

Rgba8 WangTileMap::SampleMaterialTexture(CellMatType matType, int worldX, int worldY) const
{
	auto it = m_materialTextures.find(matType);
	if (it == m_materialTextures.end() || it->second == nullptr)
	{
		// 没有找到纹理，返回默认颜色
		switch (matType)
		{
		case CellMatType::MAT_STONE:  return Rgba8::HILDA;
		default:                       return Rgba8::WHITE;
		}
	}

	Image* texture = it->second;

	// 对32x32纹理进行tile采样
	int texX = worldX %texture->GetDimensions().x;  
	int texY = worldY %texture->GetDimensions().y;

	// 处理负坐标
	if (texX < 0) texX += texture->GetDimensions().x;
	if (texY < 0) texY += texture->GetDimensions().y;

	return texture->GetTexelColor(IntVec2(texX, texY));
}

//Rgba8 WangTileMap::SampleImage(Image* img, int worldX, int worldY, float tilingMultiplier) const
//{
//	if (!img) return Rgba8::WHITE;
//
//	IntVec2 texDims = img->GetDimensions();
//
//	// 应用tiling系数
//	// tilingMultiplier > 1.0 : 纹理重复更快（缩小）
//	// tilingMultiplier < 1.0 : 纹理重复更慢（放大）
//	// tilingMultiplier = 1.0 : 默认大小
//
//	float scaledX = worldX * tilingMultiplier;
//	float scaledY = worldY * tilingMultiplier;
//
//	// 转换为整数坐标
//	int texX = static_cast<int>(floorf(scaledX)) % texDims.x;
//	int texY = static_cast<int>(floorf(scaledY)) % texDims.y;
//
//	// 处理负坐标
//	if (texX < 0) texX += texDims.x;
//	if (texY < 0) texY += texDims.y;
//
//	return img->GetTexelColor(IntVec2(texX, texY));
//}

void WangTileMap::RenderDebugUI()
{
	ImGui::Begin("Wang Tile Map Debug");

	ImGui::Checkbox("Show Wang Template", &m_showWangTemplate);
	ImGui::Checkbox("Show Edge Pixel", &m_drawEdges);

	if (m_templateImage)
	{
		ImGui::Text("Template Size: %d x %d pixels", m_templateSize.x, m_templateSize.y);
		ImGui::Text("Cells Per Pixel: %d x %d", WANG_CELLS_PER_PIXEL, WANG_CELLS_PER_PIXEL);
	}

	ImGui::End();
}

void WangTileMap::RenderWangTemplateOverlay() const
{
	if (!m_templateImage) return;

	std::vector<Vertex_PCU> verts;

	for (int templateY = 0; templateY < m_templateSize.y; ++templateY)
	{
		for (int templateX = 0; templateX < m_templateSize.x; ++templateX)
		{
			Rgba8 pixelColor = m_templateImage->GetTexelColor(IntVec2(templateX, templateY));

			float worldX = templateX * WANG_CELLS_PER_PIXEL;
			float worldY = templateY * WANG_CELLS_PER_PIXEL;
			float worldWidth = WANG_CELLS_PER_PIXEL;
			float worldHeight = WANG_CELLS_PER_PIXEL;

			AABB2 bounds(worldX, worldY, worldX + worldWidth, worldY + worldHeight);

			Rgba8 overlayColor = pixelColor;

			AddVertsForAABB2D(verts, bounds, overlayColor);
			if (m_drawEdges&&m_templateData[templateY][templateX].m_isEdge)
			{
				AddVertsForAABB2D(verts, bounds, Rgba8::RED);
			}
		}
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

void WangTileMap::RenderEdgePixelOverlay() const
{
	std::vector<Vertex_PCU> verts;

	for (int templateY = 0; templateY < m_templateSize.y; ++templateY)
	{
		for (int templateX = 0; templateX < m_templateSize.x; ++templateX)
		{
			if (m_templateData[templateY][templateX].m_isEdge)
			{
				float worldX = templateX * WANG_CELLS_PER_PIXEL;
				float worldY = templateY * WANG_CELLS_PER_PIXEL;
				float worldWidth = WANG_CELLS_PER_PIXEL;
				float worldHeight = WANG_CELLS_PER_PIXEL;

				AABB2 bounds(worldX, worldY, worldX + worldWidth, worldY + worldHeight);
				AddVertsForAABB2D(verts, bounds, Rgba8::RED);
			}
		}
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

void WangTileMap::GenerateMapFromTemplate()
{
	GUARANTEE_OR_DIE(!m_templateData.empty(), "Template not loaded!");

	// 第1步：生成基础层（黑白灰）
	GenerateBaseLayer();

	// 第2步：生成彩色层
	GenerateColoredLayers();

	// 第3步：重建顶点
	for (auto& row : m_chunks) 
	{
		for (auto& chunk : row) 
		{
			chunk->RebuildVertexUseSelfColor();
		}
	}
}

void WangTileMap::Update(float deltaTime) 
{
	m_deltaTime = deltaTime;

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)) {
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);

		// 使用全局系统发射粒子
		g_nova2D->EmitBurst(
			mousePosInWorld,
			6000,
			Vec2(0, 100),
			360.0f
		);
	}

	g_nova2D->Update(deltaTime);

	RenderDebugUI();
}

void WangTileMap::Render() const
{
	// === Chunk Grid Lines (Gray) ===

	g_theRenderer->BeginCamera(m_player->m_camera);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	for (const auto& row : m_chunks) 
	{
		for (const auto& chunk : row)
		{
			chunk->RenderChunk();
		}
	}	

	// ==========================================
	if (m_showWangTemplate && m_templateImage)
	{
		RenderWangTemplateOverlay();
	}

	if (m_drawEdges)
	{
		RenderEdgePixelOverlay();
	}

	// ==========================================
	g_nova2D->Render(m_player->m_camera);

	// ==========================================
	std::vector<Vertex_PCU> chunkGridVerts;

	// Vertical lines
	for (int chunkX = 0; chunkX <= m_chunkGridSize.x; ++chunkX) {
		float worldX = static_cast<float>(chunkX * CHUNK_SIZE);
		Vec2 start(worldX, 0.f);
		Vec2 end(worldX, static_cast<float>(m_mapSize.y));
		Rgba8 gridColor(100, 100, 100, 128);
		AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, Rgba8::MAGNETA);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(16.f, 0.f), end + Vec2(16.f, 0.f), 0.5f, gridColor);
		AddVertsForLinSegment2D(chunkGridVerts, start+Vec2(32.f,0.f), end + Vec2(32.f, 0.f), 0.5f, gridColor);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(48.f, 0.f), end + Vec2(48.f, 0.f), 0.5f, gridColor);
	}

	// Horizontal lines
	for (int chunkY = 0; chunkY <= m_chunkGridSize.y; ++chunkY) {
		float worldY = static_cast<float>(chunkY * CHUNK_SIZE);
		Vec2 start(0.f, worldY);
		Vec2 end(static_cast<float>(m_mapSize.x), worldY);
		Rgba8 gridColor(100, 100, 100, 128);
		AddVertsForLinSegment2D(chunkGridVerts, start, end, 0.5f, Rgba8::MAGNETA);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(0.f, 16.f), end + Vec2(0.f, 16.f), 0.5f, gridColor);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(0.f, 32.f), end + Vec2(0.f, 32.f), 0.5f, gridColor);
		AddVertsForLinSegment2D(chunkGridVerts, start + Vec2(0.f, 48.f), end + Vec2(0.f, 48.f), 0.5f, gridColor);
	}

	g_theRenderer->DrawVertexArray(chunkGridVerts);

}