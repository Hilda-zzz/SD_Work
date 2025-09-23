#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <utility>
class SandboxPlayer;

constexpr float GRAVITY = -98.f;           
constexpr float TERMINAL_SPEED = 200.0f; 
constexpr float AIR_RESISTANCE = 0.98f;    
constexpr float SAND_FRICTION = 0.7f;  

constexpr float WATER_HORIZONTAL_SPEED = 50.0f;  
constexpr float WATER_FLOW_DAMPING = 0.8f;      

static const float SPEED_THRESHOLD = 5.0f;

class SandboxMap {
public:
	SandboxMap(SandboxPlayer* playerPtr,IntVec2 const& size=IntVec2(1920,1080));

	void Update(float deltaTime);
	void Render() const;
	void RenderImGuiStats() const;
	void RenderCellInfo() const;

	void Initialize();

	void PlaceMaterial(int x, int y, CellMatType type, int brushSize = 1);
	void ClearArea(int x, int y, int radius);

	Cell& GetCell(int x, int y);
	bool IsValidPosition(int x, int y) const;

	void UpdateStatistics();
	void UpdateMouseGridPosition();
private:
	float GetFrameTime();

	void UpdateCell(int x, int y);

	void UpdatePhysics();
	void ResetUpdateFlags();
	bool IsInBounds(int x, int y) const;
	
	// update diff mats
	void UpdateSandParticle(int x, int y);
	void UpdateSandY(int x, int y);

	void UpdateWaterParticle(int x, int y);

	void UpdateSandParticle2(int x, int y);

	void UpdateWaterParticle2(int x, int y);
	void UpdateWaterParticle3(int x, int y);

	void GetWaterMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir);
	int GetWaterFlowDirection(const Cell& cell, int x, int y);
	int CountEmptySpacesBelow(int x, int y);
	void ApplyWaterCollisionPhysics(Cell& cell, int deltaX, int deltaY);
	void ApplyWaterStuckPhysics(Cell& cell);
	//float GetWaterRandom(int x1, int y1, int x2, int y2);
	//-----help func
	bool CanMoveTo(int x, int y);
	void ApplySlidingPhysics(Cell& cell, int slideDirection);
	void ApplyHorizontalFriction(Cell& cell, int horizontalDirection);
	bool CanMoveHorizontally(int x, int y, const Cell& cell);

	bool HasSupport(int x, int y);

	bool ActOnNeighboringElement(int targetX, int targetY, bool isFinal, bool isFirst,
		IntVec2& currentLocation, IntVec2& lastValidLocation, int depth);

	void UpdateAccumulatedMovement(int oldX, int oldY, int newX, int newY);

	const char* GetMaterialTypeName(CellMatType type) const;

	void ApplyCollisionPhysics(Cell& cell, int deltaX, int deltaY);
	int GetDeterministicDirection(int x, int y);
	void GetMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir);
	void ApplyHorizontalMovementPhysics(Cell& cell, int direction);
	void ApplyStuckPhysics(Cell& cell);
	int GetHorizontalDirection(const Cell& cell, int x, int y);
	bool HandleSandCollision(int fromX, int fromY, int toX, int toY, Cell& movingCell, Cell& targetCell);
	void TransferMomentum(Cell& fromCell, Cell& toCell, float transferRatio);
	void ActivateSandParticle(Cell& cell, float initialForce);
	bool TryPushSandParticle(int x, int y, int pushDirX, int pushDirY, float pushForce);
	void ApplySandCollisionPhysics(Cell& cell, int deltaX, int deltaY, float impactForce);
	float GetCollisionRandom(int x1, int y1, int x2, int y2);
public:
	

private:
	std::vector<std::vector<Cell>> m_grid;
	SandboxPlayer* m_player;
	IntVec2 m_mapSize;
	AABB2 m_mapBound;

	std::vector<Vertex_PCU> m_boundVerts;

	float m_curDeltaTime = 0.f;

	//------------
	std::vector<std::pair<int, int>> m_updateOrder;
	int m_frameCount = 0;

	//------------------------------------------------------------------
	int m_totalMaterialsSet = 0;        // 总的材料设置次数
	int m_sandSet = 0;
	int m_waterSet = 0;
	int m_stoneSet = 0;
	mutable int m_cachedNonEmptyCells = -1;  // 缓存的非空cell数量，-1表示需要重新计算
	mutable bool m_statsDirty = true;        // 统计数据是否需要更新
	// 分类统计缓存
	mutable int m_cachedSandCells = 0;
	mutable int m_cachedWaterCells = 0;
	mutable int m_cachedStoneCells = 0;

	int m_mouseGridX = 0;
	int m_mouseGridY = 0;
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

