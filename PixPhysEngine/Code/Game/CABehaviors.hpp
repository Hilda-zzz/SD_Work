#pragma once
#include "Cell.hpp"
#include "BaseMap.hpp"

// ============================================================
//  CABehaviors
//  每个函数签名与 CellMatDef::m_caUpdateFunc 一致：
//      void (Cell&, int worldX, int worldY, BaseMap*)
//  在材质注册时直接赋值给 m_caUpdateFunc 即可。
// ============================================================
namespace CABehaviors
{
    void Update_Snow            (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Fire            (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Smoke           (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Plant           (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Acid            (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_GameOfLife_Alive(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_GameOfLife_Dead (Cell& cell, int worldX, int worldY, BaseMap* map);
}
