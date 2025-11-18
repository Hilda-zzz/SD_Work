#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <cstdint>

enum class HbTileOrientation : uint8_t 
{
	HORIZONTAL,  // 40×20 (宽×高)
	VERTICAL     // 20×40 (宽×高)
};

struct HbEdgeConstraint 
{
	Rgba8 m_color;           // 边缘标记的颜色
	uint32_t m_colorHash;    // 颜色的哈希值，用于快速比较

	HbEdgeConstraint() : m_color(Rgba8::BLACK), m_colorHash(0) {}

	explicit HbEdgeConstraint(const Rgba8& col)
		: m_color(col)
		, m_colorHash(ComputeHash(col))
	{}

	bool operator==(const HbEdgeConstraint& other) const 
	{
		return m_colorHash == other.m_colorHash;
	}

	bool operator!=(const HbEdgeConstraint& other) const 
	{
		return m_colorHash != other.m_colorHash;
	}

	// 计算颜色的简单哈希
	static uint32_t ComputeHash(const Rgba8& m_color) 
	{
		return (uint32_t(m_color.r) << 24) |
			(uint32_t(m_color.g) << 16) |
			(uint32_t(m_color.b) << 8) |
			uint32_t(m_color.a);
	}
};

// ============================================
// Herringbone Tile 的六条边
// 对应人字形排列的六种边缘匹配方式
// ============================================
enum class HerringboneEdge : uint8_t 
{
	// 横向瓦片 (40×20 内容)
	H_TOP_LEFT = 0,      // 顶部左半边（y最大，左侧）
	H_TOP_RIGHT = 1,     // 顶部右半边（y最大，右侧）
	H_RIGHT = 2,         // 右边（x最大）
	H_BOTTOM_RIGHT = 3,  // 底部右半边（y=0，右侧）
	H_BOTTOM_LEFT = 4,   // 底部左半边（y=0，左侧）
	H_LEFT = 5,          // 左边（x=0）

	// 纵向瓦片 (20×40 内容)
	V_TOP = 0,           // 顶部（y最大）
	V_RIGHT_TOP = 1,     // 右边上半部（x最大，上侧）
	V_RIGHT_BOTTOM = 2,  // 右边下半部（x最大，下侧）
	V_BOTTOM = 3,        // 底部（y=0）
	V_LEFT_BOTTOM = 4,   // 左边下半部（x=0，下侧）
	V_LEFT_TOP = 5,      // 左边上半部（x=0，上侧）

	COUNT
};

// ============================================
// 单个 Herringbone Tile
// ============================================
class HerringboneTile 
{
public:
	HerringboneTile();
	~HerringboneTile();

	// === 初始化 ===
	void Initialize(HbTileOrientation orientation, const IntVec2& contentSize);

	// === 边缘约束 ===
	void SetEdgeConstraint(int edgeIndex, const HbEdgeConstraint& constraint);
	HbEdgeConstraint GetEdgeConstraint(int edgeIndex) const;

	// === 内容像素数据 ===
	void SetContentPixel(int x, int y, const Rgba8& m_color);
	Rgba8 GetContentPixel(int x, int y) const;
	void SetContentData(const std::vector<Rgba8>& pixels);
	const std::vector<Rgba8>& GetContentData() const { return m_contentPixels; }

	// === 属性访问 ===
	HbTileOrientation GetOrientation() const { return m_orientation; }
	IntVec2 GetContentSize() const { return m_contentSize; }
	int GetTileID() const { return m_tileID; }
	void SetTileID(int id) { m_tileID = id; }

	// === 约束匹配检查 ===
	bool MatchesConstraints(const HbEdgeConstraint* constraints, int constraintCount) const;

private:
	int m_tileID;                              // Tile在tileset中的唯一ID
	HbTileOrientation m_orientation;             // 横向或纵向
	IntVec2 m_contentSize;                     // 内容区域大小（不包括黑框）

	// 边缘约束（6条边）
	HbEdgeConstraint m_edges[6];

	// 内容像素数据（行优先存储）
	std::vector<Rgba8> m_contentPixels;        // [y * width + x]
};

