#include "RegionGenerationData_v2.hpp"
#include "HerringboneMapGenerator_v2.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "SuperChunk.hpp"

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
