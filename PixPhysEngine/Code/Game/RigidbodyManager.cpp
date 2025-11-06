#include "RigidbodyManager.hpp"
#include "RigidBodyObject.hpp"
#include "Box2DShapeBuilder.hpp"
#include "SandboxMap.hpp"
#include "Game/Cell.hpp"
#include "SandboxPlayer.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

RigidBodyManager::RigidBodyManager(SandboxMap* ownerMap)
	:m_ownerMap(ownerMap)
{
	// 1. 使用默认初始化
	m_debugDraw = b2DefaultDebugDraw();

	// 2. 设置回调（注意 Fcn 后缀）
	m_debugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
	m_debugDraw.context = this;

	// 3. 配置绘制选项
	m_debugDraw.drawShapes = true;
	m_debugDraw.drawJoints = false;
	m_debugDraw.drawBounds = false;
	m_debugDraw.drawMass = false;
	m_debugDraw.drawContacts = false;
}

RigidBodyManager::RigidBodyManager(SandboxMap* ownerMap, b2WorldId physicsWorldId)
	:m_ownerMap(ownerMap), m_b2WorldId(physicsWorldId)
{
	// 1. 使用默认初始化
	m_debugDraw = b2DefaultDebugDraw();

	// 2. 设置回调（注意 Fcn 后缀）
	m_debugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
	m_debugDraw.context = this;

	// 3. 配置绘制选项
	m_debugDraw.drawShapes = true;
	m_debugDraw.drawJoints = false;
	m_debugDraw.drawBounds = false;
	m_debugDraw.drawMass = false;
	m_debugDraw.drawContacts = false;
}

RigidBodyManager::~RigidBodyManager()
{
	for (RigidBodyObject* obj : m_testRbList)
	{
		delete obj;
		obj = nullptr;
	}
}

void RigidBodyManager::Update(float deltaTime)
{
	for (RigidBodyObject* object : m_testRbList)
	{
		object->ValidateAndCollectCells();
	}

	int subStepCount = 4;
	b2World_Step(m_b2WorldId, deltaTime, subStepCount);

	for (RigidBodyObject* object : m_testRbList)
	{
		object->Update();
	}
}

void RigidBodyManager::RenderDebug()
{
	g_theRenderer->SetModelConstants();
	b2World_Draw(m_b2WorldId, &m_debugDraw);
}

void RigidBodyManager::CreateRigidBodies(std::vector<CellWithCoords>& cells, b2BodyType type)
{
	// first sepearte the cells
	auto components = Box2DShapeBuilder::SeparateConnectedComponents(cells);

	// generate each rb and initialize it
	for (auto comp : components)
	{
		CreateRigidBody(comp, type);
	}
}

void RigidBodyManager::CreateRigidBody(std::vector<CellWithCoords>& cells, b2BodyType type)
{
	// discard the small one
	if (Box2DShapeBuilder::IfDiscardComponent(cells))
		return;

	// then filled the hole
	std::vector<IntVec2> needFilledCellsCoords = Box2DShapeBuilder::FillHoles(cells);
	for (auto coord : needFilledCellsCoords)
	{
		m_ownerMap->PlaceMaterialInChunk(coord.x, coord.y, CellMatType::MAT_STATIC_FILL);
		CellWithCoords  filledCell = CellWithCoords(
			&m_ownerMap->GetCell(coord.x, coord.y),
			IntVec2(coord.x, coord.y)
		);
		m_ownerMap->GetCurPlayer()->m_rigidBodyDrawnCells.push_back(filledCell);
		cells.push_back(filledCell);
	}

	RigidBodyObject* newObj = new RigidBodyObject(m_ownerMap->GetRigidBodyManager()->GetNextId(),
		m_ownerMap->GetPhysicsWorldId(),
		m_ownerMap->GetRigidBodyManager());
	m_ownerMap->GetRigidBodyManager()->m_testRbList.push_back(newObj);
	newObj->Initialize(cells, type);
}

void RigidBodyManager::DestoryRigidBody(RigidBodyObject* obj)
{
	if (!obj) return;

	// 1. 清除所有cells的刚体标记
	for (Cell* cell : obj->m_cells) {
		if (cell) {
			cell->m_rigidBodyId = -1;
			cell->m_isBelongRb = false;
		}
	}

	// 2. 销毁Box2D刚体
	if (!B2_IS_NULL(obj->m_b2BodyId)) {
		b2DestroyBody(obj->m_b2BodyId);
	}

	// 3. 从列表中移除
	auto it = std::find(m_testRbList.begin(), m_testRbList.end(), obj);
	if (it != m_testRbList.end()) {
		m_testRbList.erase(it);
	}

	// 4. 从ID映射中移除（如果有的话）
	m_rigidBodies.erase(obj->GetId());

	// 5. 删除对象
	delete obj;
	obj = nullptr;
}

void RigidBodyManager::DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
{
	for (int i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p1 = b2TransformPoint(transform, vertices[i]);
		b2Vec2 p2 = b2TransformPoint(transform, vertices[(i + 1) % vertexCount]);

		Vec2 v1(p1.x / METERS_PER_CELL, p1.y / METERS_PER_CELL);
		Vec2 v2(p2.x / METERS_PER_CELL, p2.y / METERS_PER_CELL);

		std::vector<Vertex_PCU> verts;
		AddVertsForLinSegment2D(verts, v1, v2, 0.2f, Rgba8::GREEN);
		g_theRenderer->DrawVertexArray(verts);
	}
}
