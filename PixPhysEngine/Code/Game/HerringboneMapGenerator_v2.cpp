#include "HerringboneMapGenerator_v2.hpp"
#include "RegionGenerationData_v2.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/Noise/RawNoise.hpp"
#include "SuperChunk.hpp"

HerringboneMapGenerator::HerringboneMapGenerator()
{
}

HerringboneMapGenerator::~HerringboneMapGenerator()
{
}

void HerringboneMapGenerator::Generate(RegionGenerationData* regionData)
{
    if (!regionData) {
        DebuggerPrintf("ERROR: RegionData is null\n");
        return;
    }
    
    DebuggerPrintf("Generator: Starting generation process...\n");
    
    // 步骤1: 初始化生成参数
    InitializeGenerationParams(regionData);
    
    // 步骤2: 初始化half tile网格
    InitializeHalfTileGrid(regionData);
    
    // 步骤3: 生成tiles
    GenerateTiles(regionData);
    
    // 步骤4: 渲染pixels
    RenderPixelsToBuffer(regionData);
    
    DebuggerPrintf("Generator: Generation complete\n");
}

// === 步骤1: 初始化生成参数 ===

void HerringboneMapGenerator::InitializeGenerationParams(RegionGenerationData* regionData)
{
	// 计算Region的chunk范围（不expand）
	IntVec2 regionBottomLeftChunk(
		regionData->m_bounds.m_bottomLeftSC.x * CHUNKS_PER_SUPER_CHUNK,
		regionData->m_bounds.m_bottomLeftSC.y * CHUNKS_PER_SUPER_CHUNK
	);

	IntVec2 regionSizeInSCs = regionData->m_bounds.GetSizeInSuperChunks();
	IntVec2 regionSizeInChunks(
		regionSizeInSCs.x * CHUNKS_PER_SUPER_CHUNK,
		regionSizeInSCs.y * CHUNKS_PER_SUPER_CHUNK
	);

	// ⭐ 直接使用实际区域，不expand
	regionData->m_expandedBottomLeftChunk = regionBottomLeftChunk;
	regionData->m_expandedSizeInChunks = regionSizeInChunks;

	// 计算herringbone pixel尺寸
	regionData->m_regionSizeHPixels = regionSizeInChunks * HB_HPIXELS_PER_CHUNK;

	// 计算half tile网格尺寸
	regionData->m_halfTileGridSize = IntVec2(
		(regionData->m_regionSizeHPixels.x + HB_HALF_TILE_SIZE - 1) / HB_HALF_TILE_SIZE,
		(regionData->m_regionSizeHPixels.y + HB_HALF_TILE_SIZE - 1) / HB_HALF_TILE_SIZE
	);

	// pixel尺寸（与region size相同）
	regionData->m_pixelSize = regionSizeInChunks * HB_HPIXELS_PER_CHUNK;

	DebuggerPrintf("  Initialized params:\n");
	DebuggerPrintf("    Chunk area: [%d,%d] + %dx%d\n",
		regionData->m_expandedBottomLeftChunk.x,
		regionData->m_expandedBottomLeftChunk.y,
		regionData->m_expandedSizeInChunks.x,
		regionData->m_expandedSizeInChunks.y);
	DebuggerPrintf("    Region size (hpixels): %dx%d\n",
		regionData->m_regionSizeHPixels.x,
		regionData->m_regionSizeHPixels.y);
}

// === 步骤2: 初始化half tile网格 ===

void HerringboneMapGenerator::InitializeHalfTileGrid(RegionGenerationData* regionData)
{
    regionData->m_halfTileGrid.clear();
    regionData->m_halfTileGrid.resize(regionData->m_halfTileGridSize.y);
    
    for (int y = 0; y < regionData->m_halfTileGridSize.y; ++y) {
        regionData->m_halfTileGrid[y].resize(regionData->m_halfTileGridSize.x);
    }
    
    DebuggerPrintf("  Initialized half tile grid: %dx%d\n",
        regionData->m_halfTileGridSize.x,
        regionData->m_halfTileGridSize.y);
}

// === 步骤3: 生成tiles ===

void HerringboneMapGenerator::GenerateTiles(RegionGenerationData* regionData)
{
    regionData->m_tilePlacements.clear();
    
    // 从左下到右上扫描每个half tile
    for (int halfY = 0; halfY < regionData->m_halfTileGridSize.y; ++halfY) 
    {
        for (int halfX = 0; halfX < regionData->m_halfTileGridSize.x; ++halfX) 
        {
            IntVec2 halfTileCoord(halfX, halfY);
            
            // 如果已填充，跳过
            if (regionData->m_halfTileGrid[halfY][halfX].m_isFilled) {
                continue;
            }
            
            // 获取约束
            HalfTileConstraints constraints = GetConstraintsForHalfTile(regionData, halfTileCoord);
            
            // 判断类型
            HalfTilePositionType positionType = DetermineHalfTilePosition(constraints);
            
            if (positionType == HalfTilePositionType::INVALID) {
                DebuggerPrintf("WARNING: Cannot determine position for half tile (%d,%d)\n",
                    halfX, halfY);
                continue;
            }
            
            // 放置tile
            PlaceTile(regionData, halfTileCoord, positionType);
        }
    }
    
    DebuggerPrintf("  Generated %d tiles\n", static_cast<int>(regionData->m_tilePlacements.size()));
}

// === 步骤4: 渲染pixels ===

void HerringboneMapGenerator::RenderPixelsToBuffer(RegionGenerationData* regionData)
{
    // 分配pixel buffer（最终尺寸）
    regionData->m_pixels.clear();
    regionData->m_pixels.resize(regionData->m_pixelSize.y);
    for (int y = 0; y < regionData->m_pixelSize.y; ++y) {
        regionData->m_pixels[y].resize(regionData->m_pixelSize.x);
    }
    
    // sourceOffset = 跳过边界的1个chunk = 4 hpixels
   // IntVec2 sourceOffset(HB_HPIXELS_PER_CHUNK, HB_HPIXELS_PER_CHUNK);
    
    // 遍历buffer的每个位置
    for (int bufferY = 0; bufferY < regionData->m_pixelSize.y; ++bufferY) {
        for (int bufferX = 0; bufferX < regionData->m_pixelSize.x; ++bufferX) {
            // 计算在生成区域中的源坐标
            IntVec2 sourceHPixel(bufferX, bufferY);
            
            // 查找该位置对应的tile
            bool found = false;
            for (const HbTilePlacement& placement : regionData->m_tilePlacements) {
                if (!placement.m_selectedTile) {
                    continue;
                }
                
                // 检查是否在tile范围内
                IntVec2 tileMin = placement.m_bottomLeftHPixel;
                IntVec2 tileMax = placement.m_bottomLeftHPixel + placement.m_sizeHPixel;
                
                if (sourceHPixel.x >= tileMin.x && sourceHPixel.x < tileMax.x &&
                    sourceHPixel.y >= tileMin.y && sourceHPixel.y < tileMax.y) {
                    // 计算tile内局部坐标
                    IntVec2 localHPixel = sourceHPixel - tileMin;
                    
                    // 获取pixel颜色
                    Rgba8 pixelColor = placement.m_selectedTile->GetContentPixel(
                        localHPixel.x, localHPixel.y
                    );
                    
                    regionData->m_pixels[bufferY][bufferX] = pixelColor;
                    found = true;
                    break;
                }
            }
            
            // 未找到tile，填充默认颜色
            if (!found) {
                regionData->m_pixels[bufferY][bufferX] = Rgba8::MAGNETA;
            }
        }
    }
    
    DebuggerPrintf("  Rendered pixels: %dx%d\n",
        regionData->m_pixelSize.x, regionData->m_pixelSize.y);
}

// === 辅助函数 ===

IntVec2 HerringboneMapGenerator::HPixelToHalfTile(IntVec2 const& hpixelCoord)
{
    return IntVec2(
        hpixelCoord.x / HB_HALF_TILE_SIZE,
        hpixelCoord.y / HB_HALF_TILE_SIZE
    );
}

IntVec2 HerringboneMapGenerator::HalfTileToHPixel(IntVec2 const& halfTileCoord)
{
    return IntVec2(
        halfTileCoord.x * HB_HALF_TILE_SIZE,
        halfTileCoord.y * HB_HALF_TILE_SIZE
    );
}

// === GetConstraintsForHalfTile ===
// 这里需要实现完整的约束检测逻辑（从原来的实现复制并修改）
HalfTileConstraints HerringboneMapGenerator::GetConstraintsForHalfTile(
    RegionGenerationData* regionData,
    IntVec2 const& halfTileCoord) const
{
	HalfTileConstraints result;

	// ========================================
	// 检查左侧
	// ========================================
	IntVec2 leftCoord = halfTileCoord + IntVec2(-1, 0);
	if (IsHalfTileInBounds(regionData, leftCoord) && regionData-> m_halfTileGrid[leftCoord.y][leftCoord.x].m_isFilled)
	{
		const HalfTileState& leftHalf = regionData->m_halfTileGrid[leftCoord.y][leftCoord.x];
		const HbTilePlacement& leftTile = regionData->m_tilePlacements[leftHalf.m_fullTileIndex];
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
	if (IsHalfTileInBounds(regionData, rightCoord) && regionData->m_halfTileGrid[rightCoord.y][rightCoord.x].m_isFilled)
	{
		const HalfTileState& rightHalf = regionData->m_halfTileGrid[rightCoord.y][rightCoord.x];
		const HbTilePlacement& rightTile = regionData->m_tilePlacements[rightHalf.m_fullTileIndex];
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
	if (IsHalfTileInBounds(regionData, bottomCoord) && regionData->m_halfTileGrid[bottomCoord.y][bottomCoord.x].m_isFilled)
	{
		const HalfTileState& bottomHalf = regionData->m_halfTileGrid[bottomCoord.y][bottomCoord.x];
		const HbTilePlacement& bottomTile = regionData->m_tilePlacements[bottomHalf.m_fullTileIndex];
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
	if (IsHalfTileInBounds(regionData, topCoord) && regionData->m_halfTileGrid[topCoord.y][topCoord.x].m_isFilled)
	{
		const HalfTileState& topHalf = regionData->m_halfTileGrid[topCoord.y][topCoord.x];
		const HbTilePlacement& topTile = regionData->m_tilePlacements[topHalf.m_fullTileIndex];
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

// === DetermineHalfTilePosition ===
HalfTilePositionType HerringboneMapGenerator::DetermineHalfTilePosition(
    HalfTileConstraints const& constraints) const
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

// === PlaceTile ===
void HerringboneMapGenerator::PlaceTile(
    RegionGenerationData* regionData,
    IntVec2 const& startHalfTile,
    HalfTilePositionType position)
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
	GetTileConstraints(regionData,placement.m_orientation, placement.m_halfTileCoords[0],
		constraints, hasConstraint);

	//-------------------------------------------------------------------------------
	auto candidates =regionData->m_tileset->FindTilesByConstraints(
		placement.m_orientation, constraints, hasConstraint);

	if (candidates.empty())
	{
		DebuggerPrintf("ERROR: No matching tile found at half tile (%d,%d)\n",
			startHalfTile.x, startHalfTile.y);
		return;
	}

	// -------------------------------------------------------------------------------

	int noiseIndex = startHalfTile.x + startHalfTile.y * 10000; // 创建唯一索引
	float noiseValue = Get1dNoiseZeroToOne(noiseIndex, regionData->m_regionSeed);
	int selectedIndex = (int)(noiseValue * (float)candidates.size());

	// 确保索引在有效范围内
	if (selectedIndex >= (int)candidates.size())
	{
		selectedIndex = (int)candidates.size() - 1;
	}

	placement.m_selectedTile = candidates[selectedIndex];

	// 添加到列表
	int tileIndex = (int)regionData->m_tilePlacements.size();
	regionData->m_tilePlacements.push_back(placement);

	// 标记占据的 half tiles
	for (int i = 0; i < 2; ++i)
	{
		IntVec2 coord = placement.m_halfTileCoords[i];
		if (IsHalfTileInBounds(regionData,coord))
		{
			HalfTileState& cell = regionData->m_halfTileGrid[coord.y][coord.x];
			cell.m_isFilled = true;
			cell.m_positionType = (i == 0) ?
				(placement.m_orientation == HbTileOrientation::HORIZONTAL ? HalfTilePositionType::H_LEFT : HalfTilePositionType::V_BOTTOM) :
				(placement.m_orientation == HbTileOrientation::HORIZONTAL ? HalfTilePositionType::H_RIGHT : HalfTilePositionType::V_TOP);
			cell.m_fullTileIndex = tileIndex;
		}
	}
}

// === GetTileConstraints ===
void HerringboneMapGenerator::GetTileConstraints(
    RegionGenerationData* regionData,
    HbTileOrientation orientation,
    IntVec2 const& bottomLeftHalfTile,
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
	HalfTileConstraints leftHalf = GetConstraintsForHalfTile(regionData,bottomLeftHalfTile);
	HalfTileConstraints rightHalf;

	if (orientation == HbTileOrientation::HORIZONTAL)
	{
		rightHalf = GetConstraintsForHalfTile(regionData,bottomLeftHalfTile + IntVec2(1, 0));

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
		rightHalf = GetConstraintsForHalfTile(regionData,bottomLeftHalfTile + IntVec2(0, 1));

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

bool HerringboneMapGenerator::IsHalfTileInBounds(RegionGenerationData* regionData, const IntVec2& halfTileCoord) const
{
	return halfTileCoord.x >= 0 && halfTileCoord.x < regionData->m_halfTileGridSize.x&&
				halfTileCoord.y >= 0 && halfTileCoord.y < regionData->m_halfTileGridSize.y;
}


