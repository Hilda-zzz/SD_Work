#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "ThirdParty/box2d/include/box2d/types.h"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Cell.hpp"
#include "ThirdParty/box2d/include/box2d/id.h"
#include "Engine/Math/AABB2.hpp"
#include "RigidBodyPrecomputedData.hpp"

class RigidBodyManager;
struct Cell;
class CellChunk;

struct CellMoveInfo {
	Cell* oldPtr;
	IntVec2 oldCoords;
	IntVec2 newCoords;
	Cell cellData;
	Vec2 localPos;
	bool needsMove;
};

class RigidBodyObject
{
public:
	RigidBodyObject(int rbID, b2WorldId b2WorldId, RigidBodyManager* manager);
	~RigidBodyObject();

	void Initialize(std::vector<CellWithCoords> const& cells, b2BodyType type);

	// 新增：从预计算数据初始化
	void InitializeFromPrecomputedData(RigidBodyPrecomputedData&& data);

	void Update();
	// Now below are all for dynamic rb
	void ValidateAndCollectCells();
	void PlaceCellsToNewPositions();
	void SyncFromBox2D();
	AABB2 CalculateRotatedCellCoverage() const;

	void Render() const;

	// === Manage Cells ===  // #TODO: need to complete, now is wrong and useless version
	void AddCell(Cell* cell, IntVec2 worldCoords);
	void RemoveCell(Cell* cell);

	// === Apply physics ===
	void ApplyForce(Vec2 force);
	void ApplyTorque(float torque);
	void SetVelocity(Vec2 velocity);

	// === Get/ Set ===
	int GetId() const { return m_rbID; }
	b2BodyId GetBody() { return m_b2BodyId; }
	RigidBodyManager* GetManager() { return m_manager; }

	// === Help funcs ===
	Vec2 GetWorldPosition() const;
	float GetRotation() const;

	// Position trans
	Vec2 LocalToWorld(Vec2 localPos) const;
	Vec2 WorldToLocal(Vec2 worldPos) const;

	// ==================== NEW: GameMap Mode Support ====================

	// === Activation Control ===
	void SetActive(bool active);
	bool IsActive() const { return m_isActive; }

	//void EnablePhysics();   // 新增
	//void DisablePhysics();  // 新增

	// === Body Type ===
	b2BodyType GetBodyType() const { return m_bodyType; }
	bool IsDynamic() const { return m_bodyType == b2_dynamicBody; }
	bool IsStatic() const { return m_bodyType == b2_staticBody; }

	// === Owner Chunk ===
	void SetOwnerChunk(CellChunk* chunk) { m_ownerChunk = chunk; }
	CellChunk* GetOwnerChunk() const { return m_ownerChunk; }

	// === Affected Chunks (for cross-chunk rigid bodies) ===
	void AddAffectedChunk(CellChunk* chunk);
	const std::unordered_set<CellChunk*>& GetAffectedChunks() const { return m_affectedChunks; }

	// for release pool position
	void Reset();

private:
	void CreateBox2DBody(std::vector<CellWithCoords> const& cells, b2BodyType type);

	void RecalculateCenterOfMass();
	void RebuildBox2DFixtures();
	bool ShouldUpdate() const;

	// 分离：只负责创建Box2D对象
	void CreateBox2DBodyFromPrecomputed(const RigidBodyPrecomputedData& data);

public:
	int m_rbID = -1;
	RigidBodyManager* m_manager=nullptr; // now for sandbox only 

	// === box 2d ===
	b2WorldId m_b2WorldId;              // 保存world ID引用
	b2BodyId m_b2BodyId;                // 该对象的body ID
	std::vector<b2ShapeId> m_b2ShapeIds; // 所有shape IDs
	b2BodyType m_bodyType = b2_dynamicBody;

	// === Cell data ====
	std::vector<Cell*> m_cells;
	std::unordered_map<Cell*, IntVec2> m_cellToWorldCoords;
	std::unordered_map<IntVec2, Cell, IntVec2Hash> m_cellBlueprint;

	// === States (Only For dynamic) ===
	Vec2 m_position = Vec2::ZERO;
	float m_rotation = 0.f;

	Vec2 m_lastPosition = Vec2::ZERO;
	float m_lastAngle = 0.f;
	float m_positionAccumulator = 0.f;
	float m_angleAccumulator = 0.f;

	// Only for static
	std::unordered_map<Cell*, IntVec2> m_staticCellCoords;

	// === Settings ===
	float m_updateThreshold = 0.2f;
	float m_minCellCount = 30; // destroy rb object if less than this count

	// ==================== NEW: GameMap Mode Members ====================
	// === Activation ===
	bool m_isActive = true;
	// === Ownership ===
	CellChunk* m_ownerChunk = nullptr;  // 质心所在的chunk（主管理者）
	std::unordered_set<CellChunk*> m_affectedChunks;  // 影响到的所有chunk

	// ============== Debug Draw verts ====================
	std::vector<Vertex_PCU> m_marchingSquaresVerts;
	std::vector<Vertex_PCU> m_douglasVerts;
	std::vector<Vertex_PCU> m_triangleMeshVerts;
	std::vector<Vertex_PCU> m_positionDebugDrawVerts;
};