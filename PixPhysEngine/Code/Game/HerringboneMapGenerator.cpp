// HerringboneMapGenerator.cpp
#include "HerringboneMapGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "ThirdParty/Noise/RawNoise.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

HerringboneMapGenerator::HerringboneMapGenerator()
{
}

HerringboneMapGenerator::~HerringboneMapGenerator()
{
}

void HerringboneMapGenerator::InitializeTileGrid(const HbRegionGenParams& params)
{
	m_params = params;

	// how many herringbone pixel in this region
	m_regionSizeHPixels = m_params.m_regionSizeInChunks * HB_HPIXELS_PER_CHUNK;

	// 计算 half tile 网格尺寸
	m_halfTileGridSize = IntVec2(
		(m_regionSizeHPixels.x + HB_HALF_TILE_SIZE - 1) / HB_HALF_TILE_SIZE,
		(m_regionSizeHPixels.y + HB_HALF_TILE_SIZE - 1) / HB_HALF_TILE_SIZE
	);

	DebuggerPrintf("=== Initializing Herringbone Tile Grid ===\n");
	DebuggerPrintf("Region: Chunk(%d,%d) + Size(%d×%d chunks)\n",
		m_params.m_regionBottomLeftChunk.x, m_params.m_regionBottomLeftChunk.y,
		m_params.m_regionSizeInChunks.x, m_params.m_regionSizeInChunks.y);
	DebuggerPrintf("Region size in HPixels: %d×%d\n",
		m_regionSizeHPixels.x, m_regionSizeHPixels.y);
	DebuggerPrintf("Half Tile Grid size: %d×%d\n",
		m_halfTileGridSize.x, m_halfTileGridSize.y);

	// 初始化 half tile 网格
	InitializeHalfTileGrid();

	// 生成所有 tiles
	GenerateTiles();

	DebuggerPrintf("Total tiles generated: %d\n", (int)m_tilePlacements.size());
}

void HerringboneMapGenerator::RenderHPixelGrid() const
{
	if (m_tilePlacements.empty())
	{
		return;
	}

	std::vector<Vertex_PCU> verts;

	// 遍历所有已放置的 tile
	for (const HbTilePlacement& placement : m_tilePlacements)
	{
		if (!placement.m_selectedTile)
		{
			continue;
		}

		// 获取 tile 的内容尺寸（herringbone pixels）
		IntVec2 tileContentSize = placement.m_selectedTile->GetContentSize();

		// 遍历 tile 中的每个 pixel
		for (int localY = 0; localY < tileContentSize.y; ++localY)
		{
			for (int localX = 0; localX < tileContentSize.x; ++localX)
			{
				// 获取该 pixel 的颜色
				Rgba8 pixelColor = placement.m_selectedTile->GetContentPixel(localX, localY);

				// 计算该 pixel 在世界中的 herringbone pixel 坐标
				IntVec2 worldHPixel = placement.m_bottomLeftHPixel + IntVec2(localX, localY);

				// 转换为 cell 坐标（世界坐标）
				float worldX = worldHPixel.x * 8.0f;
				float worldY = worldHPixel.y * 8.0f;
				float pixelSize = 8.0f;;

				// 创建矩形边界
				AABB2 bounds(worldX, worldY, worldX + pixelSize, worldY + pixelSize);

				// 添加顶点
				AddVertsForAABB2D(verts, bounds, pixelColor);
			}
		}
	}

	// 渲染所有顶点
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(verts);
}

void HerringboneMapGenerator::RenderTileBoundaries() const
{
	if (m_tilePlacements.empty())
	{
		return;
	}
	std::vector<Vertex_PCU> verts;

	// 遍历所有已放置的 tile
	for (const HbTilePlacement& placement : m_tilePlacements)
	{
		if (!placement.m_selectedTile)
		{
			continue;
		}

		// 转换为 cell 坐标（世界坐标）
		float worldX = placement.m_bottomLeftHPixel.x * 8.0f;
		float worldY = placement.m_bottomLeftHPixel.y * 8.0f;
		float worldWidth = placement.m_sizeHPixel.x * 8.0f;
		float worldHeight = placement.m_sizeHPixel.y * 8.0f;

		// 创建矩形边界
		AABB2 bounds(worldX, worldY, worldX + worldWidth, worldY + worldHeight);

		// 横向 tile = 蓝色，纵向 tile = 红色
		Rgba8 wireColor;
		if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
		{
			wireColor = Rgba8(0, 100, 255, 255);  // 蓝色
		}
		else
		{
			wireColor = Rgba8(255, 0, 0, 255);     // 红色
		}

		// 添加边框
		AddVertsForAABBWire2D(verts, bounds, wireColor, 2.f, false);

		// === 添加约束标记（在框内） ===
		float markerSize = 2.0f;   // 标记方块的大小
		float inset = 4.0f;        // 向内偏移距离，确保在框内

		if (placement.m_orientation == HbTileOrientation::HORIZONTAL)
		{
			// 横向 tile 的 6 条边
			// 边 0: TOP_LEFT (顶部左侧)
			HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
			Vec2 pos0(worldX + worldWidth * 0.25f, worldY + worldHeight - inset);
			AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
				pos0.x + markerSize, pos0.y + markerSize);
			AddVertsForAABB2D(verts, marker0, edge0.m_color);

			// 边 1: TOP_RIGHT (顶部右侧)
			HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
			Vec2 pos1(worldX + worldWidth * 0.75f, worldY + worldHeight - inset);
			AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
				pos1.x + markerSize, pos1.y + markerSize);
			AddVertsForAABB2D(verts, marker1, edge1.m_color);

			// 边 2: RIGHT (右侧中间)
			HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
			Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.5f);
			AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
				pos2.x + markerSize, pos2.y + markerSize);
			AddVertsForAABB2D(verts, marker2, edge2.m_color);

			// 边 3: BOTTOM_RIGHT (底部右侧)
			HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
			Vec2 pos3(worldX + worldWidth * 0.75f, worldY + inset);
			AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
				pos3.x + markerSize, pos3.y + markerSize);
			AddVertsForAABB2D(verts, marker3, edge3.m_color);

			// 边 4: BOTTOM_LEFT (底部左侧)
			HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
			Vec2 pos4(worldX + worldWidth * 0.25f, worldY + inset);
			AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
				pos4.x + markerSize, pos4.y + markerSize);
			AddVertsForAABB2D(verts, marker4, edge4.m_color);

			// 边 5: LEFT (左侧中间)
			HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
			Vec2 pos5(worldX + inset, worldY + worldHeight * 0.5f);
			AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
				pos5.x + markerSize, pos5.y + markerSize);
			AddVertsForAABB2D(verts, marker5, edge5.m_color);
		}
		else // VERTICAL
		{
			// 纵向 tile 的 6 条边
			// 边 0: TOP (顶部中间)
			HbEdgeConstraint edge0 = placement.m_selectedTile->GetEdgeConstraint(0);
			Vec2 pos0(worldX + worldWidth * 0.5f, worldY + worldHeight - inset);
			AABB2 marker0(pos0.x - markerSize, pos0.y - markerSize,
				pos0.x + markerSize, pos0.y + markerSize);
			AddVertsForAABB2D(verts, marker0, edge0.m_color);

			// 边 1: RIGHT_TOP (右侧上部)
			HbEdgeConstraint edge1 = placement.m_selectedTile->GetEdgeConstraint(1);
			Vec2 pos1(worldX + worldWidth - inset, worldY + worldHeight * 0.75f);
			AABB2 marker1(pos1.x - markerSize, pos1.y - markerSize,
				pos1.x + markerSize, pos1.y + markerSize);
			AddVertsForAABB2D(verts, marker1, edge1.m_color);

			// 边 2: RIGHT_BOTTOM (右侧下部)
			HbEdgeConstraint edge2 = placement.m_selectedTile->GetEdgeConstraint(2);
			Vec2 pos2(worldX + worldWidth - inset, worldY + worldHeight * 0.25f);
			AABB2 marker2(pos2.x - markerSize, pos2.y - markerSize,
				pos2.x + markerSize, pos2.y + markerSize);
			AddVertsForAABB2D(verts, marker2, edge2.m_color);

			// 边 3: BOTTOM (底部中间)
			HbEdgeConstraint edge3 = placement.m_selectedTile->GetEdgeConstraint(3);
			Vec2 pos3(worldX + worldWidth * 0.5f, worldY + inset);
			AABB2 marker3(pos3.x - markerSize, pos3.y - markerSize,
				pos3.x + markerSize, pos3.y + markerSize);
			AddVertsForAABB2D(verts, marker3, edge3.m_color);

			// 边 4: LEFT_BOTTOM (左侧下部)
			HbEdgeConstraint edge4 = placement.m_selectedTile->GetEdgeConstraint(4);
			Vec2 pos4(worldX + inset, worldY + worldHeight * 0.25f);
			AABB2 marker4(pos4.x - markerSize, pos4.y - markerSize,
				pos4.x + markerSize, pos4.y + markerSize);
			AddVertsForAABB2D(verts, marker4, edge4.m_color);

			// 边 5: LEFT_TOP (左侧上部)
			HbEdgeConstraint edge5 = placement.m_selectedTile->GetEdgeConstraint(5);
			Vec2 pos5(worldX + inset, worldY + worldHeight * 0.75f);
			AABB2 marker5(pos5.x - markerSize, pos5.y - markerSize,
				pos5.x + markerSize, pos5.y + markerSize);
			AddVertsForAABB2D(verts, marker5, edge5.m_color);
		}
	}

	// 渲染所有顶点
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);
}

void HerringboneMapGenerator::InitializeHalfTileGrid()
{
	m_halfTileGrid.clear();
	m_halfTileGrid.resize(m_halfTileGridSize.y);

	for (int y = 0; y < m_halfTileGridSize.y; ++y)
	{
		m_halfTileGrid[y].resize(m_halfTileGridSize.x);
	}
}

void HerringboneMapGenerator::GenerateTiles()
{
	m_tilePlacements.clear();

	//RandomNumberGenerator rng(m_params.randomSeed);

	// 从左下到右上扫描每个 half tile
	for (int halfY = 0; halfY < m_halfTileGridSize.y; ++halfY)
	{
		for (int halfX = 0; halfX < m_halfTileGridSize.x; ++halfX)
		{
			IntVec2 halfTileCoord(halfX, halfY);

			// 如果该 half tile 已被填充，跳过
			if (m_halfTileGrid[halfY][halfX].m_isFilled)
			{
				continue;
			}

			// 获取该 half tile 的约束
			HalfTileConstraints constraints = GetConstraintsForHalfTile(halfTileCoord);

			// 判断该 half tile 是哪种类型
			HalfTilePositionType positionType = DetermineHalfTilePosition(constraints);

			if (positionType == HalfTilePositionType::INVALID)
			{
				DebuggerPrintf("ERROR: Cannot determine half tile position at (%d,%d)\n",
					halfX, halfY);
				continue;
			}

			// 放置完整的 tile
			PlaceTile(halfTileCoord, positionType);
		}
	}
}

// === 坐标转换函数 ===

IntVec2 HerringboneMapGenerator::HPixelToHalfTile(const IntVec2& hpixelCoord) const
{
	return IntVec2(
		hpixelCoord.x / HB_HALF_TILE_SIZE,
		hpixelCoord.y / HB_HALF_TILE_SIZE
	);
}

IntVec2 HerringboneMapGenerator::HalfTileToHPixel(const IntVec2& halfTileCoord) const
{
	return IntVec2(
		halfTileCoord.x * HB_HALF_TILE_SIZE,
		halfTileCoord.y * HB_HALF_TILE_SIZE
	);
}

bool HerringboneMapGenerator::IsHalfTileInBounds(const IntVec2& halfTileCoord) const
{
	return halfTileCoord.x >= 0 && halfTileCoord.x < m_halfTileGridSize.x &&
		halfTileCoord.y >= 0 && halfTileCoord.y < m_halfTileGridSize.y;
}

// === 约束获取 ===

HalfTileConstraints HerringboneMapGenerator::GetConstraintsForHalfTile(const IntVec2& halfTileCoord) const
{
	HalfTileConstraints result;

	// ========================================
	// 检查左侧
	// ========================================
	IntVec2 leftCoord = halfTileCoord + IntVec2(-1, 0);
	if (IsHalfTileInBounds(leftCoord) && m_halfTileGrid[leftCoord.y][leftCoord.x].m_isFilled)
	{
		const HalfTileState& leftHalf = m_halfTileGrid[leftCoord.y][leftCoord.x];
		const HbTilePlacement& leftTile = m_tilePlacements[leftHalf.m_fullTileIndex];
		result.m_hasLeft = true;

		// 根据左侧 half tile 的位置，确定约束边
		if (leftHalf.m_positionType == HalfTilePositionType::H_RIGHT)
		{
			// 左侧是横向tile的右半部分 -> 约束来自横向tile的右边
			// 衔接规则：横向右 - 纵向左下
			result.m_leftConstraint = leftTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::RIGHT));
			result.m_leftEdgeType = HerringboneEdgeType::H_RIGHT;
		}
		else if (leftHalf.m_positionType == HalfTilePositionType::V_TOP)
		{
			// 左侧是纵向tile的上半部分 -> 约束来自纵向tile的右上边
			// 衔接规则：纵向右上 - 横向左
			result.m_leftConstraint = leftTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::RIGHT_TOP));
			result.m_leftEdgeType = HerringboneEdgeType::V_RIGHT_TOP;
		}
		else if (leftHalf.m_positionType == HalfTilePositionType::V_BOTTOM)
		{
			// 左侧是纵向tile的下半部分 -> 约束来自纵向tile的右下边
			// 衔接规则：纵向右下 - 纵向左上
			result.m_leftConstraint = leftTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::RIGHT_BOTTOM));
			result.m_leftEdgeType = HerringboneEdgeType::V_RIGHT_BOTTOM;
		}
	}

	// ========================================
	// 检查右侧
	// ========================================
	IntVec2 rightCoord = halfTileCoord + IntVec2(1, 0);
	if (IsHalfTileInBounds(rightCoord) && m_halfTileGrid[rightCoord.y][rightCoord.x].m_isFilled)
	{
		const HalfTileState& rightHalf = m_halfTileGrid[rightCoord.y][rightCoord.x];
		const HbTilePlacement& rightTile = m_tilePlacements[rightHalf.m_fullTileIndex];
		result.m_hasRight = true;

		// 根据右侧 half tile 的位置，确定约束边
		if (rightHalf.m_positionType == HalfTilePositionType::H_LEFT)
		{
			// 右侧是横向tile的左半部分 -> 约束来自横向tile的左边
			// 衔接规则：横向左 - 纵向右上（反向：当前在左侧）
			result.m_rightConstraint = rightTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::LEFT));
			result.m_rightEdgeType = HerringboneEdgeType::H_LEFT;
		}
		else if (rightHalf.m_positionType == HalfTilePositionType::V_TOP)
		{
			// 右侧是纵向tile的上半部分 -> 约束来自纵向tile的左上边
			// 衔接规则：纵向左上 - 纵向右下（反向）
			result.m_rightConstraint = rightTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::LEFT_TOP));
			result.m_rightEdgeType = HerringboneEdgeType::V_LEFT_TOP;
		}
		else if (rightHalf.m_positionType == HalfTilePositionType::V_BOTTOM)
		{
			// 右侧是纵向tile的下半部分 -> 约束来自纵向tile的左下边
			// 衔接规则：纵向左下 - 横向右（反向）
			result.m_rightConstraint = rightTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::LEFT_BOTTOM));
			result.m_rightEdgeType = HerringboneEdgeType::V_LEFT_BOTTOM;
		}
	}

	// ========================================
	// 检查下方
	// ========================================
	IntVec2 bottomCoord = halfTileCoord + IntVec2(0, -1);
	if (IsHalfTileInBounds(bottomCoord) && m_halfTileGrid[bottomCoord.y][bottomCoord.x].m_isFilled)
	{
		const HalfTileState& bottomHalf = m_halfTileGrid[bottomCoord.y][bottomCoord.x];
		const HbTilePlacement& bottomTile = m_tilePlacements[bottomHalf.m_fullTileIndex];
		result.m_hasBottom = true;

		// 根据下方 half tile 的位置，确定约束边
		if (bottomHalf.m_positionType == HalfTilePositionType::V_TOP)
		{
			// 下方是纵向tile的上半部分 -> 约束来自纵向tile的顶部
			// 衔接规则：纵向上 - 横向左下（反向）
			result.m_bottomConstraint = bottomTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::TOP));
			result.m_bottomEdgeType = HerringboneEdgeType::V_TOP;
		}
		else if (bottomHalf.m_positionType == HalfTilePositionType::H_LEFT)
		{
			// 下方是横向tile的左半部分 -> 约束来自横向tile的上左边
			// 衔接规则：横向左上 - 横向右下（反向：当前在上方）
			result.m_bottomConstraint = bottomTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::TOP_LEFT));
			result.m_bottomEdgeType = HerringboneEdgeType::H_TOP_LEFT;
		}
		else if (bottomHalf.m_positionType == HalfTilePositionType::H_RIGHT)
		{
			// 下方是横向tile的右半部分 -> 约束来自横向tile的上右边
			// 衔接规则：横向右上 - 纵向下（反向）
			result.m_bottomConstraint = bottomTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::TOP_RIGHT));
			result.m_bottomEdgeType = HerringboneEdgeType::H_TOP_RIGHT;
		}
	}

	// ========================================
	// 检查上方
	// ========================================
	IntVec2 topCoord = halfTileCoord + IntVec2(0, 1);
	if (IsHalfTileInBounds(topCoord) && m_halfTileGrid[topCoord.y][topCoord.x].m_isFilled)
	{
		const HalfTileState& topHalf = m_halfTileGrid[topCoord.y][topCoord.x];
		const HbTilePlacement& topTile = m_tilePlacements[topHalf.m_fullTileIndex];
		result.m_hasTop = true;

		// 根据上方 half tile 的位置，确定约束边
		if (topHalf.m_positionType == HalfTilePositionType::V_BOTTOM)
		{
			// 上方是纵向tile的下半部分 -> 约束来自纵向tile的底部
			// 衔接规则：纵向下 - 横向右上（反向：当前在下方）
			result.m_topConstraint = topTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(VTileEdge::BOTTOM));
			result.m_topEdgeType = HerringboneEdgeType::V_BOTTOM;
		}
		else if (topHalf.m_positionType == HalfTilePositionType::H_LEFT)
		{
			// 上方是横向tile的左半部分 -> 约束来自横向tile的下左边
			// 衔接规则：横向左下 - 纵向上（反向）
			result.m_topConstraint = topTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::BOTTOM_LEFT));
			result.m_topEdgeType = HerringboneEdgeType::H_BOTTOM_LEFT;
		}
		else if (topHalf.m_positionType == HalfTilePositionType::H_RIGHT)
		{
			// 上方是横向tile的右半部分 -> 约束来自横向tile的下右边
			// 衔接规则：横向右下 - 横向左上（反向）
			result.m_topConstraint = topTile.m_selectedTile->GetEdgeConstraint(
				static_cast<int>(HTileEdge::BOTTOM_RIGHT));
			result.m_topEdgeType = HerringboneEdgeType::H_BOTTOM_RIGHT;
		}
	}

	return result;
}

// === 判断 half tile 类型 ===

HalfTilePositionType HerringboneMapGenerator::DetermineHalfTilePosition(
	const HalfTileConstraints& constraints) const
{
	// ========================================
	// 无约束 -> 第一个 half tile (H_LEFT)
	// ========================================
	if (!constraints.m_hasLeft && !constraints.m_hasRight &&
		!constraints.m_hasTop && !constraints.m_hasBottom)
	{
		return HalfTilePositionType::H_LEFT;
	}

	// ========================================
	// 根据任意一个约束的边缘类型确定当前 half tile 类型
	// 优先级：左 > 下 > 右 > 上
	// ========================================

	if (constraints.m_hasLeft)
	{
		switch (constraints.m_leftEdgeType)
		{
		case HerringboneEdgeType::H_RIGHT:
			// 衔接规则：横向右 - 纵向左下
			return HalfTilePositionType::V_BOTTOM;

		case HerringboneEdgeType::V_RIGHT_TOP:
			// 衔接规则：纵向右上 - 横向左
			return HalfTilePositionType::H_LEFT;

		case HerringboneEdgeType::V_RIGHT_BOTTOM:
			// 衔接规则：纵向右下 - 纵向左上
			return HalfTilePositionType::V_TOP;

		default:
			DebuggerPrintf("ERROR: Unexpected left edge type: %d\n",
				(int)constraints.m_leftEdgeType);
			return HalfTilePositionType::INVALID;
		}
	}

	if (constraints.m_hasBottom)
	{
		switch (constraints.m_bottomEdgeType)
		{
		case HerringboneEdgeType::V_TOP:
			// 衔接规则：纵向上 - 横向左下（反向）
			return HalfTilePositionType::H_LEFT;

		case HerringboneEdgeType::H_TOP_LEFT:
			// 衔接规则：横向左上 - 横向右下（反向：当前在上方）
			return HalfTilePositionType::H_RIGHT;

		case HerringboneEdgeType::H_TOP_RIGHT:
			// 衔接规则：横向右上 - 纵向下（反向）
			return HalfTilePositionType::V_BOTTOM;

		default:
			DebuggerPrintf("ERROR: Unexpected bottom edge type: %d\n",
				(int)constraints.m_bottomEdgeType);
			return HalfTilePositionType::INVALID;
		}
	}

	if (constraints.m_hasRight)
	{
		switch (constraints.m_rightEdgeType)
		{
		case HerringboneEdgeType::H_LEFT:
			// 衔接规则：横向左 - 纵向右上（反向：当前在左侧）
			return HalfTilePositionType::V_TOP;

		case HerringboneEdgeType::V_LEFT_TOP:
			// 衔接规则：纵向左上 - 纵向右下（反向）
			return HalfTilePositionType::V_BOTTOM;

		case HerringboneEdgeType::V_LEFT_BOTTOM:
			// 衔接规则：纵向左下 - 横向右（反向）
			return HalfTilePositionType::H_RIGHT;

		default:
			DebuggerPrintf("ERROR: Unexpected right edge type: %d\n",
				(int)constraints.m_rightEdgeType);
			return HalfTilePositionType::INVALID;
		}
	}

	if (constraints.m_hasTop)
	{
		switch (constraints.m_topEdgeType)
		{
		case HerringboneEdgeType::V_BOTTOM:
			// 衔接规则：纵向下 - 横向右上（反向：当前在下方）
			return HalfTilePositionType::H_RIGHT;

		case HerringboneEdgeType::H_BOTTOM_LEFT:
			// 衔接规则：横向左下 - 纵向上（反向）
			return HalfTilePositionType::V_TOP;

		case HerringboneEdgeType::H_BOTTOM_RIGHT:
			// 衔接规则：横向右下 - 横向左上（反向）
			return HalfTilePositionType::H_LEFT;

		default:
			DebuggerPrintf("ERROR: Unexpected top edge type: %d\n",
				(int)constraints.m_topEdgeType);
			return HalfTilePositionType::INVALID;
		}
	}

	// 不应该到这里
	DebuggerPrintf("ERROR: No valid constraint found to determine half tile position\n");
	return HalfTilePositionType::INVALID;
}

// === 放置 Tile ===

void HerringboneMapGenerator::PlaceTile(const IntVec2& startHalfTile, HalfTilePositionType position)
{
	HbTilePlacement placement;

	// 根据 position 确定 tile 的方向和占据的 half tiles
	if (position == HalfTilePositionType::H_LEFT || position == HalfTilePositionType::H_RIGHT)
	{
		// 横向 tile
		placement.m_orientation = HbTileOrientation::HORIZONTAL;
		placement.m_sizeHPixel = IntVec2(HB_H_TILE_WIDTH, HB_H_TILE_HEIGHT);

		if (position == HalfTilePositionType::H_LEFT)
		{
			// 左半 -> tile 从当前 half tile 开始
			placement.m_bottomLeftHPixel = HalfTileToHPixel(startHalfTile);
			placement.m_halfTileCoords[0] = startHalfTile;
			placement.m_halfTileCoords[1] = startHalfTile + IntVec2(1, 0);  // 右半
		}
		else
		{
			// 右半 -> tile 从左侧一个 half tile 开始
			placement.m_bottomLeftHPixel = HalfTileToHPixel(startHalfTile + IntVec2(-1, 0));
			placement.m_halfTileCoords[0] = startHalfTile + IntVec2(-1, 0);  // 左半
			placement.m_halfTileCoords[1] = startHalfTile;  // 右半
		}
	}
	else
	{
		// 纵向 tile
		placement.m_orientation = HbTileOrientation::VERTICAL;
		placement.m_sizeHPixel = IntVec2(HB_V_TILE_WIDTH, HB_V_TILE_HEIGHT);

		if (position == HalfTilePositionType::V_BOTTOM)
		{
			// 下半 -> tile 从当前 half tile 开始
			placement.m_bottomLeftHPixel = HalfTileToHPixel(startHalfTile);
			placement.m_halfTileCoords[0] = startHalfTile;
			placement.m_halfTileCoords[1] = startHalfTile + IntVec2(0, 1);  // 上半
		}
		else
		{
			// 上半 -> tile 从下方一个 half tile 开始
			placement.m_bottomLeftHPixel = HalfTileToHPixel(startHalfTile + IntVec2(0, -1));
			placement.m_halfTileCoords[0] = startHalfTile + IntVec2(0, -1);  // 下半
			placement.m_halfTileCoords[1] = startHalfTile;  // 上半
		}
	}

	// ------------------------------------------------------------------------------
	// 获取 tile 的约束并从 tileset 选择匹配的 tile
	HbEdgeConstraint constraints[6];
	bool hasConstraint[6];
	GetTileConstraints(placement.m_orientation, placement.m_halfTileCoords[0],
		constraints, hasConstraint);

	//-------------------------------------------------------------------------------
	auto candidates = m_params.m_tileset->FindTilesByConstraints(
		placement.m_orientation, constraints, hasConstraint);

	if (candidates.empty())
	{
		DebuggerPrintf("ERROR: No matching tile found at half tile (%d,%d)\n",
			startHalfTile.x, startHalfTile.y);
		return;
	}

	// -------------------------------------------------------------------------------

	int noiseIndex = startHalfTile.x + startHalfTile.y * 10000; // 创建唯一索引
	float noiseValue = Get1dNoiseZeroToOne(noiseIndex, m_params.m_randomSeed);
	int selectedIndex = (int)(noiseValue * (float)candidates.size());

	// 确保索引在有效范围内
	if (selectedIndex >= (int)candidates.size())
	{
		selectedIndex = (int)candidates.size() - 1;
	}

	placement.m_selectedTile = candidates[selectedIndex];

	// 添加到列表
	int tileIndex = (int)m_tilePlacements.size();
	m_tilePlacements.push_back(placement);

	// 标记占据的 half tiles
	for (int i = 0; i < 2; ++i)
	{
		IntVec2 coord = placement.m_halfTileCoords[i];
		if (IsHalfTileInBounds(coord))
		{
			HalfTileState& cell = m_halfTileGrid[coord.y][coord.x];
			cell.m_isFilled = true;
			cell.m_positionType = (i == 0) ?
				(placement.m_orientation == HbTileOrientation::HORIZONTAL ? HalfTilePositionType::H_LEFT : HalfTilePositionType::V_BOTTOM) :
				(placement.m_orientation == HbTileOrientation::HORIZONTAL ? HalfTilePositionType::H_RIGHT : HalfTilePositionType::V_TOP);
			cell.m_fullTileIndex = tileIndex;
		}
	}
}

// === 获取 Tile 约束 ===

// 在 HerringboneMapGenerator.cpp 中重新实现
void HerringboneMapGenerator::GetTileConstraints(
	HbTileOrientation orientation,
	const IntVec2& bottomLeftHalfTile,
	HbEdgeConstraint constraints[6],
	bool hasConstraint[6]) const
{
	// 初始化：默认所有边都没有约束
	for (int i = 0; i < 6; ++i)
	{
		constraints[i] = HbEdgeConstraint();
		hasConstraint[i] = false;
	}

	// 获取两个 half tile 的约束信息
	HalfTileConstraints leftHalf = GetConstraintsForHalfTile(bottomLeftHalfTile);
	HalfTileConstraints rightHalf;

	if (orientation == HbTileOrientation::HORIZONTAL)
	{
		rightHalf = GetConstraintsForHalfTile(bottomLeftHalfTile + IntVec2(1, 0));

		// 横向 tile 的 6 条边：
		// 0: TOP_LEFT    1: TOP_RIGHT    2: RIGHT
		// 3: BOTTOM_RIGHT 4: BOTTOM_LEFT  5: LEFT

		// 边 5: LEFT (来自左侧 half tile 的左边约束)
		if (leftHalf.m_hasLeft)
		{
			constraints[5] = leftHalf.m_leftConstraint;
			hasConstraint[5] = true;
		}

		// 边 0: TOP_LEFT (来自左侧 half tile 的上边约束)
		if (leftHalf.m_hasTop)
		{
			constraints[0] = leftHalf.m_topConstraint;
			hasConstraint[0] = true;
		}

		// 边 1: TOP_RIGHT (来自右侧 half tile 的上边约束)
		if (rightHalf.m_hasTop)
		{
			constraints[1] = rightHalf.m_topConstraint;
			hasConstraint[1] = true;
		}

		// 边 2: RIGHT (来自右侧 half tile 的右边约束)
		if (rightHalf.m_hasRight)
		{
			constraints[2] = rightHalf.m_rightConstraint;
			hasConstraint[2] = true;
		}

		// 边 3: BOTTOM_RIGHT (来自右侧 half tile 的下边约束)
		if (rightHalf.m_hasBottom)
		{
			constraints[3] = rightHalf.m_bottomConstraint;
			hasConstraint[3] = true;
		}

		// 边 4: BOTTOM_LEFT (来自左侧 half tile 的下边约束)
		if (leftHalf.m_hasBottom)
		{
			constraints[4] = leftHalf.m_bottomConstraint;
			hasConstraint[4] = true;
		}
	}
	else // VERTICAL
	{
		rightHalf = GetConstraintsForHalfTile(bottomLeftHalfTile + IntVec2(0, 1));

		// 纵向 tile 的 6 条边：
		// 0: TOP         1: RIGHT_TOP    2: RIGHT_BOTTOM
		// 3: BOTTOM      4: LEFT_BOTTOM  5: LEFT_TOP

		// 边 3: BOTTOM (来自下侧 half tile 的下边约束)
		if (leftHalf.m_hasBottom)
		{
			constraints[3] = leftHalf.m_bottomConstraint;
			hasConstraint[3] = true;
		}

		// 边 4: LEFT_BOTTOM (来自下侧 half tile 的左边约束)
		if (leftHalf.m_hasLeft)
		{
			constraints[4] = leftHalf.m_leftConstraint;
			hasConstraint[4] = true;
		}

		// 边 5: LEFT_TOP (来自上侧 half tile 的左边约束)
		if (rightHalf.m_hasLeft)
		{
			constraints[5] = rightHalf.m_leftConstraint;
			hasConstraint[5] = true;
		}

		// 边 0: TOP (来自上侧 half tile 的上边约束)
		if (rightHalf.m_hasTop)
		{
			constraints[0] = rightHalf.m_topConstraint;
			hasConstraint[0] = true;
		}

		// 边 1: RIGHT_TOP (来自上侧 half tile 的右边约束)
		if (rightHalf.m_hasRight)
		{
			constraints[1] = rightHalf.m_rightConstraint;
			hasConstraint[1] = true;
		}

		// 边 2: RIGHT_BOTTOM (来自下侧 half tile 的右边约束)
		if (leftHalf.m_hasRight)
		{
			constraints[2] = leftHalf.m_rightConstraint;
			hasConstraint[2] = true;
		}
	}
}

//// === 调试输出 ===
//
//void HerringboneMapGenerator::PrintGridDebugInfo() const
//{
//	DebuggerPrintf("\n=== Tile Placement List ===\n");
//
//	for (size_t i = 0; i < m_tilePlacements.size(); ++i)
//	{
//		const HbTilePlacement& p = m_tilePlacements[i];
//
//		char orientChar = (p.orientation == HbTileOrientation::HORIZONTAL) ? 'H' : 'V';
//		IntVec2 cellBL = p.GetBottomLeftCell();
//		IntVec2 chunkBL = p.GetBottomLeftChunk();
//
//		DebuggerPrintf("[%03d] %c HPixel(%3d,%3d) Size(%2dx%2d) Cell(%4d,%4d) Chunk(%2d,%2d)\n",
//			(int)i, orientChar,
//			p.bottomLeftHPixel.x, p.bottomLeftHPixel.y,
//			p.sizeHPixel.x, p.sizeHPixel.y,
//			cellBL.x, cellBL.y,
//			chunkBL.x, chunkBL.y
//		);
//	}
//}
//
//void HerringboneMapGenerator::PrintHalfTileGrid() const
//{
//	DebuggerPrintf("\n=== Half Tile Grid (X: filled, .: empty) ===\n");
//
//	for (int y = m_halfTileGridSize.y - 1; y >= 0; --y)
//	{
//		for (int x = 0; x < m_halfTileGridSize.x; ++x)
//		{
//			char c = m_halfTileGrid[y][x].isFilled ? 'X' : '.';
//			DebuggerPrintf("%c", c);
//		}
//		DebuggerPrintf("\n");
//	}
//}