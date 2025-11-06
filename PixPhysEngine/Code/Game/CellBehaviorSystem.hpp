//#pragma once
//#include "Cell.hpp"
//
//class SandboxMap;
//
//class CellBehaviorSystem
//{
//public:
//	// === Update Entrance ===
//	static void UpdateCell(Cell& cell, int x, int y, SandboxMap* map);
//
//	// === 移动处理 ===
//	static bool TryMoveCell(Cell& cell, int fromX, int fromY, int toX, int toY, SandboxMap* map);
//
//	static void HandleMoveSolideMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map);
//	static bool HandleMSvsMSCollision(int fromX, int fromY, int toX, int toY,
//		Cell& movingCell, Cell& targetCell); // bool marks if the cell activate another cell
//
//	static void HandleLiquidMovement(int& currentX, int& currentY, int targetX, int targetY, SandboxMap* map);
//
//	// === 碰撞处理 ===
//	static void HandleCollision(Cell& movingCell, Cell& targetCell, int deltaX, int deltaY);
//	static void HandleParticleInteraction(Cell& particle1, Cell& particle2, int deltaX, int deltaY);
//
//	// === 物理效果应用 ===
//	static void ApplyGravity(Cell& cell, float deltaTime);
//	static void ApplyVelocityDamping(Cell& cell);
//	static void ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY); //Move Solid
//	static void ApplyLiquidCollisionPhysics(Cell& cell, int deltaX, int deltaY);
//
//	// === 特殊移动逻辑 ===
//	static bool TryVerticalMovement(Cell& cell, int& currentX, int& currentY, SandboxMap* map);
//	static bool TryDiagonalMovement(Cell& cell, int& currentX, int& currentY, SandboxMap* map);
//	static bool TryHorizontalMovement(Cell& cell, int& currentX, int& currentY, SandboxMap* map);
//
//	static void ClampVelocity(Cell& cell, float maxSpeed);
//private:
//	// === 物理类型特化处理 ===
//	static void UpdateMoveSolid(Cell& cell, int x, int y, SandboxMap* map);
//	static void UpdateLiquid(Cell& cell, int x, int y, SandboxMap* map);
//
//	// === 碰撞细分处理 ===
//	static void HandleSolidToSolidCollision(Cell& movingCell, Cell& targetCell, int deltaX, int deltaY);
//	static void HandleSolidToLiquidCollision(Cell& movingCell, Cell& targetCell, int deltaX, int deltaY);
//	static void HandleLiquidToSolidCollision(Cell& movingCell, Cell& targetCell, int deltaX, int deltaY);
//
//	// === 激活和传播 ===
//	static void TryActivateNeighbor(Cell& movingCell, Cell& targetCell, float impactForce);
//	static void TransferMomentum(Cell& fromCell, Cell& toCell, float transferRatio);
//
//	// === 辅助函数 ===
//	static float CalculateImpactForce(const Cell& cell);
//	
//	static bool ShouldActivateNeighbor(const Cell& movingCell, const Cell& targetCell, float impactForce);
//};