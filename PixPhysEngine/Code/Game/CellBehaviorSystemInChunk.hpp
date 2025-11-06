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

	static void UpdateAccumulatedMovement(
		int oldX, int oldY,
		int newX, int newY,
		SandboxMap* map
	);

	static void UpdateAccumulatedMovementLiquid(int oldX, int oldY, int newX, int newY, SandboxMap* map);

	static void GetMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir);
};

template<typename PointCallback>
inline void BresenhamLineExcludeStart(int x0, int y0, int x1, int y1, PointCallback&& callback)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	// 如果起点和终点相同，没有其他点要处理
	if (dx == 0 && dy == 0) {
		return;
	}

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	int x = x0, y = y0;

	// 跳过起点，直接执行第一步
	int e2 = err << 1;
	if (e2 > -dy) {
		err -= dy;
		x += sx;
	}
	if (e2 < dx) {
		err += dx;
		y += sy;
	}

	// 现在开始处理剩余点（包括终点）
	while (true) {
		if (!callback(x, y)) {
			break;
		}

		if (x == x1 && y == y1) break;

		e2 = err << 1;
		if (e2 > -dy) {
			err -= dy;
			x += sx;
		}
		if (e2 < dx) {
			err += dx;
			y += sy;
		}
	}
}