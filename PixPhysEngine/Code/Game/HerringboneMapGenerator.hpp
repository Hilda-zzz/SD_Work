//#pragma once
//#include "HerringboneTileset.hpp"
//#include "Engine/Math/IntVec2.hpp"
//#include <vector>
//#include <map>
//
//constexpr int HB_CELLS_PER_HPIXEL = 16;       // 1 herringbone pixel = 16×16 cells
//constexpr int HB_CELLS_PER_CHUNK = 64;        // 1 chunk = 64×64 cells
//constexpr int HB_HPIXELS_PER_CHUNK = 4;       // 1 chunk = 4×4 herringbone pixels
//
//// Half Tile Size
//constexpr int HB_HALF_TILE_SIZE = 20;         // half tile = 20×20 herringbone pixels
//
//// Tile Size
//constexpr int HB_H_TILE_WIDTH = HB_HALF_TILE_SIZE * 2;        
//constexpr int HB_H_TILE_HEIGHT = HB_HALF_TILE_SIZE;         
//constexpr int HB_V_TILE_WIDTH = HB_HALF_TILE_SIZE;           
//constexpr int HB_V_TILE_HEIGHT = HB_HALF_TILE_SIZE * 2;       
//
//// =========================================================================
//
//// Half Tile 在完整 Tile 中的位置
//enum class HalfTilePositionType : uint8_t
//{
//	// 横向 Tile 的两半
//	H_LEFT,          // 横向tile的左半部分
//	H_RIGHT,         // 横向tile的右半部分
//
//	// 纵向 Tile 的两半
//	V_BOTTOM,        // 纵向tile的下半部分
//	V_TOP,           // 纵向tile的上半部分
//
//	INVALID
//};
//
//// Half Tile 的状态
//struct HalfTileState
//{
//	bool m_isFilled;                          // 是否已被填充
//	HalfTilePositionType m_positionType;              // 该half tile是完整tile的哪个部分
//	int m_fullTileIndex;                      // 所属的完整tile在m_tilePlacements中的索引
//
//	HalfTileState()
//		: m_isFilled(false)
//		, m_positionType(HalfTilePositionType::INVALID)
//		, m_fullTileIndex(-1)
//	{}
//};
//
//// 完整 Tile 的放置信息
//struct HbTilePlacement
//{
//	HbTileOrientation m_orientation;          // HORIZONTAL 或 VERTICAL
//	IntVec2 m_bottomLeftHPixel;               // 左下角坐标（herringbone pixel）
//	IntVec2 m_sizeHPixel;                     // tile尺寸（herringbone pixel）
//	HerringboneTile* m_selectedTile = nullptr;          // 被选中的tile
//
//	// 占据的 half tile 坐标
//	IntVec2 m_halfTileCoords[2];              // 2个half tile的坐标
//
//	HbTilePlacement() : m_selectedTile(nullptr) {}
//
//	IntVec2 GetBottomLeftCell() const 
//	{
//		return m_bottomLeftHPixel * HB_CELLS_PER_HPIXEL;
//	}
//
//	IntVec2 GetBottomLeftChunk() const 
//	{
//		IntVec2 cell = GetBottomLeftCell();
//		return IntVec2(cell.x / HB_CELLS_PER_CHUNK, cell.y / HB_CELLS_PER_CHUNK);
//	}
//};
//
//// 边缘约束信息（用于匹配）
//struct HalfTileConstraints
//{
//	bool m_hasLeft;
//	bool m_hasRight;
//	bool m_hasTop;
//	bool m_hasBottom;
//
//	HbEdgeConstraint m_leftConstraint;
//	HbEdgeConstraint m_rightConstraint;
//	HbEdgeConstraint m_topConstraint;
//	HbEdgeConstraint m_bottomConstraint;
//
//	// 边缘类型（用于判断当前 half tile 类型） 
//	HerringboneEdgeType m_leftEdgeType;   // 左侧约束来自哪条边
//	HerringboneEdgeType m_rightEdgeType;
//	HerringboneEdgeType m_topEdgeType;
//	HerringboneEdgeType m_bottomEdgeType;
//
//	HalfTileConstraints()
//		: m_hasLeft(false), m_hasRight(false), m_hasTop(false), m_hasBottom(false)
//		, m_leftEdgeType(HerringboneEdgeType::COUNT)
//		, m_rightEdgeType(HerringboneEdgeType::COUNT)
//		, m_topEdgeType(HerringboneEdgeType::COUNT)
//		, m_bottomEdgeType(HerringboneEdgeType::COUNT)
//	{}
//};
//
//// =========================================================================
//
//struct HbRegionGenParams
//{
//	IntVec2 m_regionBottomLeftChunk;      // 区块左下角（chunk坐标）
//	IntVec2 m_regionSizeInChunks;         // 区块尺寸（chunk数量）
//	HerringboneTileset* m_tileset;        
//	unsigned int m_randomSeed;            // 随机种子
//};
//
//// 生成器类
//class HerringboneMapGenerator
//{
//public:
//	HerringboneMapGenerator();
//	~HerringboneMapGenerator();
//
//	// === 主流程 ===
//	void InitializeTileGrid(const HbRegionGenParams& params);
//
//	// == = 渲染到外部buffer（避免拷贝） == =
//		// 直接将pixels渲染到提供的buffer中
//		// outPixels: 外部提供的pixel buffer [y][x]
//		// bufferSize: buffer的尺寸（herringbone pixel单位）
//		// sourceOffset: 从生成的区域的哪个位置开始读取（用于跳过边界）
//		void RenderPixelsToBuffer(
//			std::vector<std::vector<Rgba8>>&outPixels,
//			IntVec2 const& bufferSize,
//			IntVec2 const& sourceOffset = IntVec2(0, 0)
//		) const;
//
//	// === 获取生成区域信息 ===
//	IntVec2 GetRegionSizeHPixels() const { return m_regionSizeHPixels; }
//	IntVec2 GetRegionBottomLeftChunk() const { return m_params.m_regionBottomLeftChunk; }
//
//
//	// === 调试输出 ===
//	void PrintGridDebugInfo() const;
//	void PrintHalfTileGrid() const;
//
//	void RenderHPixelGrid() const;
//	void RenderTileBoundaries() const;
//
//private:
//	// === 步骤1: 初始化 half tile 网格 ===
//	void InitializeHalfTileGrid();
//
//	// === 步骤2: 从左下到右上扫描生成 ===
//	void GenerateTiles();
//
//	// === 辅助函数 ===
//
//	// 获取 half tile 坐标对应的网格索引
//	IntVec2 HPixelToHalfTile(const IntVec2& hpixelCoord) const;
//
//	// 获取 half tile 网格坐标对应的 hpixel 坐标（左下角）
//	IntVec2 HalfTileToHPixel(const IntVec2& halfTileCoord) const;
//
//	// 检查 half tile 坐标是否在区域内
//	bool IsHalfTileInBounds(const IntVec2& halfTileCoord) const;
//
//	// 获取 half tile 的约束信息
//	HalfTileConstraints GetConstraintsForHalfTile(const IntVec2& halfTileCoord) const;
//
//	// 根据约束判断该 half tile 应该是哪种类型
//	HalfTilePositionType DetermineHalfTilePosition(const HalfTileConstraints& constraints) const;
//
//	// 放置一个完整的 tile
//	void PlaceTile(const IntVec2& startHalfTile, HalfTilePositionType position);
//
//	// 获取 tile 的约束（用于从 tileset 查找匹配的 tile）
//	void GetTileConstraints(
//		HbTileOrientation orientation,
//		const IntVec2& bottomLeftHalfTile,
//		HbEdgeConstraint constraints[6],  // 改为数组，存储所有 6 条边的约束
//		bool hasConstraint[6]              // 标记哪些边有约束
//	) const;
//
//private:
//	HbRegionGenParams m_params;
//
//	// Half Tile 网格：[halfTileY][halfTileX]
//	std::vector<std::vector<HalfTileState>> m_halfTileGrid;
//	IntVec2 m_halfTileGridSize;             // 网格尺寸（half tile单位）
//
//	// 完整 Tile 列表
//	std::vector<HbTilePlacement> m_tilePlacements;
//
//	// 区块尺寸（herringbone pixel单位）
//	IntVec2 m_regionSizeHPixels;
//};