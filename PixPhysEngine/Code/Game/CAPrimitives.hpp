#pragma once
#include "Cell.hpp"
#include "CellMatDef.hpp"
#include "BaseMap.hpp"
#include "CellBehaviorSystemInChunk.hpp"
#include <cstdlib>

// ============================================================
//  CAContext
//  每次 UpdateCellularAutomaton 调用时构造，持有当前 cell 的
//  可变世界坐标（swap 后自动跟踪），作为所有原语函数的入参。
// ============================================================
struct CAContext
{
    int      x;
    int      y;
    BaseMap* map;

    CAContext(int worldX, int worldY, BaseMap* m)
        : x(worldX), y(worldY), map(m) {}

    Cell&       Me()       { return map->GetCell(x, y); }
    const Cell& Me() const { return map->GetCell(x, y); }
};

// ============================================================
//  CADir  —  方向向量
//  对标 Sandspiel Studio 的 Vector 块。
//  使用独立 struct 而非引擎的 IntVec2，避免不必要的头文件依赖。
// ============================================================
struct CADir
{
    int dx, dy;

    constexpr CADir(int x, int y) : dx(x), dy(y) {}
    constexpr CADir operator+(const CADir& o) const { return { dx + o.dx, dy + o.dy }; }
    constexpr CADir operator*(int s)           const { return { dx * s,   dy * s   }; }
    constexpr bool  operator==(const CADir& o)  const { return dx == o.dx && dy == o.dy; }
};

// ============================================================
//  Dir  —  方向常量与工具函数
// ============================================================
namespace Dir
{
    // 基本方向
    constexpr CADir Me     {  0,  0 };
    constexpr CADir Up     {  0,  1 };
    constexpr CADir Down   {  0, -1 };
    constexpr CADir Left   { -1,  0 };
    constexpr CADir Right  {  1,  0 };
    constexpr CADir NE     {  1,  1 };
    constexpr CADir NW     { -1,  1 };
    constexpr CADir SE     {  1, -1 };
    constexpr CADir SW     { -1, -1 };

    // 常用方向集合
    constexpr CADir All8[8]      = { Up, Down, Left, Right, NE, NW, SE, SW };
    constexpr CADir Cardinal4[4] = { Up, Down, Left, Right };
    constexpr CADir Below3[3]    = { Down, SW, SE };

    // 旋转（constexpr，可在编译期使用）
    constexpr CADir RotateCW (CADir d) { return {  d.dy, -d.dx }; }
    constexpr CADir RotateCCW(CADir d) { return { -d.dy,  d.dx }; }

    // 运行期随机方向（实现在 CAPrimitives.cpp）
    CADir RandomH();   // 随机返回 Left 或 Right
    CADir Random8();   // 随机返回 All8 中的一个
}

// ============================================================
//  CA  —  原语函数
//
//  普通函数：声明在此，实现在 CAPrimitives.cpp
//  模板函数：实例化需要在调用点可见，因此完整定义保留在此
// ============================================================
namespace CA
{
    // ----------------------------------------------------------
    //  工具
    // ----------------------------------------------------------
    bool Chance(float probability);              // [0,1] 概率返回 true
    bool EveryNFrames(int frameCounter, int n);  // frameCounter % n == 0

    // ----------------------------------------------------------
    //  查询
    // ----------------------------------------------------------
    CellMatType TypeAt       (const CAContext& ctx, CADir dir);
    bool        IsEmpty      (const CAContext& ctx, CADir dir);
    bool        IsType       (const CAContext& ctx, CADir dir, CellMatType type);
    PhyType     PhysicsTypeAt(const CAContext& ctx, CADir dir);
    bool        IsTouching   (const CAContext& ctx, CellMatType type);
    int         CountTouching(const CAContext& ctx, CellMatType type);

    template<typename Pred>
    int CountTouchingIf(const CAContext& ctx, Pred predicate)
    {
        int count = 0;
        for (const auto& d : Dir::All8)
        {
            int tx = ctx.x + d.dx;
            int ty = ctx.y + d.dy;
            if (ctx.map->IsInBounds(tx, ty) && predicate(ctx.map->GetCell(tx, ty)))
                ++count;
        }
        return count;
    }

    template<typename Pred>
    CADir FindTouchingDir(const CAContext& ctx, Pred predicate)
    {
        for (const auto& d : Dir::All8)
        {
            int tx = ctx.x + d.dx;
            int ty = ctx.y + d.dy;
            if (ctx.map->IsInBounds(tx, ty) && predicate(ctx.map->GetCell(tx, ty)))
                return d;
        }
        return Dir::Me;
    }

    // ----------------------------------------------------------
    //  动词
    // ----------------------------------------------------------
    bool Swap            (CAContext& ctx, CADir dir);
    bool SwapIfEmpty     (CAContext& ctx, CADir dir);
    bool SwapIfLessDense (CAContext& ctx, CADir dir);
    bool CopyInto        (CAContext& ctx, CADir dir);
    void ChangeInto      (CAContext& ctx, CellMatType newType, CADir dir = Dir::Me);
    void Destroy         (CAContext& ctx, CADir dir = Dir::Me);

    template<typename Pred>
    bool SwapIf(CAContext& ctx, CADir dir, Pred predicate)
    {
        int tx = ctx.x + dir.dx;
        int ty = ctx.y + dir.dy;
        if (!ctx.map->IsInBounds(tx, ty))         return false;
        if (!predicate(ctx.map->GetCell(tx, ty))) return false;
        return Swap(ctx, dir);
    }

    template<typename Pred>
    bool CopyIntoIf(CAContext& ctx, CADir dir, Pred predicate)
    {
        int tx = ctx.x + dir.dx;
        int ty = ctx.y + dir.dy;
        if (!ctx.map->IsInBounds(tx, ty))         return false;
        if (!predicate(ctx.map->GetCell(tx, ty))) return false;
        return CopyInto(ctx, dir);
    }

    // ----------------------------------------------------------
    //  extraData 寄存器（对标 Sandspiel ra / rb）
    //  当前借用 m_lifeCountDown / m_flameCountDown；
    //  若 Cell 增加专用字段，只改这四个函数即可。
    // ----------------------------------------------------------
    int  GetRA(const CAContext& ctx);
    void SetRA(CAContext& ctx, int v);
    int  GetRB(const CAContext& ctx);
    void SetRB(CAContext& ctx, int v);

} // namespace CA
