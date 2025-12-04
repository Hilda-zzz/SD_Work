//#include "RegionGenerationData.hpp"
//#include "HerringboneMapGenerator.hpp"
//#include "HerringboneTileset.hpp"
//#include "GameCommon.hpp"
//#include "Engine/Core/ErrorWarningAssert.hpp"
//#include "SuperChunk.hpp"
//#include "../../VerdantLedger/Code/Game/Game.hpp"
//
//RegionGenerationData::RegionGenerationData()
//    : m_regionSeed(0)
//    , m_pixelSize(0, 0)
//    , m_tileset(nullptr)
//{
//}
//
//RegionGenerationData::~RegionGenerationData()
//{
//}
//
//void RegionGenerationData::Generate(HerringboneMapGenerator* generator)
//{
//    if (!generator || !m_tileset) {
//        DebuggerPrintf("ERROR: Cannot generate region - missing generator or tileset\n");
//        return;
//    }
//    
//    // 计算Region的chunk范围
//    IntVec2 bottomLeftChunk(
//        m_bounds.m_bottomLeftSC.x * CHUNKS_PER_SUPER_CHUNK,
//        m_bounds.m_bottomLeftSC.y * CHUNKS_PER_SUPER_CHUNK
//    );
//    
//    IntVec2 regionSizeInSCs = m_bounds.GetSizeInSuperChunks();
//    IntVec2 regionSizeInChunks(
//        regionSizeInSCs.x * CHUNKS_PER_SUPER_CHUNK,
//        regionSizeInSCs.y * CHUNKS_PER_SUPER_CHUNK
//    );
//    
//    DebuggerPrintf("=== Generating Region ===\n");
//    DebuggerPrintf("Bounds: SC[%d,%d] - SC[%d,%d]\n",
//        m_bounds.m_bottomLeftSC.x, m_bounds.m_bottomLeftSC.y,
//        m_bounds.m_topRightSC.x, m_bounds.m_topRightSC.y);
//    DebuggerPrintf("Size: %dx%d SuperChunks (%dx%d chunks)\n",
//        regionSizeInSCs.x, regionSizeInSCs.y,
//        regionSizeInChunks.x, regionSizeInChunks.y);
//    
//    // 扩大生成区域（周围各多1个chunk） - 确保边界连续
//    IntVec2 expandedBottomLeft = bottomLeftChunk - IntVec2(1, 1);
//    IntVec2 expandedSize = regionSizeInChunks + IntVec2(2, 2);
//    
//    // 设置生成参数
//    HbRegionGenParams params;
//    params.m_regionBottomLeftChunk = expandedBottomLeft;
//    params.m_regionSizeInChunks = expandedSize;
//    params.m_tileset = m_tileset;
//    params.m_randomSeed = m_regionSeed;
//    
//    // 生成tiles
//    generator->InitializeTileGrid(params);
//    
//    // 渲染到临时pixel缓冲区（在generator内部）
//    // generator->RenderPixelsFromTiles();  // 如果需要的话
//    
//    // 复制pixel数据（只复制中心区域，跳过边界）
//    m_pixelSize = regionSizeInChunks * HB_HPIXELS_PER_CHUNK;
//    m_pixels.clear();
//    m_pixels.resize(m_pixelSize.y);
//    
//    for (int y = 0; y < m_pixelSize.y; ++y) 
//    {
//        m_pixels[y].resize(m_pixelSize.x);
//        for (int x = 0; x < m_pixelSize.x; ++x) 
//        {
//            // 从扩大的生成区域中读取（offset跳过边界的1个chunk）
//            IntVec2 genPixel(x + HB_HPIXELS_PER_CHUNK, y + HB_HPIXELS_PER_CHUNK);
//            
//            // 从generator获取pixel颜色
//            // 这里需要generator提供GetPixelColor接口
//            // 暂时用占位颜色
//            m_pixels[y][x] = Rgba8::WHITE;  // TODO: 从generator获取
//        }
//    }
//    
//    DebuggerPrintf("Region pixel size: %dx%d hpixels\n", m_pixelSize.x, m_pixelSize.y);
//    DebuggerPrintf("Region generation complete\n\n");
//}
//
//Rgba8 RegionGenerationData::GetPixelForChunk(IntVec2 const& chunkCoords, IntVec2 const& localCell) const
//{
//    // 计算Region的bottomLeft chunk坐标
//    IntVec2 regionBottomLeftChunk(
//        m_bounds.m_bottomLeftSC.x * CHUNKS_PER_SUPER_CHUNK,
//        m_bounds.m_bottomLeftSC.y * CHUNKS_PER_SUPER_CHUNK
//    );
//    
//    // 计算在Region内的相对chunk坐标
//    IntVec2 relativeChunk = chunkCoords - regionBottomLeftChunk;
//    
//    // 转换为cell坐标（在Region内的相对位置）
//    IntVec2 relativeCell = relativeChunk * CHUNK_SIZE + localCell;
//    
//    // 转换为herringbone pixel坐标
//    IntVec2 hpixel(
//        relativeCell.x / HB_CELLS_PER_HPIXEL,
//        relativeCell.y / HB_CELLS_PER_HPIXEL
//    );
//    
//    // 边界检查
//    if (hpixel.x >= 0 && hpixel.x < m_pixelSize.x &&
//        hpixel.y >= 0 && hpixel.y < m_pixelSize.y) {
//        return m_pixels[hpixel.y][hpixel.x];
//    }
//    
//    // 超出范围，返回默认颜色
//    DebuggerPrintf("Warning: GetPixelForChunk out of bounds - hpixel(%d,%d), size(%d,%d)\n",
//        hpixel.x, hpixel.y, m_pixelSize.x, m_pixelSize.y);
//    return Rgba8::BLACK;
//}
//
//bool RegionGenerationData::ContainsChunk(IntVec2 const& chunkCoords) const
//{
//    // 计算chunk对应的SuperChunk坐标
//    IntVec2 scCoords(
//        chunkCoords.x / CHUNKS_PER_SUPER_CHUNK,
//        chunkCoords.y / CHUNKS_PER_SUPER_CHUNK
//    );
//    
//    return m_bounds.ContainsSuperChunk(scCoords);
//}
