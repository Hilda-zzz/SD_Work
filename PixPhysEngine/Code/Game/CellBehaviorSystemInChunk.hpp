#pragma once
#include "Cell.hpp"
#include "CellChunk.hpp"

class SandboxMap;

// 新的基于Chunk的Cell行为系统
class CellBehaviorSystemInChunk
{
public:
	// === Update Entrance ===
	static void UpdateCell(Cell& cell, int worldX, int worldY, SandboxMap* map);

	// === 移动处理（新版本 - chunk aware） ===
	static void HandleMoveSolidMovement(
		int& currentX, int& currentY,
		int targetX, int targetY,
		SandboxMap* map
	);

	static void HandleLiquidMovement(
		int& currentX, int& currentY,
		int targetX, int targetY,
		SandboxMap* map
	);

	// === 碰撞处理（复用旧的） ===
	static bool HandleMSvsMSCollision(
		int fromX, int fromY,
		int toX, int toY,
		Cell& movingCell,
		Cell& targetCell
	);

	// === 物理效果应用（复用旧的） ===
	static void ApplyGravity(Cell& cell, float deltaTime);
	static void ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY);
	static void ClampVelocity(Cell& cell, float maxSpeed);

private:
	// === 物理类型特化处理 ===
	static void UpdateMoveSolid(Cell& cell, int worldX, int worldY, SandboxMap* map);
	static void UpdateLiquid(Cell& cell, int worldX, int worldY, SandboxMap* map);

	static void MarkChunkDirtyWithNeighbors(SandboxMap* map, int worldX, int worldY);

	// === Chunk相关辅助函数 ===
	//static void MoveCellWithinChunk(
	//	CellChunk* chunk,
	//	int fromLocalX, int fromLocalY,
	//	int toLocalX, int toLocalY
	//);

	//static void MoveCellAcrossChunk(
	//	SandboxMap* map,
	//	int fromWorldX, int fromWorldY,
	//	int toWorldX, int toWorldY,
	//	Cell& movingCell
	//);

	//static bool TryMoveCell(
	//	SandboxMap* map,
	//	int& currentX, int& currentY,
	//	int targetX, int targetY,
	//	Cell& cell
	//);

	static void UpdateAccumulatedMovement(
		int oldX, int oldY,
		int newX, int newY,
		SandboxMap* map
	);

	static void UpdateAccumulatedMovementLiquid(int oldX, int oldY, int newX, int newY, SandboxMap* map);
};