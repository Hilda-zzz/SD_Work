#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <utility>
#include "Game/CellChunk.hpp"
class SandboxPlayer;

constexpr float GRAVITY = -98.f;           
constexpr float TERMINAL_SPEED = 200.0f; 
constexpr float AIR_RESISTANCE = 0.98f;    
constexpr float SAND_FRICTION = 0.7f;  

constexpr float WATER_HORIZONTAL_SPEED = 50.0f;  
constexpr float WATER_FLOW_DAMPING = 0.8f;      

static const float SPEED_THRESHOLD = 5.0f;

enum class UpdateOrder {
	FIXED,       // 固定顺序 0→1→2→3
	ROTATING,    // 轮换顺序
	RANDOM       // 随机顺序
};

enum class CellColorMode {
	NORMAL = 0,           // Normal material colors
	STATIC_DYNAMIC = 1,   // Red=static, Green=dynamic, White=static solid
	FRAME_COUNT = 2       // Red->Green gradient by frame count, White=static solid
};

class SandboxMap {
public:
	SandboxMap(SandboxPlayer* playerPtr,IntVec2 const& size=IntVec2(1920,1080));
	~SandboxMap();

	void Update(float deltaTime);
	void Render() const;
	void RenderImGuiStats() const;
	void RenderCellInfo() const;
	void RenderDebugDrawPanel();

	void ChunkAddVerts(CellChunk* chunk, std::vector<Vertex_PCU>& outVerts) const;

	Rgba8 GetCellDebugColor(const Cell& cell) const;

	void Initialize();

	void PlaceMaterial(int x, int y, CellMatType type, int brushSize = 1);
	void ClearArea(int x, int y, int radius);

	Cell& GetCell(int x, int y);
	bool IsValidPosition(int x, int y) const;

	void SetUpdateOrder(UpdateOrder order) { m_updateOrder = order; }
	//=======Chunk Base New ===============
	Cell& GetCellInChunk(int worldX, int worldY);
	bool IsInBounds_Chunk(int worldX, int worldY) const;
	void PlaceMaterialInChunk(int worldX, int worldY, CellMatType type, int brushSize = 1);
	void UpdateChunksOfPhase(int phaseIndex);
	void UpdateSingleChunk(CellChunk* chunk);
	bool MSCanMoveToInChunk(int x, int y, float curDensity);
	bool LiquidCanMoveToInChunk(int x, int y, float curDensity);
	bool CanMoveHorizontallyInChunk(int x, int y, const Cell& cell);
	bool HasSupportInChunk(int x, int y);
	//=====================================

	void UpdateStatistics();
	void UpdateMouseGridPosition();

	//=============== useful in refractor code ===========================
	float GetDeltaTime();
	bool IsInBounds(int x, int y) const;
	void UpdateAccumulatedMovement(int oldX, int oldY, int newX, int newY); 
	void UpdateAccumulatedMovementLiquid(int oldX, int oldY, int newX, int newY);

	bool CanMoveTo(int x, int y);
	bool CanMoveToEmpty(int x, int y);
	bool LiquidCanMoveTo(int x, int y,float curDensity);
	bool MSCanMoveTo(int x, int y, float curDensity);
	void GetMovementDirections(const Cell& cell, int x, int y, int& primaryDir, int& secondaryDir);
	bool CanMoveHorizontally(int x, int y, const Cell& cell);

	// Chunk访问
	CellChunk* GetChunk(int chunkX, int chunkY);
	CellChunk* GetChunkByWorldPos(int worldX, int worldY);
	bool IsChunkIndexValid(int chunkX, int chunkY) const;

private:
	void UpdatePhysics();
	void UpdatePhysicsInChunk();
	void ResetUpdateFlags();
	bool HasSupport(int x, int y);
	const char* GetMaterialTypeName(CellMatType type) const;

	UpdateOrder m_updateOrder = UpdateOrder::ROTATING;


public:
	

private:
	float m_deltaTime = 0.f;

	// === OLD - 准备删除 ===
	std::vector<std::vector<Cell>> m_grid;

	// === NEW - Chunk 管理 ===
	std::vector<std::vector<CellChunk*>> m_chunks;  // [chunkY][chunkX]
	IntVec2 m_chunkGridSize;                        // chunk网格的行列数 (例如 8x4 表示8列4行)

	IntVec2 m_mapSize;
	AABB2 m_mapBound;

	SandboxPlayer* m_player;
	int m_mouseGridX = 0;
	int m_mouseGridY = 0;

	std::vector<Vertex_PCU> m_boundVerts;

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
	//------------------------------------------------------------------

	struct DebugVisualizationSettings {
		// Grid options
		bool drawChunkGrid = true;
		bool drawStaticChunks = false;
		bool drawDynamicChunks = true;

		// Cell coloring mode
		CellColorMode colorMode = CellColorMode::NORMAL;
	} m_debugSettings;

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

