#pragma once
#include <vector>
#include <ThirdParty/box2d/include/box2d/types.h>
#include <unordered_map>
#include <vector>
#include "Game/Cell.hpp"
class SandboxMap;
class RigidBodyObject;
class b2World;

class RigidBodyManager
{
public:
	RigidBodyManager(SandboxMap* ownerMap);
	RigidBodyManager(SandboxMap* ownerMap, b2WorldId physicsWorldId);
	~RigidBodyManager();

	RigidBodyManager(RigidBodyManager const&) = delete;
	RigidBodyManager& operator=(RigidBodyManager const&) = delete;

	void Update(float deltaTime);

	void RenderDebug();

	void CreateRigidBodies(std::vector<CellWithCoords>& cells, b2BodyType type);

	void CreateRigidBody(std::vector<CellWithCoords>& cells,
		b2BodyType type = b2_dynamicBody);

	void DestroyRigidBody(int rigidBodyId);
	void DestoryRigidBody(RigidBodyObject* obj);

	void UpdateAll();

	void OnCellRemoved();

	SandboxMap* GetOwnerMap() { return m_ownerMap; }
	b2WorldId GetPhysicsWorld() { return m_b2WorldId; }

	int GetNextId() { return m_nextId; }

	static void DrawSolidPolygon(
		b2Transform transform,
		const b2Vec2* vertices,
		int vertexCount,
		float radius,
		b2HexColor m_color,
		void* context);
	//static void DrawCircle(b2Vec2 center, float radius,
	//	b2HexColor color, void* context);

public:
	std::vector<RigidBodyObject*> m_testRbList;
private:
	void HandleRigidBodySplit(RigidBodyObject* obj);

private:
	SandboxMap* m_ownerMap = nullptr;;
	b2WorldId m_b2WorldId;
	b2DebugDraw m_debugDraw;
	
	std::unordered_map<int, RigidBodyObject*> m_rigidBodies;
	std::unordered_map<Cell*, RigidBodyObject*> m_cellToRigidBody;
	int m_nextId=0;
};