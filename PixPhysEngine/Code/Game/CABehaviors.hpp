#pragma once
struct Cell;
class BaseMap;

// ============================================================
//  CABehaviors
//  每个函数签名与 CellMatDef::m_caUpdateFunc 一致：
//      void (Cell&, int worldX, int worldY, BaseMap*)
//  在材质注册时直接赋值给 m_caUpdateFunc 即可。
// ============================================================
namespace CABehaviors
{
    void Update_Snow            (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Cloud(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Steam(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Fire            (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Smoke           (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Plant           (Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Flower(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Vine(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_Seed(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_FireCA(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_GameOfLife_Alive(Cell& cell, int worldX, int worldY, BaseMap* map);
    void Update_GameOfLife_Dead (Cell& cell, int worldX, int worldY, BaseMap* map);
}
