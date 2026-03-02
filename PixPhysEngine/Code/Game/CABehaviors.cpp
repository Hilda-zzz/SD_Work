#include "CABehaviors.hpp"
#include "CAPrimitives.hpp"
#include "CellMatManager.hpp"

// ============================================================
//  落雪
//  规则：
//    Falling（ra==1）：随机选 Down / SW / SE，目标为空则 swap
//    Stationary（ra==2）：左右都空时随机水平漂移；
//                         斜上有雪且正下为空时下落
// ============================================================
void CABehaviors::Update_Snow(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	bool belowInBounds = ctx.map->IsInBounds(ctx.x, ctx.y - 1);
	bool belowEmpty = belowInBounds && CA::IsEmpty(ctx, Dir::Down);
	int state = belowEmpty ? 1 : 2;

	if (state == 1)
	{
		const CADir fallDirs[3] = { Dir::Down, Dir::SW, Dir::SE };
		for (int i = 0; i < 3; ++i)
		{
			if (rand() % 3 != 0) continue;
			CA::SwapIfEmpty(ctx, fallDirs[rand() % 3]);
		}
	}

	if (state == 2)
	{
		int originalX = ctx.x;
		int originalY = ctx.y;

		if (CA::IsEmpty(ctx, Dir::Left) && CA::IsEmpty(ctx, Dir::Right))
		{
			CA::SwapIfEmpty(ctx, Dir::RandomH());
		}

		CAContext ctxOriginal(originalX, originalY, map);
		CADir diagUp = CADir((rand() % 2 == 0) ? -1 : 1, 1);

		for (int i = 0; i < 2; ++i)
		{
			if (CA::IsEmpty(ctxOriginal, Dir::Down) &&
				CA::IsType(ctxOriginal, diagUp, CellMatType::MAT_CA_SNOW))
			{
				CA::Swap(ctxOriginal, Dir::Down);
			}
			else
			{
				break;
			}
		}
	}

	// 规则6: 左右上都是雪且下方为空 → 与上方交换
	if (CA::IsType(ctx, Dir::Left, CellMatType::MAT_CA_SNOW) &&
		CA::IsType(ctx, Dir::Right, CellMatType::MAT_CA_SNOW) &&
		CA::IsType(ctx, Dir::Up, CellMatType::MAT_CA_SNOW) &&
		CA::IsEmpty(ctx, Dir::Down))
	{
		CA::Swap(ctx, Dir::Up);
	}
}

void CABehaviors::Update_Cloud(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	bool belowInBounds = ctx.map->IsInBounds(ctx.x, ctx.y - 1);
	bool belowEmpty = belowInBounds && CA::IsEmpty(ctx, Dir::Down);
	int state = belowEmpty ? 1 : 2;

	// 规则2: Falling
	if (state == 1)
	{
		const CADir fallDirs[3] = { Dir::Down, Dir::SW, Dir::SE };
		for (int i = 0; i < 3; ++i)
		{
			if (rand() % 3 != 0) continue;
			CA::SwapIfEmpty(ctx, fallDirs[rand() % 3]);
		}
	}

	// 规则3: Stationary
	if (state == 2)
	{
		int originalX = ctx.x;
		int originalY = ctx.y;

		if (CA::IsEmpty(ctx, Dir::Left) && CA::IsEmpty(ctx, Dir::Right))
		{
			CA::SwapIfEmpty(ctx, Dir::RandomH());
		}

		// 3.2 改为检查 MAT_CA_CLOUD 自身类型
		CAContext ctxOriginal(originalX, originalY, map);
		int randomDir = (rand() % 2 == 0) ? -1 : 1;
		CADir diagUp = Dir::Up + CADir(randomDir, 0);

		for (int i = 0; i < 2; ++i)
		{
			if (CA::IsEmpty(ctxOriginal, Dir::Down) &&
				CA::IsType(ctxOriginal, diagUp, CellMatType::MAT_CA_CLOUD))
			{
				CA::Swap(ctxOriginal, Dir::Down);
			}
			else break;
		}

		// ← 新增：Stationary时若周围云多，说明被挤压，尝试向上逃逸
		int cloudCount = CA::CountTouching(ctx, CellMatType::MAT_CA_CLOUD);
		if (cloudCount >= 2 && CA::Chance(0.3f))
		{
			if (!CA::SwapIfEmpty(ctx, Dir::Up))
			{
				if (!CA::SwapIfEmpty(ctx, Dir::NE))
					CA::SwapIfEmpty(ctx, Dir::NW);
			}
		}
	}

	// 规则5: Falling时周围空格>6向斜上飘
	if (state == 1)
	{
		for (int attempt = 0; attempt < 4; ++attempt)
		{
			int emptyCount = CA::CountTouchingIf(ctx, [](const Cell& c) { return c.IsEmpty(); });
			if (emptyCount > 6)
			{
				CADir diagUp = (rand() % 2 == 0) ? Dir::NE : Dir::NW;
				CA::SwapIfEmpty(ctx, diagUp);
			}
		}
	}

	// 规则6: 改为检查 MAT_CA_CLOUD
	if (CA::IsType(ctx, Dir::Left, CellMatType::MAT_CA_CLOUD) &&
		CA::IsType(ctx, Dir::Right, CellMatType::MAT_CA_CLOUD) &&
		CA::IsType(ctx, Dir::Up, CellMatType::MAT_CA_CLOUD) &&
		CA::IsEmpty(ctx, Dir::Down))
	{
		CA::Swap(ctx, Dir::Up);
	}
}

void CABehaviors::Update_Steam(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	// 入口 mark dirty
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	// ========================================================================
	// 规则1: 主要水平漂移
	// 随机选一个水平方向，目标为空则移动
	// ========================================================================
	CA::SwapIfEmpty(ctx, Dir::RandomH());

	// ========================================================================
	// 规则2: 密度压力 — 周围云粒子越多越想上浮
	// 统计8邻居中的云数量，超过阈值时尝试向上
	// ========================================================================
	int cloudCount = CA::CountTouching(ctx, CellMatType::MAT_CA_SNOW);
	if (cloudCount >= 3)
	{
		// 先正上，不行再斜上
		if (!CA::SwapIfEmpty(ctx, Dir::Up))
		{
			if (!CA::SwapIfEmpty(ctx, Dir::NE))
				CA::SwapIfEmpty(ctx, Dir::NW);
		}
	}

	// ========================================================================
	// 规则3: 上方为空时有小概率自然上浮（保持漂浮感）
	// ========================================================================
	if (CA::Chance(0.1f))
	{
		CA::SwapIfEmpty(ctx, Dir::Up);
	}

	// ========================================================================
	// 规则4: 下方为空时以小概率抵抗重力（不是完全不下落，而是大概率抵抗）
	// 若下方为空，80%概率向上或水平移动而不是下落
	// ========================================================================
	if (CA::IsEmpty(ctx, Dir::Down))
	{
		if (CA::Chance(0.8f))
		{
			// 抵抗重力：优先水平，其次斜上
			if (!CA::SwapIfEmpty(ctx, Dir::RandomH()))
				CA::SwapIfEmpty(ctx, (rand() % 2 == 0) ? Dir::NE : Dir::NW);
		}
		// 剩余20%概率：什么都不做，让其自然轻微下沉
	}
}

// ============================================================
//  火焰
//  规则：
//    - ra 存剩余寿命，每帧 -1，耗尽后消亡
//    - 以一定概率向 8 邻居中的可燃物蔓延（copyInto）
//    - 偶尔向上产生烟雾
// ============================================================
void CABehaviors::Update_Fire(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);

    int life = CA::GetRA(ctx);
    if (life <= 0)
    {
        CA::ChangeInto(ctx, CellMatType::MAT_EMPTY); // 替换成烟雾类型
        return;
    }
    CA::SetRA(ctx, life - 1);

    // 向可燃邻居蔓延
    if (CA::Chance(0.1f))
    {
        CA::CopyIntoIf(ctx, Dir::Random8(), [](const Cell& c) {
            return CellMatManager::GetMaterialDef(c.m_type.load()).m_isFlammable;
        });
    }

    // 向上产生烟雾
    if (CA::Chance(0.05f))
    {
        if (CA::CopyIntoIf(ctx, Dir::Up, [](const Cell& c) { return c.IsEmpty(); }))
        {
            // CA::ChangeInto(ctx, CellMatType::MAT_SMOKE, Dir::Up);
        }
    }
}
void CABehaviors::Update_Smoke(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	// 40%帧直接跳过，烟的整体节奏比云慢
	if (!CA::Chance(0.4f)) return;

	// 规则1: 判断状态
	bool aboveInBounds = ctx.map->IsInBounds(ctx.x, ctx.y + 1);
	bool aboveEmpty = aboveInBounds && CA::IsEmpty(ctx, Dir::Up);
	int state = aboveEmpty ? 1 : 2;

	// 规则2: Floating — 1/4概率触发，每次只移动一格
	if (state == 1)
	{
		if (rand() % 4 == 0)
		{
			const CADir floatDirs[3] = { Dir::Up, Dir::NW, Dir::NE };
			CA::SwapIfEmpty(ctx, floatDirs[rand() % 3]);
		}
	}

	// 规则3: Stationary
	if (state == 2)
	{
		int originalX = ctx.x;
		int originalY = ctx.y;

		if (CA::IsEmpty(ctx, Dir::Left) && CA::IsEmpty(ctx, Dir::Right))
		{
			CA::SwapIfEmpty(ctx, Dir::RandomH());
		}

		CAContext ctxOriginal(originalX, originalY, map);
		CADir diagDown = Dir::Down + CADir((rand() % 2 == 0) ? -1 : 1, 0);

		for (int i = 0; i < 2; ++i)
		{
			if (CA::IsEmpty(ctxOriginal, Dir::Up) &&
				CA::IsType(ctxOriginal, diagDown, CellMatType::MAT_CA_SMOKE))
			{
				CA::Swap(ctxOriginal, Dir::Up);
			}
			else break;
		}

		// 被挤压时小概率上浮
		int smokeCount = CA::CountTouching(ctx, CellMatType::MAT_CA_SMOKE);
		if (smokeCount >= 2 && CA::Chance(0.15f))
		{
			if (!CA::SwapIfEmpty(ctx, Dir::Up))
			{
				if (!CA::SwapIfEmpty(ctx, Dir::NE))
					CA::SwapIfEmpty(ctx, Dir::NW);
			}
		}
	}

	// 规则5: 周围空格>5时向斜上飘一次（不循环）
	if (state == 1)
	{
		int emptyCount = CA::CountTouchingIf(ctx, [](const Cell& c) { return c.IsEmpty(); });
		if (emptyCount > 5)
		{
			CADir diagUp = (rand() % 2 == 0) ? Dir::NE : Dir::NW;
			CA::SwapIfEmpty(ctx, diagUp);
		}
	}

	// 规则6: 左右下都是烟且上方为空 → 与下方交换
	if (CA::IsType(ctx, Dir::Left, CellMatType::MAT_CA_SMOKE) &&
		CA::IsType(ctx, Dir::Right, CellMatType::MAT_CA_SMOKE) &&
		CA::IsType(ctx, Dir::Down, CellMatType::MAT_CA_SMOKE) &&
		CA::IsEmpty(ctx, Dir::Up))
	{
		CA::Swap(ctx, Dir::Down);
	}
}
// ============================================================
//  植物生长
//  规则：
//    - 大部分帧跳过（性能优化）
//    - 找到相邻水格后，向该方向 copyInto 扩散
// ============================================================
void CABehaviors::Update_Plant(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CAContext ctx(worldX, worldY, map);
	// 没有水时极低概率慢速生长，有水时正常速度
	bool hasTouchingWater = CA::IsTouching(ctx, CellMatType::MAT_WATER);
	float activateChance = hasTouchingWater ? 0.05f : 0.005f;
	if (!CA::Chance(activateChance)) return;
	// 有活动可能才 mark dirty
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);
	// 密度限制
	int plantCount = CA::CountTouching(ctx, CellMatType::MAT_CA_PLANT);
	if (plantCount >= 5) return;
	// 方向权重：Up=40%, NW/NE=20%each, Left/Right=10%each
	// 用加权随机选方向
	struct WeightedDir { CADir dir; int weight; };
	const WeightedDir weightedDirs[5] = {
		{ Dir::Up,    70 },
		{ Dir::NW,    13 },
		{ Dir::NE,    13 },
		{ Dir::Left,  2 },
		{ Dir::Right, 2 },
	};
	// 先筛选出目标是水或空（无水时可以往空格长）的方向，按权重随机选一个
	int totalWeight = 0;
	for (int i = 0; i < 5; ++i)
	{
		const auto& wd = weightedDirs[i];
		bool targetIsWater = CA::IsType(ctx, wd.dir, CellMatType::MAT_WATER);
		bool targetIsEmpty = CA::IsEmpty(ctx, wd.dir);
		if (hasTouchingWater ? targetIsWater : targetIsEmpty)
			totalWeight += wd.weight;
	}
	if (totalWeight == 0) return;
	int roll = rand() % totalWeight;
	int accumulated = 0;
	for (int i = 0; i < 5; ++i)
	{
		const auto& wd = weightedDirs[i];
		bool targetIsWater = CA::IsType(ctx, wd.dir, CellMatType::MAT_WATER);
		bool targetIsEmpty = CA::IsEmpty(ctx, wd.dir);
		if (hasTouchingWater ? targetIsWater : targetIsEmpty)
		{
			accumulated += wd.weight;
			if (roll < accumulated)
			{
				CA::CopyInto(ctx, wd.dir);
				if (CA::Chance(0.002f))
				{
					// 随机色系：红/橙/黄/紫/粉
					static const Rgba8 flowerColors[] = {
						Rgba8(255, 100, 180, 255),  // 樱花粉
						Rgba8(180,  80, 255, 255),  // 薰衣草紫
						Rgba8(80,  200, 255, 255),  // 天蓝
						Rgba8(255, 200,  80, 255),  // 琥珀金
						Rgba8(80,  255, 200, 255),  // 薄荷青
					};
					Rgba8 chosenColor = flowerColors[rand() % 5];
					int tx = ctx.x + wd.dir.dx;  // dir 是刚才 CopyInto 用的方向
					int ty = ctx.y + wd.dir.dy;
					if (ctx.map->IsInBounds(tx, ty))
					{
						ctx.map->GetCell(tx, ty).m_type.store(CellMatType::MAT_CA_FLOWER);
						ctx.map->GetCell(tx, ty).m_color = chosenColor;
						CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, tx, ty);
					}
				}
				return;
			}
		}
	}
}

// 辅助函数：把颜色往白色方向淡一步
static Rgba8 FadeTowardWhite(Rgba8 color, float fadeFactor)
{
	auto lerp = [](uint8_t a, uint8_t b, float t) -> uint8_t {
		return static_cast<uint8_t>(a + (b - a) * t);
		};
	return Rgba8(
		lerp(color.r, 255, fadeFactor),
		lerp(color.g, 255, fadeFactor),
		lerp(color.b, 255, fadeFactor),
		255 // 同时变透明
	);
}

// 辅助函数：颜色是否已经足够淡（接近白色），用于判断是否停止扩散
static bool IsTooFaded(Rgba8 color)
{
	int avg = (color.r + color.g + color.b) / 3;
	return avg > 220 || color.a < 90;
}

void CABehaviors::Update_Flower(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	// 颜色已经很淡，停止扩散，只等寿命结束
	if (IsTooFaded(cell.m_color)) return;

	if (!CA::Chance(0.03f)) return;

	// 向全8方向尝试扩散，目标是空格或植物
	for (const auto& dir : Dir::All8)
	{
		bool targetIsEmpty = CA::IsEmpty(ctx, dir);
		bool targetIsPlant = CA::IsType(ctx, dir, CellMatType::MAT_CA_PLANT);
		if (!targetIsEmpty && !targetIsPlant) continue;

		if (!CA::Chance(0.3f)) continue;

		// CopyInto 后立刻取出目标 cell，把颜色改淡一步
		int tx = ctx.x + dir.dx;
		int ty = ctx.y + dir.dy;
		if (!ctx.map->IsInBounds(tx, ty)) continue;

		CA::CopyInto(ctx, dir);
		ctx.map->GetCell(tx, ty).m_color = FadeTowardWhite(cell.m_color, 0.35f);

		CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, tx, ty);
	}
}

// ============================================================
// Vine
// ============================================================
static bool IsVineMain(Rgba8 color)
{
	// 亮度低 = 主体（深绿），亮度高 = 须末端（浅绿）
	int brightness = (color.r + color.g + color.b) / 3;
	return brightness < 100;
}

static bool IsVineTooLong(Rgba8 color)
{
	// 颜色太浅时停止向下生长
	int brightness = (color.r + color.g + color.b) / 3;
	return brightness > 180;
}

static Rgba8 FadeVineLight(Rgba8 color, float factor)
{
	auto lerp = [](uint8_t a, uint8_t b, float t) -> uint8_t {
		return static_cast<uint8_t>(a + (b - a) * t);
		};
	return Rgba8(
		lerp(color.r, 160, factor),
		lerp(color.g, 210, factor),
		lerp(color.b, 130, factor),
		color.a
	);
}

void CABehaviors::Update_Vine(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	if (!CA::Chance(0.04f)) return;

	// ========================================================================
	// extraData 编码：
	//  0          = 未初始化主体
	// -1          = 主体，不能长须
	// >= 1000     = 主体，可以长须，值 = 1000 + maxLength
	// < 0 且!=-1  = 须，值 = -(depth * 10000 + maxLength)
	// ========================================================================

	// 未初始化：一次性决定能不能长须及最大长度
	if (cell.m_extraData == 0)
	{
		if (CA::Chance(0.35f))
		{
			int maxLength = 5 + rand() % 20;  // 5~24格
			cell.m_extraData = 1000 + maxLength;
		}
		else
		{
			cell.m_extraData = -1;
		}
		return;
	}

	// 主体，不能长须
	if (cell.m_extraData == -1) return;

	// 主体，可以长须
	if (cell.m_extraData >= 1000)
	{
		int maxLength = cell.m_extraData - 1000;

		if (!CA::IsEmpty(ctx, Dir::Down)) return;
		if (!CA::Chance(0.05f)) return;

		CA::CopyInto(ctx, Dir::Down);
		int tx = ctx.x;
		int ty = ctx.y - 1;
		if (ctx.map->IsInBounds(tx, ty))
		{
			Cell& newCell = ctx.map->GetCell(tx, ty);
			newCell.m_extraData = -(1 * 10000 + maxLength);
			newCell.m_color = Rgba8(70, 160, 100, 255);
		}
		return;
	}

	// 须
	if (cell.m_extraData < -1)
	{
		int packed = -cell.m_extraData;
		int depth = packed / 10000;
		int maxLength = packed % 10000;

		if (depth >= maxLength) return;  // 达到最大长度

		if (!CA::IsEmpty(ctx, Dir::Down)) return;
		if (!CA::Chance(0.4f)) return;

		CA::CopyInto(ctx, Dir::Down);
		int tx = ctx.x;
		int ty = ctx.y - 1;
		if (ctx.map->IsInBounds(tx, ty))
		{
			Cell& newCell = ctx.map->GetCell(tx, ty);
			newCell.m_extraData = -((depth + 1) * 10000 + maxLength);

			float t = depth / float(maxLength);
			newCell.m_color = Rgba8(
				static_cast<uint8_t>(50 + t * 100),
				static_cast<uint8_t>(130 + t * 60),
				static_cast<uint8_t>(60 + t * 80),
				255
			);
		}
	}
}

void CABehaviors::Update_Seed(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

    CAContext ctx(worldX, worldY, map);

    // ========================================================================
    // extraData：
    //  0  = 下落中
    // >0  = 静止帧计数，累积到阈值后发芽
    // ========================================================================

    bool belowInBounds = ctx.map->IsInBounds(ctx.x, ctx.y - 1);
    bool belowEmpty    = belowInBounds && CA::IsEmpty(ctx, Dir::Down);

	// ========================================================================
	// 下落逻辑（带侧向滑落，可沉入密度小的液体）
	// ========================================================================
	if (belowEmpty || CA::PhysicsTypeAt(ctx, Dir::Down) == PhyType::PHY_LIQUID)
	{
		cell.m_extraData = 0;
		CA::SwapIfLessDense(ctx, Dir::Down);
		return;
	}

	// 正下方不为空，尝试侧向滑落（SW/SE）
	bool swPassable = CA::IsEmpty(ctx, Dir::SW) ||
		CA::PhysicsTypeAt(ctx, Dir::SW) == PhyType::PHY_LIQUID;
	bool sePassable = CA::IsEmpty(ctx, Dir::SE) ||
		CA::PhysicsTypeAt(ctx, Dir::SE) == PhyType::PHY_LIQUID;

	if (swPassable || sePassable)
	{
		cell.m_extraData = 0;
		if (swPassable && sePassable)
			CA::SwapIfLessDense(ctx, (rand() % 2 == 0) ? Dir::SW : Dir::SE);
		else if (swPassable)
			CA::SwapIfLessDense(ctx, Dir::SW);
		else
			CA::SwapIfLessDense(ctx, Dir::SE);
		return;
	}

    // ========================================================================
    // 静止：检查下方材质，决定是否能发芽
    // ========================================================================
    if (!belowInBounds) return;

    CellMatType belowType = CA::TypeAt(ctx, Dir::Down);
    bool canSprout = (belowType == CellMatType::MAT_SOIL      ||
                      belowType == CellMatType::MAT_CA_PLANT  ||
                      belowType == CellMatType::MAT_CA_FLOWER ||
					  belowType == CellMatType::MAT_CA_VINE);

    if (!canSprout)
    {
        cell.m_extraData = 0;  // 不满足条件，重置计数，保持种子状态
        return;
    }

    // 满足条件，累积静止帧
    cell.m_extraData += 1;

    bool hasTouchingWater = CA::IsTouching(ctx, CellMatType::MAT_WATER);
    int sproutThreshold   = hasTouchingWater ? 30 : 120;

    if (cell.m_extraData >= sproutThreshold)
    {
        CA::ChangeInto(ctx, CellMatType::MAT_CA_PLANT);
    }
}

void CABehaviors::Update_FireCA(Cell& cell, int worldX, int worldY, BaseMap* map)
{
	CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(map, worldX, worldY);

	CAContext ctx(worldX, worldY, map);

	// 整体激活概率，控制火焰节奏
	if (!CA::Chance(0.5f)) return;

	// extraData：摆动方向，0=未初始化，1=右，-1=左
	if (cell.m_extraData == 0)
	{
		cell.m_extraData = (rand() % 2 == 0) ? 1 : -1;
	}

	// 随机切换摆动方向
	if (CA::Chance(0.15f))
	{
		cell.m_extraData = -cell.m_extraData;
	}

	int swingDir = cell.m_extraData;
	CADir diagUp = (swingDir == 1) ? Dir::NE : Dir::NW;
	CADir diagUpOpp = (swingDir == 1) ? Dir::NW : Dir::NE;

	// 优先往摆动方向斜上
	if (CA::SwapIfEmpty(ctx, diagUp)) return;

	// 正上
	if (CA::SwapIfEmpty(ctx, Dir::Up)) return;

	// 反方向斜上
	if (CA::SwapIfEmpty(ctx, diagUpOpp))
	{
		cell.m_extraData = -cell.m_extraData;
		return;
	}

	// 横向漂移
	CADir sideDir = (swingDir == 1) ? Dir::Right : Dir::Left;
	if (!CA::SwapIfEmpty(ctx, sideDir))
	{
		cell.m_extraData = -cell.m_extraData;
		CA::SwapIfEmpty(ctx, (swingDir == 1) ? Dir::Left : Dir::Right);
	}
}

// ============================================================
//  Conway 生命游戏
// ============================================================
void CABehaviors::Update_GameOfLife_Alive(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);
    int n = CA::CountTouching(ctx, CellMatType::MAT_CA_SNOW); // 替换成你的 Alive 类型
    if (n < 2 || n > 3)
        CA::ChangeInto(ctx, CellMatType::MAT_EMPTY);
}

void CABehaviors::Update_GameOfLife_Dead(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);
    int n = CA::CountTouching(ctx, CellMatType::MAT_CA_SNOW);
    if (n == 3)
        CA::ChangeInto(ctx, CellMatType::MAT_CA_SNOW); // 替换成你的 Alive 类型
}
