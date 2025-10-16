#pragma once
#include <vector>
#include <ThirdParty/box2d/include/box2d/types.h>
#include <unordered_map>
#include <vector>
class SandboxMap;
struct Cell;
class RigidBodyObject;
class b2World;

class RigidBodyManager
{
public:
	RigidBodyManager(SandboxMap* ownerMap);
	RigidBodyManager(SandboxMap* ownerMap, b2World* physicsWorld);
	~RigidBodyManager();

	RigidBodyManager(RigidBodyManager const&) = delete;
	RigidBodyManager& operator=(RigidBodyManager const&) = delete;

	RigidBodyObject* CreateRigidBody(std::vector<Cell*> const& cells, 
		b2BodyType type = b2_dynamicBody);

	void DestroyRigidBody(int rigidBodyId);
	void DestoryRigidBody(RigidBodyObject* obj);

	void UpdateAll();

	void OnCellRemoved();

	SandboxMap* GetOwnerMap() { return m_ownerMap; }
	b2World* GetPhysicsWorld() { return m_physicsWorld; }

	int GetNextId() { return m_nextId; }

public:
	std::vector<RigidBodyObject*> m_testRbList;
private:
	void HandleRigidBodySplit(RigidBodyObject* obj);

private:
	SandboxMap* m_ownerMap; 
	b2World* m_physicsWorld;

	
	std::unordered_map<int, RigidBodyObject*> m_rigidBodies;
	std::unordered_map<Cell*, RigidBodyObject*> m_cellToRigidBody;
	int m_nextId;
};