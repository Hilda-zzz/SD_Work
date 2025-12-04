//#pragma once
//#include "RegionBounds.hpp"
//#include "Engine/Math/IntVec2.hpp"
//#include "Engine/Core/Rgba8.hpp"
//#include <vector>
//
//class HerringboneMapGenerator;
//class HerringboneTileset;
//
//// Region 生成数据
//class RegionGenerationData 
//{
//public:
//    RegionGenerationData();
//    ~RegionGenerationData();
//    
//    // 生成该Region的pixel数据
//    void Generate(HerringboneMapGenerator* generator);
//    
//    // 根据chunk坐标获取pixel颜色
//    Rgba8 GetPixelForChunk(IntVec2 const& chunkCoords, IntVec2 const& localCell) const;
//    
//    // 检查某个chunk是否属于该Region
//    bool ContainsChunk(IntVec2 const& chunkCoords) const;
//    
//    // Getters
//    RegionBounds GetBounds() const { return m_bounds; }
//    unsigned int GetSeed() const { return m_regionSeed; }
//    
//    // Setters
//    void SetBounds(RegionBounds const& bounds) { m_bounds = bounds; }
//    void SetSeed(unsigned int seed) { m_regionSeed = seed; }
//    void SetTileset(HerringboneTileset* tileset) { m_tileset = tileset; }
//    
//private:
//    RegionBounds m_bounds;          // Region边界（SuperChunk坐标）
//    unsigned int m_regionSeed;       // Region种子
//    
//    // 存储生成的pixel数据
//    std::vector<std::vector<Rgba8>> m_pixels;  // [hpixelY][hpixelX]
//    IntVec2 m_pixelSize;             // 以herringbone pixel为单位
//    
//    // Biome信息
//    HerringboneTileset* m_tileset;  // 使用的tileset
//};
