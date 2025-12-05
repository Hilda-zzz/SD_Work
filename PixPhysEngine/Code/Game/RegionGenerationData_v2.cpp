#include "RegionGenerationData_v2.hpp"
#include "HerringboneMapGenerator_v2.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "SuperChunk.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "CellChunk.hpp"

RegionGenerationData::RegionGenerationData()
    : m_regionSeed(0)
    , m_tileset(nullptr)
    , m_expandedBottomLeftChunk(0, 0)
    , m_expandedSizeInChunks(0, 0)
    , m_regionSizeHPixels(0, 0)
    , m_halfTileGridSize(0, 0)
    , m_pixelSize(0, 0)
{
}

RegionGenerationData::~RegionGenerationData()
{
}

void RegionGenerationData::Generate(HerringboneMapGenerator* generator)
{
    if (!generator || !m_tileset) {
        DebuggerPrintf("ERROR: Cannot generate region - missing generator or tileset\n");
        return;
    }
    
    DebuggerPrintf("\n=== Generating Region ===\n");
    DebuggerPrintf("Bounds: SC[%d,%d] - SC[%d,%d]\n",
        m_bounds.m_bottomLeftSC.x, m_bounds.m_bottomLeftSC.y,
        m_bounds.m_topRightSC.x, m_bounds.m_topRightSC.y);
    DebuggerPrintf("Seed: %u\n", m_regionSeed);
    
    // 清空之前的数据
    m_halfTileGrid.clear();
    m_tilePlacements.clear();
    m_pixels.clear();
    
    // ⭐ Generator在这个RegionData中生成所有数据
    generator->Generate(this);
    
    DebuggerPrintf("Generation complete:\n");
    DebuggerPrintf("  - Tiles placed: %d\n", static_cast<int>(m_tilePlacements.size()));
    DebuggerPrintf("  - Pixel size: %dx%d\n", m_pixelSize.x, m_pixelSize.y);
    DebuggerPrintf("  - Half tile grid: %dx%d\n", m_halfTileGridSize.x, m_halfTileGridSize.y);
    DebuggerPrintf("\n");
}

Rgba8 RegionGenerationData::GetPixelForChunk(IntVec2 const& globalChunkCoords,
	IntVec2 const& localCellCoords) const
{
	// 计算该chunk在Region中的相对位置
	IntVec2 chunkOffset(
		globalChunkCoords.x - m_expandedBottomLeftChunk.x,
		globalChunkCoords.y - m_expandedBottomLeftChunk.y
	);

	// 检查chunk是否在Region范围内
	if (chunkOffset.x < 0 || chunkOffset.x >= m_expandedSizeInChunks.x ||
		chunkOffset.y < 0 || chunkOffset.y >= m_expandedSizeInChunks.y)
	{
		return Rgba8::BLACK;  // 超出范围返回黑色
	}

	// 计算herringbone pixel坐标（直接使用，不需要处理overlap）
	int hpixelX = chunkOffset.x * HB_HPIXELS_PER_CHUNK + localCellCoords.x / HB_CELLS_PER_HPIXEL;
	int hpixelY = chunkOffset.y * HB_HPIXELS_PER_CHUNK + localCellCoords.y / HB_CELLS_PER_HPIXEL;

	// 边界检查
	if (hpixelX < 0 || hpixelX >= m_regionSizeHPixels.x ||
		hpixelY < 0 || hpixelY >= m_regionSizeHPixels.y)
	{
		return Rgba8::BLACK;
	}

	return m_pixels[hpixelY][hpixelX];
}

bool RegionGenerationData::ContainsChunk(IntVec2 const& chunkCoords) const
{
    IntVec2 scCoords(
        chunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
        chunkCoords.y / CHUNKS_PER_SUPER_CHUNK
    );
    
    return m_bounds.ContainsSuperChunk(scCoords);
}

void RegionGenerationData::PrintDebugInfo() const
{
    DebuggerPrintf("\n=== Region Debug Info ===\n");
    DebuggerPrintf("Bounds: SC[%d,%d] - SC[%d,%d]\n",
        m_bounds.m_bottomLeftSC.x, m_bounds.m_bottomLeftSC.y,
        m_bounds.m_topRightSC.x, m_bounds.m_topRightSC.y);
    DebuggerPrintf("Seed: %u\n", m_regionSeed);
    DebuggerPrintf("Expanded area: Chunk[%d,%d], Size: %dx%d chunks\n",
        m_expandedBottomLeftChunk.x, m_expandedBottomLeftChunk.y,
        m_expandedSizeInChunks.x, m_expandedSizeInChunks.y);
    DebuggerPrintf("Region size (hpixels): %dx%d\n",
        m_regionSizeHPixels.x, m_regionSizeHPixels.y);
    DebuggerPrintf("Half tile grid: %dx%d\n",
        m_halfTileGridSize.x, m_halfTileGridSize.y);
    DebuggerPrintf("Tiles placed: %d\n", static_cast<int>(m_tilePlacements.size()));
    DebuggerPrintf("Final pixel size: %dx%d\n", m_pixelSize.x, m_pixelSize.y);
    DebuggerPrintf("\n");
}

float RegionGenerationData::CalculateBaseDensity(int worldX, int worldY) const
{
	// ========================================
	// 步骤 1: 世界坐标 → Chunk 坐标 + 局部坐标
	// ========================================
	int chunkX = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX - CHUNK_SIZE + 1) / CHUNK_SIZE);
	int chunkY = (worldY >= 0) ? (worldY / CHUNK_SIZE) : ((worldY - CHUNK_SIZE + 1) / CHUNK_SIZE);

	int localX = worldX - (chunkX * CHUNK_SIZE);
	int localY = worldY - (chunkY * CHUNK_SIZE);

	// ========================================
	// 步骤 2: 验证 Chunk 在 Region 范围内
	// ========================================
	IntVec2 chunkOffset(
		chunkX - m_expandedBottomLeftChunk.x,
		chunkY - m_expandedBottomLeftChunk.y
	);

	if (chunkOffset.x < 0 || chunkOffset.x >= m_expandedSizeInChunks.x ||
		chunkOffset.y < 0 || chunkOffset.y >= m_expandedSizeInChunks.y)
	{
		return 1.0f;  // ⭐ 超出 Region 范围，返回 1.0（白色/实心）
	}

	// ========================================
	// 步骤 3: 计算 Cell 在 Region 内的绝对坐标
	// ========================================
	int regionCellX = chunkOffset.x * CHUNK_SIZE + localX;
	int regionCellY = chunkOffset.y * CHUNK_SIZE + localY;

	// ========================================
	// 步骤 4: Cell 中心对齐到 Pixel 中心的坐标映射
	// ========================================
	float halfCells = (HB_CELLS_PER_HPIXEL - 1) * 0.5f;  // 对于 16，这是 7.5
	float hpixelX_f = (regionCellX + 0.5f - halfCells) / static_cast<float>(HB_CELLS_PER_HPIXEL);
	float hpixelY_f = (regionCellY + 0.5f - halfCells) / static_cast<float>(HB_CELLS_PER_HPIXEL);

	// ========================================
	// 步骤 5: 找到周围 4 个 Pixel 的索引
	// ========================================
	int x0 = static_cast<int>(std::floor(hpixelX_f));
	int y0 = static_cast<int>(std::floor(hpixelY_f));
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// ⭐ 不进行边界裁剪，而是在取 pixel 时判断

	// ========================================
	// 步骤 6: 计算插值参数
	// ========================================
	float fx = hpixelX_f - std::floor(hpixelX_f);
	float fy = hpixelY_f - std::floor(hpixelY_f);

	// ========================================
	// 步骤 7: 获取 4 个 Pixel 的颜色（带边界处理）
	// ========================================
	auto GetPixelSafe = [this](int x, int y) -> Rgba8 {
		// ⭐ 边界外返回白色（255, 255, 255, 255）
		if (x < 0 || x >= m_regionSizeHPixels.x ||
			y < 0 || y >= m_regionSizeHPixels.y)
		{
			return Rgba8(255, 255, 255, 255);  // 白色 = density 1.0
		}
		return m_pixels[y][x];
		};

	Rgba8 c00 = GetPixelSafe(x0, y0);  // 左下
	Rgba8 c10 = GetPixelSafe(x1, y0);  // 右下
	Rgba8 c01 = GetPixelSafe(x0, y1);  // 左上
	Rgba8 c11 = GetPixelSafe(x1, y1);  // 右上

	// ========================================
	// 步骤 8: 转换为灰度密度
	// ========================================
	auto ToGrayscaleDensity = [](const Rgba8& color) -> float {
		// ⭐ 检查是否为灰度（r == g == b）
		if (color.r == color.g && color.g == color.b)
		{
			// 灰度像素：使用亮度作为密度
			return NormalizeByte(color.r);
		}
		else
		{
			// ⭐ 彩色像素：density = 0.0（用于 ColoredLayer）
			return 0.0f;
		}
		};

	float v00 = ToGrayscaleDensity(c00);
	float v10 = ToGrayscaleDensity(c10);
	float v01 = ToGrayscaleDensity(c01);
	float v11 = ToGrayscaleDensity(c11);

	// ========================================
	// 步骤 9: 双线性插值
	// ========================================
	float v0 = v00 * (1.0f - fx) + v10 * fx;  // 下边
	float v1 = v01 * (1.0f - fx) + v11 * fx;  // 上边
	float density = v0 * (1.0f - fy) + v1 * fy;

	return density;
}

float RegionGenerationData::CalculateColoredDensity(int worldX, int worldY, const Rgba8& targetColor) const
{
	// ========================================
	// 步骤 1: 世界坐标 → Chunk 坐标 + 局部坐标
	// ========================================
	int chunkX = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX - CHUNK_SIZE + 1) / CHUNK_SIZE);
	int chunkY = (worldY >= 0) ? (worldY / CHUNK_SIZE) : ((worldY - CHUNK_SIZE + 1) / CHUNK_SIZE);

	int localX = worldX - (chunkX * CHUNK_SIZE);
	int localY = worldY - (chunkY * CHUNK_SIZE);

	// ========================================
	// 步骤 2: 验证 Chunk 在 Region 范围内
	// ========================================
	IntVec2 chunkOffset(
		chunkX - m_expandedBottomLeftChunk.x,
		chunkY - m_expandedBottomLeftChunk.y
	);

	if (chunkOffset.x < 0 || chunkOffset.x >= m_expandedSizeInChunks.x ||
		chunkOffset.y < 0 || chunkOffset.y >= m_expandedSizeInChunks.y)
	{
		return 0.0f;  // ⭐ 超出 Region 范围，返回 0.0（ColoredLayer 边界外无内容）
	}

	// ========================================
	// 步骤 3: 计算 Cell 在 Region 内的绝对坐标
	// ========================================
	int regionCellX = chunkOffset.x * CHUNK_SIZE + localX;
	int regionCellY = chunkOffset.y * CHUNK_SIZE + localY;

	// ========================================
	// 步骤 4: Cell 中心对齐到 Pixel 中心的坐标映射
	// ========================================
	float halfCells = (HB_CELLS_PER_HPIXEL - 1) * 0.5f;  // 对于 16，这是 7.5
	float hpixelX_f = (regionCellX + 0.5f - halfCells) / static_cast<float>(HB_CELLS_PER_HPIXEL);
	float hpixelY_f = (regionCellY + 0.5f - halfCells) / static_cast<float>(HB_CELLS_PER_HPIXEL);

	// ========================================
	// 步骤 5: 找到周围 4 个 Pixel 的索引
	// ========================================
	int x0 = static_cast<int>(std::floor(hpixelX_f));
	int y0 = static_cast<int>(std::floor(hpixelY_f));
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// ========================================
	// 步骤 6: 计算插值参数
	// ========================================
	float fx = hpixelX_f - std::floor(hpixelX_f);
	float fy = hpixelY_f - std::floor(hpixelY_f);

	// ========================================
	// 步骤 7: 获取 4 个 Pixel 的颜色（带边界处理）
	// ========================================
	auto GetPixelSafe = [this](int x, int y) -> Rgba8 {
		// ⭐ 边界外返回黑色（0, 0, 0, 255） - 表示无 ColoredLayer
		if (x < 0 || x >= m_regionSizeHPixels.x ||
			y < 0 || y >= m_regionSizeHPixels.y)
		{
			return Rgba8(0, 0, 0, 255);  // 黑色 = density 0.0
		}
		return m_pixels[y][x];
		};

	Rgba8 c00 = GetPixelSafe(x0, y0);  // 左下
	Rgba8 c10 = GetPixelSafe(x1, y0);  // 右下
	Rgba8 c01 = GetPixelSafe(x0, y1);  // 左上
	Rgba8 c11 = GetPixelSafe(x1, y1);  // 右上

	// ========================================
	// 步骤 8: 计算颜色匹配密度
	// ========================================
	auto ColorMatchDensity = [&targetColor](const Rgba8& pixelColor) -> float {
		// ⭐ 只有完全匹配目标颜色才返回 1.0
		if (pixelColor.r == targetColor.r &&
			pixelColor.g == targetColor.g &&
			pixelColor.b == targetColor.b)
		{
			return 1.0f;  // 完全匹配
		}
		else
		{
			return 0.0f;  // 不匹配
		}
		};

	float v00 = ColorMatchDensity(c00);
	float v10 = ColorMatchDensity(c10);
	float v01 = ColorMatchDensity(c01);
	float v11 = ColorMatchDensity(c11);

	// ========================================
	// 步骤 9: 双线性插值
	// ========================================
	float v0 = v00 * (1.0f - fx) + v10 * fx;  // 下边
	float v1 = v01 * (1.0f - fx) + v11 * fx;  // 上边
	float density = v0 * (1.0f - fy) + v1 * fy;

	return density;
}

Rgba8 RegionGenerationData::GetPixelColorAtWorld(int worldX, int worldY) const
{
	    // 将世界坐标转换为 Chunk 坐标和局部坐标
    int chunkX = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX - CHUNK_SIZE + 1) / CHUNK_SIZE);
    int chunkY = (worldY >= 0) ? (worldY / CHUNK_SIZE) : ((worldY - CHUNK_SIZE + 1) / CHUNK_SIZE);
    
    int localX = worldX - (chunkX * CHUNK_SIZE);
    int localY = worldY - (chunkY * CHUNK_SIZE);
    
    // 复用 GetPixelForChunk
    return GetPixelForChunk(IntVec2(chunkX, chunkY), IntVec2(localX, localY));
}
