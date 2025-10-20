#pragma once
#include <vector>
#include <unordered_map>
#include "Engine/Math/Vec2.hpp"
#include <ThirdParty/box2d/include/box2d/types.h>
#include <Engine/Core/Vertex_PCU.hpp>
#include "Game/Cell.hpp"
#include "ThirdParty/box2d/include/box2d/id.h"

class RigidBodyManager;
struct Cell;

class RigidBodyObject
{
public:
	RigidBodyObject(int rbID, b2WorldId b2WorldId, RigidBodyManager* manager);
	~RigidBodyObject();

	void Initialize(std::vector<CellWithCoords> const& cells, b2BodyType type);
	void Update();
	void SyncFromBox2D();
	void Render() const;

	// === Manage Cells ===
	void AddCell(Cell* cell);
	void RemoveCell(Cell* cell);

	int GetCellCount() const { return m_cells.size(); }
	const std::vector<Cell*>& GetCells() const { return m_cells; }

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
	Vec2 LocalToWorld(Vec2 localPos) const;
	Vec2 WorldToLocal(Vec2 worldPos) const;

	void RebuildFixtures();

private:
	void CreateBox2DBody(std::vector<CellWithCoords> const& cells, b2BodyType type);
	void UpdateCellWorldPositions();
	void RecalculateCenterOfMass();
	void RebuildBox2DFixtures();
	bool ShouldUpdate() const;

public:
	int m_rbID = -1;
	RigidBodyManager* m_manager;

	// === box 2d ===
	b2WorldId m_b2WorldId;              // 保存world ID引用
	b2BodyId m_b2BodyId;                // 该对象的body ID
	std::vector<b2ShapeId> m_b2ShapeIds; // 所有shape IDs

	// === Cell data ====
	std::vector<Cell*> m_cells;
	std::unordered_map<Cell*, Vec2> m_localCoords;

	// === States ===
	Vec2 m_position = Vec2::ZERO;
	float m_rotation = 0.f;

	Vec2 m_lastPosition = Vec2::ZERO;
	float m_lastAngle = 0.f;
	float m_positionAccumulator = 0.f;
	float m_angleAccumulator = 0.f;

	// === Settings ===
	float m_updateThreshold = 0.2f;
	float m_minCellCount = 3; // destroy rb object if less than this count

	// ============== Debug Draw verts ====================
	std::vector<Vertex_PCU> m_marchingSquaresVerts;
	std::vector<Vertex_PCU> m_douglasVerts;
	std::vector<Vertex_PCU> m_triangleMeshVerts;
};