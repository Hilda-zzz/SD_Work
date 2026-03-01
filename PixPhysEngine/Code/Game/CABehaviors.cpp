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
    CAContext ctx(worldX, worldY, map);

    // 根据正下方是否为空决定当前状态
    CA::SetRA(ctx, CA::IsEmpty(ctx, Dir::Down) ? 1 : 2);

    if (CA::GetRA(ctx) == 1)
    {
        // Falling：尝试最多 3 次，随机选落点
        const CADir fallDirs[3] = { Dir::Down, Dir::SW, Dir::SE };
        for (int i = 0; i < 3; ++i)
        {
            if (!CA::Chance(0.33f)) continue;
            if (CA::SwapIfEmpty(ctx, fallDirs[rand() % 3])) return;
        }
        return;
    }

    // Stationary：左右都空时随机水平漂移
    if (CA::IsEmpty(ctx, Dir::Left) && CA::IsEmpty(ctx, Dir::Right))
    {
        CA::Swap(ctx, Dir::RandomH());
        return;
    }

    // Stationary：斜上有同类且正下为空时，下落最多 2 格
    CADir diagUp = Dir::Up + Dir::RandomH();
    for (int i = 0; i < 2; ++i)
    {
        if (CA::IsEmpty(ctx, Dir::Down) && CA::IsType(ctx, diagUp, CellMatType::MAT_CA_SAND))
            CA::Swap(ctx, Dir::Down);
        else
            break;
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

// ============================================================
//  烟雾
//  规则：
//    - ra 存寿命，耗尽消失
//    - 优先向上漂浮，其次斜上，再次随机横向
// ============================================================
void CABehaviors::Update_Smoke(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);

    int life = CA::GetRA(ctx);
    if (life <= 0) { CA::Destroy(ctx); return; }
    CA::SetRA(ctx, life - 1);

    if (CA::SwapIfEmpty(ctx, Dir::Up))                                    return;
    if (CA::SwapIfEmpty(ctx, CA::Chance(0.5f) ? Dir::NE : Dir::NW))      return;
    if (CA::Chance(0.3f)) CA::SwapIfEmpty(ctx, Dir::RandomH());
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

    if (!CA::Chance(0.02f)) return;

    CADir growDir = CA::FindTouchingDir(ctx, [](const Cell& c) {
        return c.m_type.load() == CellMatType::MAT_WATER;
    });

    if (growDir == Dir::Me) return; // 没有相邻水格
    CA::CopyInto(ctx, growDir);
}

// ============================================================
//  酸液
//  规则：
//    - 密度驱动向下沉降（同液体）
//    - 接触可溶物时概率性腐蚀（双方同时消亡）
//    - 横向流动
// ============================================================
void CABehaviors::Update_Acid(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);

    if (CA::SwapIfLessDense(ctx, Dir::Down)) return;

    if (CA::Chance(0.3f))
    {
        CADir eatDir = CA::FindTouchingDir(ctx, [](const Cell& c) {
            return CellMatManager::GetMaterialDef(c.m_type.load()).m_interaction.m_isSoluble;
        });

        if (!(eatDir == Dir::Me))
        {
            CA::Destroy(ctx, eatDir);
            CA::Destroy(ctx);
            return;
        }
    }

    CA::SwapIfLessDense(ctx, Dir::RandomH());
}

// ============================================================
//  Conway 生命游戏
// ============================================================
void CABehaviors::Update_GameOfLife_Alive(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);
    int n = CA::CountTouching(ctx, CellMatType::MAT_CA_SAND); // 替换成你的 Alive 类型
    if (n < 2 || n > 3)
        CA::ChangeInto(ctx, CellMatType::MAT_EMPTY);
}

void CABehaviors::Update_GameOfLife_Dead(Cell& cell, int worldX, int worldY, BaseMap* map)
{
    CAContext ctx(worldX, worldY, map);
    int n = CA::CountTouching(ctx, CellMatType::MAT_CA_SAND);
    if (n == 3)
        CA::ChangeInto(ctx, CellMatType::MAT_CA_SAND); // 替换成你的 Alive 类型
}
