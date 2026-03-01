#include "CAPrimitives.hpp"
#include "CellMatManager.hpp"
#include <cstdlib>

// ============================================================
//  Dir
// ============================================================

CADir Dir::RandomH()
{
    return (rand() % 2 == 0) ? Left : Right;
}

CADir Dir::Random8()
{
    return All8[rand() % 8];
}

// ============================================================
//  CA — 工具
// ============================================================

bool CA::Chance(float probability)
{
    return (rand() / static_cast<float>(RAND_MAX)) < probability;
}

bool CA::EveryNFrames(int frameCounter, int n)
{
    return (frameCounter % n) == 0;
}

// ============================================================
//  CA — 查询
// ============================================================

CellMatType CA::TypeAt(const CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty))
        return CellMatType::MAT_STATIC_FILL; // 边界视为实心
    return ctx.map->GetCell(tx, ty).m_type.load();
}

bool CA::IsEmpty(const CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return false;
    return ctx.map->GetCell(tx, ty).IsEmpty();
}

bool CA::IsType(const CAContext& ctx, CADir dir, CellMatType type)
{
    return TypeAt(ctx, dir) == type;
}

PhyType CA::PhysicsTypeAt(const CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty))
        return PhyType::PHY_STATIC_SOLID;
    return CellMatManager::GetMaterialDef(
        ctx.map->GetCell(tx, ty).m_type.load()).m_physicsType;
}

bool CA::IsTouching(const CAContext& ctx, CellMatType type)
{
    for (const auto& d : Dir::All8)
    {
        if (IsType(ctx, d, type)) return true;
    }
    return false;
}

int CA::CountTouching(const CAContext& ctx, CellMatType type)
{
    int count = 0;
    for (const auto& d : Dir::All8)
    {
        if (IsType(ctx, d, type)) ++count;
    }
    return count;
}

// ============================================================
//  CA — 动词
// ============================================================

bool CA::Swap(CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return false;

    std::swap(ctx.map->GetCell(ctx.x, ctx.y), ctx.map->GetCell(tx, ty));
    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(ctx.map, ctx.x, ctx.y);

    ctx.x = tx;
    ctx.y = ty;

    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(ctx.map, ctx.x, ctx.y);
    return true;
}

bool CA::SwapIfEmpty(CAContext& ctx, CADir dir)
{
    return SwapIf(ctx, dir, [](const Cell& c) { return c.IsEmpty(); });
}

bool CA::SwapIfLessDense(CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return false;

    const Cell& me     = ctx.map->GetCell(ctx.x, ctx.y);
    const Cell& target = ctx.map->GetCell(tx, ty);
    float myDensity  = CellMatManager::GetMaterialDef(me.m_type.load()).m_density;
    float tgtDensity = CellMatManager::GetMaterialDef(target.m_type.load()).m_density;
    if (myDensity <= tgtDensity) return false;
    return Swap(ctx, dir);
}

bool CA::CopyInto(CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return false;

    ctx.map->GetCell(tx, ty).SetToType(ctx.map->GetCell(ctx.x, ctx.y).m_type.load());
    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(ctx.map, tx, ty);
    return true;
}

void CA::ChangeInto(CAContext& ctx, CellMatType newType, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return;

    ctx.map->GetCell(tx, ty).SetToType(newType);
    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(ctx.map, tx, ty);
}

void CA::Destroy(CAContext& ctx, CADir dir)
{
    int tx = ctx.x + dir.dx;
    int ty = ctx.y + dir.dy;
    if (!ctx.map->IsInBounds(tx, ty)) return;

    ctx.map->GetCell(tx, ty).SetEmpty();
    CellBehaviorSystemInChunk::MarkChunkDirtyWithNeighbors(ctx.map, tx, ty);
}

// ============================================================
//  CA — extraData 寄存器
// ============================================================

int CA::GetRA(const CAContext& ctx)
{
    return ctx.map->GetCell(ctx.x, ctx.y).m_lifeCountDown;
}

void CA::SetRA(CAContext& ctx, int v)
{
    ctx.map->GetCell(ctx.x, ctx.y).m_lifeCountDown = v;
}

int CA::GetRB(const CAContext& ctx)
{
    return ctx.map->GetCell(ctx.x, ctx.y).m_flameCountDown;
}

void CA::SetRB(CAContext& ctx, int v)
{
    ctx.map->GetCell(ctx.x, ctx.y).m_flameCountDown = v;
}
