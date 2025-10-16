#include "RigidbodyManager.hpp"
#include "RigidBodyObject.hpp"


RigidBodyManager::RigidBodyManager(SandboxMap* ownerMap)
	:m_ownerMap(ownerMap)
{
}

RigidBodyManager::RigidBodyManager(SandboxMap* ownerMap, b2World* physicsWorld) 
	:m_ownerMap(ownerMap),m_physicsWorld(physicsWorld)
{
}

RigidBodyManager::~RigidBodyManager()
{
	for (RigidBodyObject* obj : m_testRbList)
	{
		delete obj;
		obj = nullptr;
	}
}

//RigidBodyObject* RigidBodyManager::CreateRigidBody(std::vector<Cell*> const& cells, b2BodyType type)
//{
//	// ========== 步骤1: 验证输入 ==========
//	// 需要: 最小cell数量阈值
//	//if (!ValidateInput(cells)) {
//	//	return nullptr;
//	//}
//
//	// ========== 步骤2: 计算质心 ==========
//	// 需要: cells 的世界坐标
//	// 输出: 质心的世界坐标(格子单位)
//	Vec2 centroid = CalculateCentroid(cells);
//
//	// ========== 步骤3: 创建RigidBodyObject ==========
//	// 需要: 下一个可用的ID
//	RigidBodyObject* obj = new RigidBodyObject(m_nextId++, this);
//
//	// ========== 步骤4: 创建Box2D body ==========
//	// 需要: 
//	// - 质心坐标(格子) → 转换为Box2D坐标(米)
//	// - body类型
//	b2Body* body = CreateBox2DBody(centroid, type);
//
//	// ========== 步骤5: 计算每个cell的本地坐标 ==========
//	// 需要: 
//	// - 每个cell的世界坐标
//	// - 质心坐标
//	// 输出: cell → 本地坐标的映射
//	std::unordered_map<Cell*, Vec2> localCoords =
//		CalculateLocalCoordinates(cells, centroid);
//
//	// ========== 步骤6: 生成Box2D fixtures(碰撞体) ==========
//	// 需要:
//	// - cells集合
//	// - 质心坐标
//	// - 物理材质参数(密度、摩擦力、弹性)
//	CreateFixturesForBody(body, cells, centroid);
//
//	// ========== 步骤7: 初始化RigidBodyObject ==========
//	// 设置: body, cells, localCoords
//	obj->Initialize(body, cells, localCoords, centroid);
//
//	// ========== 步骤8: 建立映射关系 ==========
//	// 需要: 两个map
//	// - rigidBodies: id → RigidBodyObject
//	// - cellToRigidBody: Cell → RigidBodyObject
//
//	//RegisterRigidBody(obj, cells);
//
//	// ========== 步骤9: 标记cells属于刚体 ==========
//	// 需要: 在Cell上设置标志位
//
//	// MarkCellsAsRigidBody(cells, obj);
//
//	// ========== 步骤10: 返回创建的刚体 ==========
//	return obj;
//
//
//	{
//		b2BodyDef groundBodyDef = b2DefaultBodyDef();
//		groundBodyDef.type = b2_staticBody;
//		groundBodyDef.position = { SCREEN_SIZE_X * 0.5f * METERS_PER_PIXEL,
//								   2.0f * METERS_PER_PIXEL };
//		m_groundBodyId = b2CreateBody(m_worldId, &groundBodyDef);
//
//		b2Polygon groundBox = b2MakeBox(SCREEN_SIZE_X * 0.5f * METERS_PER_PIXEL, 2.0f);
//
//		b2ShapeDef groundShapeDef = b2DefaultShapeDef();
//		groundShapeDef.material.friction = 0.5f;
//		groundShapeDef.material.restitution = 0.3f;
//		b2CreatePolygonShape(m_groundBodyId, &groundShapeDef, &groundBox);
//	}
//}
