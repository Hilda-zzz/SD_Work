#pragma once
#include "RegionBounds.hpp"
#include "HerringboneTileset.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include "RegionDefinition.hpp"

// 前向声明
class HerringboneMapGenerator;
class RegionDefinition;

// Half Tile 在完整 Tile 中的位置
enum class HalfTilePositionType : uint8_t
{
    H_LEFT,          // 横向tile的左半部分
    H_RIGHT,         // 横向tile的右半部分
    V_BOTTOM,        // 纵向tile的下半部分
    V_TOP,           // 纵向tile的上半部分
    INVALID
};

// Half Tile 的状态
struct HalfTileState
{
    bool m_isFilled;
    HalfTilePositionType m_positionType;
    int m_fullTileIndex;

    HalfTileState()
        : m_isFilled(false)
        , m_positionType(HalfTilePositionType::INVALID)
        , m_fullTileIndex(-1)
    {}
};

// 完整 Tile 的放置信息
struct HbTilePlacement
{
    HbTileOrientation m_orientation;
    IntVec2 m_bottomLeftHPixel;
    IntVec2 m_sizeHPixel;
    HerringboneTile* m_selectedTile;
    IntVec2 m_halfTileCoords[2];

    HbTilePlacement() : m_selectedTile(nullptr) {}
};

// =========================================================================

// Region生成数据 - 包含所有生成相关的数据
class RegionGenerationData 
{
public:
    RegionGenerationData();
    ~RegionGenerationData();
    
    // === 生成流程 ===
    void Generate(HerringboneMapGenerator* generator);
    
    // === 数据访问 ===
    
    // 根据chunk坐标获取pixel颜色
    Rgba8 GetPixelForChunk(IntVec2 const& chunkCoords, IntVec2 const& localCell) const;
    
    // 检查某个chunk是否属于该Region
    bool ContainsChunk(IntVec2 const& chunkCoords) const;
    
    // === Getters ===
    RegionBounds GetBounds() const { return m_bounds; }
    unsigned int GetSeed() const { return m_regionSeed; }
    HerringboneTileset* GetTileset() const { return m_tileset; }
    
    // 生成数据访问（只读）
    const std::vector<HbTilePlacement>& GetTilePlacements() const { return m_tilePlacements; }
    const std::vector<std::vector<Rgba8>>& GetPixels() const { return m_pixels; }
    IntVec2 GetPixelSize() const { return m_pixelSize; }
    
    // === Setters ===
    void SetBounds(RegionBounds const& bounds) { m_bounds = bounds; }
    void SetSeed(unsigned int seed) { m_regionSeed = seed; }
    void SetTileset(HerringboneTileset* tileset) { m_tileset = tileset; }
    
    // === 调试 ===
    void PrintDebugInfo() const;
    
	// === 密度计算 ===
	float CalculateBaseDensity(int worldX, int worldY) const;
	float CalculateColoredDensity(int worldX, int worldY, const Rgba8& targetColor) const;

	// === Region Definition 访问 ===
	RegionDefinition const& GetRegionDefinition() const { return m_regionDef; }
	void SetRegionDefinition(const RegionDefinition& def) { m_regionDef = def; }

    Rgba8 GetPixelColorAtWorld(int worldX, int worldY) const;

    // === Generator需要访问的数据（友元或公开） ===
    friend class HerringboneMapGenerator;
 
private:
    // === 基本信息 ===
    RegionBounds m_bounds;
    unsigned int m_regionSeed;
    HerringboneTileset* m_tileset;
    RegionDefinition m_regionDef;
    
    // === 生成参数（扩展后的区域） ===
    IntVec2 m_expandedBottomLeftChunk;  // 扩展后的区域左下角
    IntVec2 m_expandedSizeInChunks;      // 扩展后的区域尺寸
    IntVec2 m_regionSizeHPixels;         // 扩展区域的hpixel尺寸
    
    // === 生成中间数据 ===
    
    // Half Tile 网格
    std::vector<std::vector<HalfTileState>> m_halfTileGrid;  // [halfTileY][halfTileX]
    IntVec2 m_halfTileGridSize;
    
    // Tile 放置列表
    std::vector<HbTilePlacement> m_tilePlacements;
    
    // === 最终pixel数据 ===
    std::vector<std::vector<Rgba8>> m_pixels;  // [hpixelY][hpixelX]
    IntVec2 m_pixelSize;  // 最终尺寸（不包括边界）

};
