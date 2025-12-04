#pragma once
#include "HerringboneTileset.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

constexpr int HB_CELLS_PER_HPIXEL = 16;       // 1 herringbone pixel = 16×16 cells
constexpr int HB_CELLS_PER_CHUNK = 64;        // 1 chunk = 64×64 cells
constexpr int HB_HPIXELS_PER_CHUNK = 4;       // 1 chunk = 4×4 herringbone pixels
// Half Tile Size
constexpr int HB_HALF_TILE_SIZE = 20;         // half tile = 20×20 herringbone pixels

// Tile Size
constexpr int HB_H_TILE_WIDTH = HB_HALF_TILE_SIZE * 2;        
constexpr int HB_H_TILE_HEIGHT = HB_HALF_TILE_SIZE;         
constexpr int HB_V_TILE_WIDTH = HB_HALF_TILE_SIZE;           
constexpr int HB_V_TILE_HEIGHT = HB_HALF_TILE_SIZE * 2;       


// 前向声明
class RegionGenerationData;
struct HbTilePlacement;
struct HalfTileState;
enum class HalfTilePositionType : uint8_t;

// 边缘约束信息（用于匹配）
struct HalfTileConstraints
{
    bool m_hasLeft;
    bool m_hasRight;
    bool m_hasTop;
    bool m_hasBottom;

    HbEdgeConstraint m_leftConstraint;
    HbEdgeConstraint m_rightConstraint;
    HbEdgeConstraint m_topConstraint;
    HbEdgeConstraint m_bottomConstraint;

    HerringboneEdgeType m_leftEdgeType;
    HerringboneEdgeType m_rightEdgeType;
    HerringboneEdgeType m_topEdgeType;
    HerringboneEdgeType m_bottomEdgeType;

    HalfTileConstraints()
        : m_hasLeft(false), m_hasRight(false), m_hasTop(false), m_hasBottom(false)
        , m_leftEdgeType(HerringboneEdgeType::COUNT)
        , m_rightEdgeType(HerringboneEdgeType::COUNT)
        , m_topEdgeType(HerringboneEdgeType::COUNT)
        , m_bottomEdgeType(HerringboneEdgeType::COUNT)
    {}
};

// =========================================================================

// Herringbone Map Generator - 无状态的生成工具
// 所有数据都存储在RegionGenerationData中
class HerringboneMapGenerator
{
public:
    HerringboneMapGenerator();
    ~HerringboneMapGenerator();

    // === 主流程 ===
    // 在提供的RegionData中生成所有数据
    void Generate(RegionGenerationData* regionData);

private:
    // === 生成步骤 ===
    
    // 步骤1: 初始化生成参数
    void InitializeGenerationParams(RegionGenerationData* regionData);
    
    // 步骤2: 初始化half tile网格
    void InitializeHalfTileGrid(RegionGenerationData* regionData);
    
    // 步骤3: 生成tiles
    void GenerateTiles(RegionGenerationData* regionData);
    
    // 步骤4: 渲染pixels到buffer
    void RenderPixelsToBuffer(RegionGenerationData* regionData);

    // === 辅助函数（纯函数，不依赖成员变量） ===
    
    // 坐标转换
    static IntVec2 HPixelToHalfTile(IntVec2 const& hpixelCoord);
    static IntVec2 HalfTileToHPixel(IntVec2 const& halfTileCoord);
    
    // Half tile约束检测
    HalfTileConstraints GetConstraintsForHalfTile(
        RegionGenerationData* regionData,
        IntVec2 const& halfTileCoord) const;
    
    // 判断half tile类型
    HalfTilePositionType DetermineHalfTilePosition(
        HalfTileConstraints const& constraints) const;
    
    // 放置tile
    void PlaceTile(
        RegionGenerationData* regionData,
        IntVec2 const& startHalfTile,
        HalfTilePositionType position);
    
    // 获取tile约束
    void GetTileConstraints(
        RegionGenerationData* regionData,
        HbTileOrientation orientation,
        IntVec2 const& bottomLeftHalfTile,
        HbEdgeConstraint constraints[6],
        bool hasConstraint[6]) const;

    bool IsHalfTileInBounds(RegionGenerationData* regionData, const IntVec2& halfTileCoord) const;
};
