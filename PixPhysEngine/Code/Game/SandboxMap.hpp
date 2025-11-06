#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <utility>
#include "Game/CellChunk.hpp"
#include "ThirdParty/box2d/include/box2d/id.h"
#include <ThirdParty/box2d/include/box2d/math_functions.h>
#include "BaseMap.hpp"
class SandboxPlayer;
class RigidBodyManager;

constexpr float GRAVITY = -9.8f * 16.f;
constexpr float TERMINAL_SPEED = 200.0f; 
constexpr float AIR_RESISTANCE = 0.98f;    
constexpr float SAND_FRICTION = 0.7f;  

constexpr float WATER_HORIZONTAL_SPEED = 50.0f;  
constexpr float WATER_FLOW_DAMPING = 0.8f;      

static const float SPEED_THRESHOLD = 5.0f;

// ======== Size btw box2d ========================================
constexpr float CELLS_PER_METER = 16.0f;  // 16 cell = 1 meter
constexpr float METERS_PER_CELL = 1.0f / CELLS_PER_METER;

constexpr float GRAVITY_METERS = 9.8f;           // 标准重力加速度
constexpr float TERMINAL_SPEED_METERS = 20.0f;  // 合理的落体终端速度
constexpr float WATER_SPEED_METERS = 5.0f;      // 合理的液体流速

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

class SandboxMap : public BaseMap
{
public:
	SandboxMap(SandboxPlayer* playerPtr,IntVec2 const& size=IntVec2(1920,1080));
	~SandboxMap() override;

	void Initialize() override;

	void Update(float deltaTime) override;

	void Render() const override;
	void RenderImGuiStats() const;
	void RenderCellInfo() const;
	void RenderDebugDrawPanel();
	
	// Help Funcs
	Rgba8 GetCellDebugColor(const Cell& cell, IntVec2 const& worldCoords) const override;
	bool IsValidPosition(int x, int y) const;
	SandboxPlayer* GetCurPlayer() { return m_player; }

	//=======Chunk Base New ===============
	Cell& GetCell(int worldX, int worldY) override;
	Cell const& GetCell(int worldX, int worldY) const override;

	bool IsInBounds_Chunk(int worldX, int worldY) const;
	void PlaceMaterialInChunk(int worldX, int worldY, CellMatType type,bool isRb=false);

	void UpdateChunksOfPhase(int phaseIndex);
	void UpdateSingleChunk(CellChunk* chunk);

	// should be moved from here 
	bool MSCanMoveTo(int x, int y, float curDensity) const override;
	bool LiquidCanMoveTo(int x, int y, float curDensity) const override;
	bool CanMoveHorizontally(int x, int y, const Cell& cell) const override;
	bool HasSupport(int x, int y) const override;
	//=====================================

	float GetDeltaTime();
	bool IsInBounds(int x, int y) const;

	// Chunk 访问
	CellChunk* GetChunk(int chunkX, int chunkY) override;
	CellChunk const* GetChunk(int chunkX, int chunkY) const override;

	CellChunk* GetChunkByWorldPos(int worldX, int worldY);
	bool IsChunkIndexValid(int chunkX, int chunkY) const;

	// ============================= Box2d ==================================
	b2WorldId GetPhysicsWorldId() { return m_b2WorldId; }
	RigidBodyManager* GetRigidBodyManager() { return m_rigidBodyManager; }

private:

	void UpdateCellsPhysInChunk();
	const char* GetMaterialTypeName(CellMatType type) const;

	void UpdateStatistics();
	void UpdateMouseGridPosition();

	// ================= Chemical Reaction =========================
	void UpdateCellsChemicalInChunk();
	void UpdateSingleChunkChemical(CellChunk* chunk);

	// =============== Box 2d ======================================
	void CreateBox2dWorld();

	// ==============================RB TEST========================================
	void CreateTestGround();
	void SpawnTestBox(Vec2 const& position);
	void RenderPhysicsDebug() const;
	
	// Coords trasitions
	b2Vec2 CellToPhysics(Vec2 const& cellPos) const {
		return b2Vec2{ cellPos.x * METERS_PER_CELL, cellPos.y * METERS_PER_CELL };
	}

	b2Vec2 CellToPhysics(float x, float y) const {
		return b2Vec2{ x * METERS_PER_CELL, y * METERS_PER_CELL };
	}

	Vec2 PhysicsToCell(b2Vec2 const& physicsPos) const {
		return Vec2{ physicsPos.x * CELLS_PER_METER, physicsPos.y * CELLS_PER_METER };
	}

	b2Vec2 CellVelocityToPhysics(Vec2 const& cellVel) const {
		return b2Vec2{ cellVel.x * METERS_PER_CELL, cellVel.y * METERS_PER_CELL };
	}

	Vec2 PhysicsVelocityToCell(b2Vec2 const& physicsVel) const {
		return Vec2{ physicsVel.x * CELLS_PER_METER, physicsVel.y * CELLS_PER_METER };
	}

public:
	struct DebugVisualizationSettings {
		// Grid options
		bool m_drawChunkGrid = true;
		bool m_drawStaticChunks = false;
		bool m_drawDynamicChunks = true;

		// Cell coloring mode
		CellColorMode m_colorMode = CellColorMode::NORMAL;

		// Rigid Body 2d
		bool m_drawMarchingSquares = false;
		bool m_drawDouglas = false;
		bool m_drawTriangleMesh = false;

	} m_debugSettings;

private:
	//float m_deltaTime = 0.f;

	// === NEW - Chunk 管理 ===
	//std::vector<std::vector<CellChunk*>> m_chunks;  // [chunkY][chunkX]
	//IntVec2 m_chunkGridSize;                        // chunk网格的行列数 (例如 8x4 表示8列4行)

	//IntVec2 m_mapSize;
	//AABB2 m_mapBound;

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

	UpdateOrder m_updateOrder = UpdateOrder::ROTATING;

	//-----------------Box 2d ----------------------------------------
	b2WorldId m_b2WorldId;
	RigidBodyManager* m_rigidBodyManager;

	// test ground and box
	b2BodyId m_testGroundBodyId;
	std::vector<b2BodyId> m_testBoxBodies;

	// ==== multi thread =====
	bool m_useJobSystem = true;

};



