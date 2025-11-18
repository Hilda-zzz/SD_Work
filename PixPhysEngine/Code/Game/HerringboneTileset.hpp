#pragma once
#include "HerringboneTile.hpp"
#include <vector>

struct TileRect
{
	IntVec2 bottomLeft;         // 黑框左上角
	IntVec2 size;            // 黑框大小（包含黑框）
	HbTileOrientation orientation;
};

class HerringboneTileset 
{
public:
	HerringboneTileset();
	~HerringboneTileset();

	// === 从文件加载 ===
	bool LoadFromImage(const char* filename);

	// === Tile访问 ===
	int GetTileCount() const { return static_cast<int>(m_tiles.size()); }
	HerringboneTile* GetTile(int index);
	const HerringboneTile* GetTile(int index) const;

	// === 按约束查找 ===
	// 获取满足给定边缘约束的所有tiles
	std::vector<HerringboneTile*> FindTilesByConstraints(
		HbTileOrientation orientation,
		const HbEdgeConstraint& constraint1,
		const HbEdgeConstraint& constraint2,
		const HbEdgeConstraint& constraint3
	);

	// === 调试信息 ===
	void PrintDebugInfo() const;

private:
	// 步骤1: 检测所有tile的黑框位置
	std::vector<TileRect> DetectTileRects(const class Image* image);

	bool ValidateTileRect(const Image* image, const IntVec2& bottomLeft, const IntVec2& size);

	// 步骤2: 从单个tile矩形中提取边缘约束
	void ExtractEdgeConstraints(
		const class Image* image,
		const TileRect& rect,
		HbEdgeConstraint edges[6]
	);

	// 步骤3: 提取tile的内容像素
	void ExtractTileContent(
		const class Image* image,
		const TileRect& rect,
		HerringboneTile* tile
	);

	// 辅助：读取边缘标记点（5个连续像素）
	HbEdgeConstraint ReadEdgeMarker(
		const class Image* image,
		const IntVec2& startPos,
		const IntVec2& direction
	);

	// 辅助：检查像素是否为黑色边框
	bool IsBlackBorder(const Rgba8& m_color) const;

private:
	std::vector<HerringboneTile*> m_tiles;     // 所有tiles

	// 快速查找表：[orientation][constraint1][constraint2][constraint3] -> tile indices
	// 暂时使用简单的线性搜索，后续可优化
};