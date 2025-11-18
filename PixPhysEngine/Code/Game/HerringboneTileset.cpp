#include "HerringboneTileset.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

HerringboneTileset::HerringboneTileset() 
{
}

HerringboneTileset::~HerringboneTileset() 
{
	for (auto* tile : m_tiles) 
	{
		delete tile;
	}
	m_tiles.clear();
}

bool HerringboneTileset::LoadFromImage(const char* filename) 
{
	// 步骤1: 加载图像
	Image* image = new Image(filename);
	if (!image) 
	{
		DebuggerPrintf("Failed to load tileset image: %s\n", filename);
		return false;
	}

	IntVec2 imageDims = image->GetDimensions();
	DebuggerPrintf("Loading Herringbone Tileset from: %s (%d×%d)\n",
		filename, imageDims.x, imageDims.y);

	// 步骤2: 检测所有tile的黑框位置
	std::vector<TileRect> tileRects = DetectTileRects(image);
	DebuggerPrintf("Detected %d tiles in tileset\n", static_cast<int>(tileRects.size()));

	// 步骤3: 为每个检测到的tile创建对象
	for (size_t i = 0; i < tileRects.size(); ++i) 
	{
		const TileRect& rect = tileRects[i];

		HerringboneTile* tile = new HerringboneTile();
		tile->SetTileID(static_cast<int>(i));

		// 根据方向确定内容大小
		IntVec2 contentSize;
		if (rect.orientation == HbTileOrientation::HORIZONTAL)
		{
			contentSize = IntVec2(40, 20);
		}
		else 
		{
			contentSize = IntVec2(20, 40);
		}

		tile->Initialize(rect.orientation, contentSize);

		// 提取边缘约束
		HbEdgeConstraint edges[6];
		ExtractEdgeConstraints(image, rect, edges);
		for (int e = 0; e < 6; ++e) 
		{
			tile->SetEdgeConstraint(e, edges[e]);
		}

		// 提取内容像素
		ExtractTileContent(image, rect, tile);

		m_tiles.push_back(tile);
	}

	delete image;

	DebuggerPrintf("Successfully loaded %d tiles\n", GetTileCount());
	return true;
}

// ============================================
// Tile矩形检测
// ============================================

std::vector<TileRect> HerringboneTileset::DetectTileRects(const Image* image) 
{
	std::vector<TileRect> results;
	IntVec2 imageDims = image->GetDimensions();

	std::vector<std::vector<bool>> visited(imageDims.y, std::vector<bool>(imageDims.x, false));

	// 扫描寻找可能的左上角（彩色像素，且周围有黑框特征）
	for (int y = 0; y < imageDims.y - 20; ++y) 
	{
		for (int x = 0; x < imageDims.x - 20; ++x) 
		{
			if (visited[y][x]) continue;

			Rgba8 cornerColor = image->GetTexelColor(IntVec2(x, y));

			// 左上角应该是彩色的（不是黑色，不是白色）
			if (IsBlackBorder(cornerColor) || cornerColor == Rgba8::WHITE) 
			{
				continue;
			}

			// 检查右侧第2个像素是否是黑色（边框的一部分）
			Rgba8 rightPixel = image->GetTexelColor(IntVec2(x + 1, y));
			if (!IsBlackBorder(rightPixel)) continue;

			// 检查下方第2个像素是否是黑色
			Rgba8 bottomPixel = image->GetTexelColor(IntVec2(x, y + 1));
			if (!IsBlackBorder(bottomPixel)) continue;

			// 尝试识别这是横向还是纵向tile
			// 横向: 宽度约42, 高度约22
			// 纵向: 宽度约22, 高度约42

			// 先尝试横向 (42×22)
			if (x + 41 < imageDims.x && y + 21 < imageDims.y) 
			{
				bool isHorizontal = ValidateTileRect(image, IntVec2(x, y), IntVec2(42, 22));
				if (isHorizontal) 
				{
					TileRect rect;
					rect.bottomLeft = IntVec2(x, y);
					rect.size = IntVec2(42, 22);
					rect.orientation = HbTileOrientation::HORIZONTAL;
					results.push_back(rect);

					// 标记区域为已访问
					for (int dy = 0; dy < 22; ++dy) 
					{
						for (int dx = 0; dx < 42; ++dx) 
						{
							visited[y + dy][x + dx] = true;
						}
					}
					continue;
				}
			}

			// 再尝试纵向 (22×42)
			if (x + 21 < imageDims.x && y + 41 < imageDims.y) 
			{
				bool isVertical = ValidateTileRect(image, IntVec2(x, y), IntVec2(22, 42));
				if (isVertical) 
				{
					TileRect rect;
					rect.bottomLeft = IntVec2(x, y);
					rect.size = IntVec2(22, 42);
					rect.orientation = HbTileOrientation::VERTICAL;
					results.push_back(rect);

					// 标记区域为已访问
					for (int dy = 0; dy < 42; ++dy) 
					{
						for (int dx = 0; dx < 22; ++dx) 
						{
							visited[y + dy][x + dx] = true;
						}
					}
				}
			}
		}
	}

	return results;
}

// 新增辅助函数：验证指定位置和大小是否为有效的tile矩形
bool HerringboneTileset::ValidateTileRect(const Image* image, const IntVec2& bottomLeft, const IntVec2& size) 
{
	// 检查四个角是否都是彩色（非黑非白）
	IntVec2 corners[4] = {
		bottomLeft,                                    // 左上
		bottomLeft + IntVec2(size.x - 1, 0),          // 右上
		bottomLeft + IntVec2(size.x - 1, size.y - 1), // 右下
		bottomLeft + IntVec2(0, size.y - 1)           // 左下
	};

	for (int i = 0; i < 4; ++i) 
	{
		Rgba8 color = image->GetTexelColor(corners[i]);
		// 角点应该是彩色的
		if (IsBlackBorder(color) || color == Rgba8::WHITE) 
		{
			return false;
		}
	}

	//// 检查边框的黑色像素（跳过角点和中点标记）
	//// 检查顶边（跳过角点和中点）
	//int midX = size.x / 2;
	//for (int x = 1; x < size.x - 1; ++x) 
	//{
	//	if (x >= midX - 2 && x <= midX + 2) continue; // 跳过中点标记区域
	//	Rgba8 color = image->GetTexelColor(topLeft + IntVec2(x, 0));
	//	if (!IsBlackBorder(color) && color != Rgba8::WHITE) 
	//	{	// 允许白色（内容区）
	//		return false;
	//	}
	//}

	// 类似地检查其他三条边...
	// 为了简化，这里只检查角点，实际使用时可以更严格

	return true;
}

// ============================================
// 边缘约束提取
// ============================================

void HerringboneTileset::ExtractEdgeConstraints(
	const Image* image,
	const TileRect& rect,
	HbEdgeConstraint edges[6]) 
{
	IntVec2 bottomLeft = rect.bottomLeft;
	IntVec2 size = rect.size;

	if (rect.orientation == HbTileOrientation::HORIZONTAL) 
	{
		// 横向瓦片 (42×22，包含边框)
		// 黑框坐标：bottomLeft 到 bottomLeft + size - (1,1)

		// 边0: TOP 左半边 (顶部，y 最大)
		IntVec2 topLeftMid = bottomLeft + IntVec2(8, size.y - 1);
		edges[0] = ReadEdgeMarker(image, topLeftMid, IntVec2(1, 0));

		// 边1: TOP 右半边
		IntVec2 topRightMid = bottomLeft + IntVec2(28, size.y - 1);
		edges[1] = ReadEdgeMarker(image, topRightMid, IntVec2(1, 0));

		// 边2: RIGHT 边 (中点)
		IntVec2 rightMid = bottomLeft + IntVec2(size.x - 1, size.y / 2);
		edges[2] = ReadEdgeMarker(image, rightMid, IntVec2(0, 1));

		// 边3: BOTTOM 右半边 (底部，y=0)
		IntVec2 bottomRightMid = bottomLeft + IntVec2(28, 0);
		edges[3] = ReadEdgeMarker(image, bottomRightMid, IntVec2(1, 0));

		// 边4: BOTTOM 左半边
		IntVec2 bottomLeftMid = bottomLeft + IntVec2(8, 0);
		edges[4] = ReadEdgeMarker(image, bottomLeftMid, IntVec2(1, 0));

		// 边5: LEFT 边 (中点)
		IntVec2 leftMid = bottomLeft + IntVec2(0, size.y / 2);
		edges[5] = ReadEdgeMarker(image, leftMid, IntVec2(0, 1));

	}
	else {
		// 纵向瓦片 (22×42)

		// 边0: TOP (顶部，y 最大)
		IntVec2 topMid = bottomLeft + IntVec2(8, size.y - 1);
		edges[0] = ReadEdgeMarker(image, topMid, IntVec2(1, 0));

		// 边1: RIGHT 上半部
		IntVec2 rightTopMid = bottomLeft + IntVec2(size.x-1, 28);
		edges[1] = ReadEdgeMarker(image, rightTopMid, IntVec2(0, 1));

		// 边2: RIGHT 下半部
		IntVec2 rightBottomMid = bottomLeft + IntVec2(size.x - 1, 8);
		edges[2] = ReadEdgeMarker(image, rightBottomMid, IntVec2(0, 1));

		// 边3: BOTTOM (底部，y=0)
		IntVec2 bottomMid = bottomLeft + IntVec2(8, 0);
		edges[3] = ReadEdgeMarker(image, bottomMid, IntVec2(1, 0));

		// 边4: LEFT 下半部
		IntVec2 leftBottomMid = bottomLeft + IntVec2(0, 8);
		edges[4] = ReadEdgeMarker(image, leftBottomMid, IntVec2(0, 1));

		// 边5: LEFT 上半部
		IntVec2 leftTopMid = bottomLeft + IntVec2(0, 28);
		edges[5] = ReadEdgeMarker(image, leftTopMid, IntVec2(0, 1));
	}
}

HbEdgeConstraint HerringboneTileset::ReadEdgeMarker(
	const Image* image,
	const IntVec2& startPos,
	const IntVec2& direction) 
{
	// 读取5个连续的彩色像素，取中间的颜色作为约束标识
	IntVec2 midPos = startPos + direction * 2;  // 第3个像素（索引2）

	Rgba8 markerColor = image->GetTexelColor(midPos);

	// 如果是黑色或白色，可能读取位置不对，返回默认约束
	if (IsBlackBorder(markerColor) || markerColor == Rgba8::WHITE) 
	{
		DebuggerPrintf("Warning: Edge marker appears to be black/white at (%d,%d)\n",
			midPos.x, midPos.y);
	}

	return HbEdgeConstraint(markerColor);
}

// ============================================
// 内容像素提取
// ============================================

void HerringboneTileset::ExtractTileContent(
	const Image* image,
	const TileRect& rect,
	HerringboneTile* tile) 
{
	IntVec2 contentSize = tile->GetContentSize();

	// 内容区域在黑框内，偏移1像素（边框宽度）
	IntVec2 contentStart = rect.bottomLeft + IntVec2(1, 1);

	std::vector<Rgba8> pixels;
	pixels.reserve(contentSize.x * contentSize.y);

	for (int y = 0; y < contentSize.y; ++y) 
	{
		for (int x = 0; x < contentSize.x; ++x) 
		{
			IntVec2 pixelPos = contentStart + IntVec2(x, y);
			Rgba8 color = image->GetTexelColor(pixelPos);
			pixels.push_back(color);
		}
	}

	tile->SetContentData(pixels);
}

// ============================================
// 工具函数
// ============================================

bool HerringboneTileset::IsBlackBorder(const Rgba8& color) const 
{
	// 纯黑色判定（允许一点误差）
	return color.r ==0 && color.g == 0 && color.b == 0;
}

// ============================================
// Tile查找
// ============================================

HerringboneTile* HerringboneTileset::GetTile(int index) 
{
	if (index >= 0 && index < static_cast<int>(m_tiles.size())) 
	{
		return m_tiles[index];
	}
	return nullptr;
}

const HerringboneTile* HerringboneTileset::GetTile(int index) const 
{
	if (index >= 0 && index < static_cast<int>(m_tiles.size())) 
	{
		return m_tiles[index];
	}
	return nullptr;
}

std::vector<HerringboneTile*> HerringboneTileset::FindTilesByConstraints(
	HbTileOrientation orientation,
	const HbEdgeConstraint& constraint1,
	const HbEdgeConstraint& constraint2,
	const HbEdgeConstraint& constraint3) 
{
	std::vector<HerringboneTile*> results;

	// 线性搜索所有tiles
	for (HerringboneTile* tile : m_tiles) 
	{
		if (tile->GetOrientation() != orientation) 
		{
			continue;
		}

		// 检查前三条边的约束是否匹配
		if (tile->GetEdgeConstraint(0) == constraint1 &&
			tile->GetEdgeConstraint(1) == constraint2 &&
			tile->GetEdgeConstraint(2) == constraint3) 
		{
			results.push_back(tile);
		}
	}

	return results;
}

// ============================================
// 调试信息
// ============================================

void HerringboneTileset::PrintDebugInfo() const 
{
	DebuggerPrintf("=== Herringbone Tileset Info ===\n");
	DebuggerPrintf("Total tiles: %d\n", GetTileCount());

	int horizontalCount = 0;
	int verticalCount = 0;

	for (const auto* tile : m_tiles) 
	{
		if (tile->GetOrientation() == HbTileOrientation::HORIZONTAL)
		{
			horizontalCount++;
		}
		else 
		{
			verticalCount++;
		}
	}

	DebuggerPrintf("Horizontal tiles: %d\n", horizontalCount);
	DebuggerPrintf("Vertical tiles: %d\n", verticalCount);
	DebuggerPrintf("================================\n");
}