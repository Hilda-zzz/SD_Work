#include "RigidBodyObject.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Box2DShapeBuilder.hpp"
#include <Engine/Core/VertexUtils.hpp>
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include "SandboxMap.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "Game/RigidbodyManager.hpp"
#include "Engine/Math/MathUtils.hpp"

extern Renderer* g_theRenderer;

RigidBodyObject::RigidBodyObject(int id, b2WorldId b2WorldId, RigidBodyManager* manager)
	:m_rbID(id),m_b2WorldId(b2WorldId),m_manager(manager)
{
}

RigidBodyObject::~RigidBodyObject()
{
}

void RigidBodyObject::Initialize(std::vector<CellWithCoords> const& cells, b2BodyType type)
{
	CreateBox2DBody(cells, type);

	// add all cells to management
	SandboxMap* map = m_manager->GetOwnerMap();
	for (const CellWithCoords& cellData : cells) 
	{
		m_cells.push_back(cellData.m_cell);
		Vec2 cellWorldPos = Vec2((float)cellData.m_worldCoords.x, (float)cellData.m_worldCoords.y);
		Vec2 localPos = WorldToLocal(cellWorldPos);
		m_cellToLocal[cellData.m_cell] = localPos;
		m_cellToWorld[cellData.m_cell] = cellData.m_worldCoords;

		cellData.m_cell->m_isBelongRb = true;
		cellData.m_cell->m_rigidBodyId = m_rbID;
	}
}

void RigidBodyObject::Update()
{
	SyncFromBox2D();
	PlaceCellsToNewPositions();

	m_positionVerts.clear();
	AddVertsForDisc2D(m_positionVerts, m_position, 2.f, Rgba8::CYAN);
}

void RigidBodyObject::ValidateAndCollectCells()
{
	std::vector<Cell*> validCells;
	std::unordered_map<Cell*, Vec2> validMapping;

	// 遍历所有cell指针
	for (Cell* cellPtr : m_cells) 
	{
		// 1. 验证指针指向的cell是否还属于此刚体
		if (cellPtr->m_rigidBodyId == m_rbID && !cellPtr->IsEmpty()) 
		{
			// ✅ 有效：备份cell数据
			validCells.push_back(cellPtr);
			validMapping[cellPtr] = m_cellToLocal[cellPtr];

			// 清空旧位置（准备移动）
			//cellPtr->SetEmpty();
		}
	}

	// 2. 更新列表（只保留有效的）
	m_cellToLocal.clear();
	m_cells.clear();
	m_cells = validCells;
	m_cellToLocal = validMapping;

	// 3. 检查是否需要销毁刚体
	//if (m_cells.size() < m_minCellCount) {
	//	m_manager->DestoryRigidBody(this);
	//}
}

void RigidBodyObject::PlaceCellsToNewPositions()
{
	SandboxMap* map = m_manager->GetOwnerMap();
	if (!map) return;

	struct CellMoveInfo {
		Cell* oldPtr;
		IntVec2 oldCoords;
		IntVec2 newCoords;
		Cell cellData;
		Vec2 localPos;
		bool needsMove;
	};

	std::vector<CellMoveInfo> moveList;

	// === 阶段1：收集所有移动信息 ===
	for (Cell* oldCellPtr : m_cells)
	{
		// 验证有效性
		if (oldCellPtr->m_rigidBodyId != m_rbID || oldCellPtr->IsEmpty()) {
			continue;
		}

		// 获取局部坐标
		auto it_local = m_cellToLocal.find(oldCellPtr);
		if (it_local == m_cellToLocal.end()) {
			ERROR_AND_DIE("Cell not found in local mapping!");
		}
		Vec2 localPos = it_local->second;

		// ⭐ 从映射中获取旧世界坐标
		auto it_world = m_cellToWorld.find(oldCellPtr);
		if (it_world == m_cellToWorld.end()) {
			ERROR_AND_DIE("Cell not found in world mapping!");
		}
		IntVec2 oldCoords = it_world->second;

		// 计算新位置
		Vec2 newWorldPos = LocalToWorld(localPos);
		int newWorldX = (int)roundf(newWorldPos.x);
		int newWorldY = (int)roundf(newWorldPos.y);

		// 边界检查
		if (!map->IsInBounds(newWorldX, newWorldY)) {
			continue;
		}

		IntVec2 newCoords = IntVec2(newWorldX, newWorldY);

		// 检查目标位置
		if (newCoords != oldCoords) {
			Cell& targetCell = map->GetCellInChunk(newWorldX, newWorldY);
			if (!targetCell.IsEmpty() && targetCell.m_rigidBodyId != m_rbID) {
				// 被其他东西占用，保留原位
				newCoords = oldCoords;
			}
		}

		// 保存移动信息
		CellMoveInfo info;
		info.oldPtr = oldCellPtr;
		info.oldCoords = oldCoords;
		info.newCoords = newCoords;
		info.cellData = *oldCellPtr;  // 快照
		info.localPos = localPos;
		info.needsMove = (oldCoords != newCoords);

		moveList.push_back(info);
	}

	// === 阶段2：清空所有旧位置 ===
	for (const CellMoveInfo& info : moveList) {
		if (info.needsMove) {
			Cell& oldCell = map->GetCellInChunk(info.oldCoords.x, info.oldCoords.y);
			oldCell.SetEmpty();
		}
	}

	// === 阶段3：写入所有新位置并更新映射 ===
	std::vector<Cell*> newCells;
	std::unordered_map<Cell*, Vec2> newLocalMapping;
	std::unordered_map<Cell*, IntVec2> newWorldMapping;  // ⭐ 新的世界坐标映射

	for (const CellMoveInfo& info : moveList) {
		Cell& newCellRef = map->GetCellInChunk(info.newCoords.x, info.newCoords.y);

		if (info.needsMove) {
			if (!newCellRef.IsEmpty()) {
				continue;  // 冲突，跳过
			}
			// 写入数据
			newCellRef = info.cellData;
		}

		// 更新指针和映射
		Cell* newCellPtr = &newCellRef;
		newCells.push_back(newCellPtr);
		newLocalMapping[newCellPtr] = info.localPos;
		newWorldMapping[newCellPtr] = info.newCoords;  // ⭐ 更新世界坐标

		// 标记dirty
		CellChunk* newChunk = map->GetChunkByWorldPos(info.newCoords.x, info.newCoords.y);
		if (newChunk) newChunk->MarkDirty();

		if (info.needsMove) {
			CellChunk* oldChunk = map->GetChunkByWorldPos(info.oldCoords.x, info.oldCoords.y);
			if (oldChunk) oldChunk->MarkDirty();
		}
	}

	// === 阶段4：更新成员变量 ===
	m_cells = newCells;
	m_cellToLocal = newLocalMapping;
	m_cellToWorld = newWorldMapping;  // ⭐ 更新世界坐标映射

	if (m_cells.size() < m_minCellCount) {
		m_manager->DestoryRigidBody(this);
	}
}

void RigidBodyObject::SyncFromBox2D()
{
	// 1. 从 Box2D 获取物理坐标（米制单位）
	b2Vec2 b2Position = b2Body_GetPosition(m_b2BodyId);
	b2Rot b2Rotation = b2Body_GetRotation(m_b2BodyId);

	// 2. 转换到游戏坐标系
	// Box2D 使用米制单位，需要转换回 cell 单位
	m_position.x = b2Position.x / METERS_PER_CELL;
	m_position.y = b2Position.y / METERS_PER_CELL;

	// 从 Box2D 的 rotation (cos, sin) 计算角度
	float angleRadians = b2Rot_GetAngle(b2Rotation);
	m_rotation = angleRadians * (180.0f / B2_PI);

	// 3. 检查是否发生了显著移动
	bool hasMoved = false;

	float positionDelta = (m_position - m_lastPosition).GetLengthSquared();
	float angleDelta = fabsf(m_rotation - m_lastAngle);

	if (positionDelta > m_updateThreshold * m_updateThreshold ||
		angleDelta > 0.01f)  // ~0.57度
	{
		hasMoved = true;
		m_lastPosition = m_position;
		m_lastAngle = m_rotation;

		// 4. 更新依赖于刚体位置的内容
		// UpdateCellWorldPositions();
	}
}

void RigidBodyObject::Render() const
{
	Mat44 modelMatrix = Mat44::MakeTranslation2D(m_position);  
	modelMatrix.Append(Mat44::MakeZRotationDegrees(m_rotation));
	g_theRenderer->SetModelConstants(modelMatrix);

	if(m_manager->GetOwnerMap()->m_debugSettings.m_drawMarchingSquares)
		g_theRenderer->DrawVertexArray(m_marchingSquaresVerts);
	if (m_manager->GetOwnerMap()->m_debugSettings.m_drawDouglas)
		g_theRenderer->DrawVertexArray(m_douglasVerts);
	if(m_manager->GetOwnerMap()->m_debugSettings.m_drawTriangleMesh)
		g_theRenderer->DrawVertexArray(m_triangleMeshVerts);

	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(m_positionVerts);
}

void RigidBodyObject::AddCell(Cell* cell, IntVec2 worldCoords)
{
	if (!cell) return;

	// 检查是否已经存在
	if (std::find(m_cells.begin(), m_cells.end(), cell) != m_cells.end()) {
		return;
	}

	// 添加到列表
	m_cells.push_back(cell);

	// 计算并存储局部坐标
	Vec2 cellWorldPos = Vec2((float)worldCoords.x, (float)worldCoords.y);
	Vec2 localPos = WorldToLocal(cellWorldPos);
	m_cellToLocal[cell] = localPos;

	// 标记cell属于此刚体
	cell->m_isBelongRb = true;
	cell->m_rigidBodyId = m_rbID;

}

void RigidBodyObject::RemoveCell(Cell* cell)
{
	if (!cell) return;

	auto it = std::find(m_cells.begin(), m_cells.end(), cell);
	if (it != m_cells.end()) {
		m_cells.erase(it);
	}

	m_cellToLocal.erase(cell);

	cell->m_rigidBodyId = -1;
	cell->m_isBelongRb = false;

	if (m_cells.size() < m_minCellCount)
	{
		if (m_manager)
		{
			m_manager->DestoryRigidBody(this);
		}
	}
}

Vec2 RigidBodyObject::LocalToWorld(Vec2 localPos) const
{
	// 1. 旋转局部坐标（应用刚体的旋转）
    float angleRad = m_rotation * (PI / 180.0f);  // 角度转弧度
    float cosA = cosf(angleRad);
    float sinA = sinf(angleRad);
    
    // 旋转矩阵: [cos -sin]
    //           [sin  cos]
    Vec2 rotated;
    rotated.x = localPos.x * cosA - localPos.y * sinA;
    rotated.y = localPos.x * sinA + localPos.y * cosA;
    
    // 2. 平移到世界坐标（加上刚体中心位置）
    Vec2 worldPos = rotated + m_position;
    
    return worldPos;
}

Vec2 RigidBodyObject::WorldToLocal(Vec2 worldPos) const
{
	// 1. 平移：计算相对于刚体中心的偏移
	Vec2 offset = worldPos - m_position;

	// 2. 旋转：应用反向旋转（-angle）
	float angleRad = -m_rotation * (PI / 180.0f);  // 注意负号
	float cosA = cosf(angleRad);
	float sinA = sinf(angleRad);

	// 反向旋转矩阵: [cos  sin]
	//               [-sin cos]
	Vec2 localPos;
	localPos.x = offset.x * cosA - offset.y * sinA;
	localPos.y = offset.x * sinA + offset.y * cosA;

	return localPos;
}

void RigidBodyObject::CreateBox2DBody(std::vector<CellWithCoords> const& cells, b2BodyType type)
{
	std::vector<Vec2>  marchingSquarePoints;
	std::vector<Vec2>  outlinePoints = Box2DShapeBuilder::ExtractOutlineFromCells(cells, marchingSquarePoints);

	Vec2 centroid_Cell = Box2DShapeBuilder::CalculateCentroid(outlinePoints);
	m_position = centroid_Cell;

	m_douglasVerts.clear();
	if (outlinePoints.size() >= 2)
	{
		Rgba8 lineColor = Rgba8::GREEN;

		for (size_t i = 0; i < outlinePoints.size(); ++i)
		{
			float t = static_cast<float>(i) / static_cast<float>(outlinePoints.size() - 1);

			Rgba8 startColor = Rgba8::WHITE;
			Rgba8 endColor = Rgba8::GREEN;
			Rgba8 lineColor = Interpolate(startColor, endColor, t);

			Vec2 currentPoint = outlinePoints[i];
			Vec2 nextPoint = outlinePoints[(i + 1) % outlinePoints.size()];
			//AddVertsForDisc2D(newObj->m_marchingSquaresVerts, currentPoint, 0.1f, lineColor);
			AddVertsForLinSegment2D(m_douglasVerts, currentPoint - centroid_Cell, nextPoint - centroid_Cell, 0.4f, lineColor);
		}
	}

	if (marchingSquarePoints.size() >= 2)
	{
		Rgba8 lineColor = Rgba8::GREEN;

		for (size_t i = 0; i < marchingSquarePoints.size(); ++i)
		{
			float t = static_cast<float>(i) / static_cast<float>(marchingSquarePoints.size() - 1);

			Rgba8 startColor = Rgba8::WHITE;
			Rgba8 endColor = Rgba8::GREEN;
			Rgba8 lineColor = Interpolate(startColor, endColor, t);

			Vec2 currentPoint = marchingSquarePoints[i];
			Vec2 nextPoint = marchingSquarePoints[(i + 1) % marchingSquarePoints.size()];
			//AddVertsForDisc2D(newObj->m_marchingSquaresVerts, currentPoint, 0.1f, lineColor);
			AddVertsForLinSegment2D(m_marchingSquaresVerts, currentPoint - centroid_Cell, nextPoint - centroid_Cell, 0.4f, lineColor);
		}
	}
	// ================== Test triangulation result ====================
	//TriangulationOutput triangulationResult = Box2DShapeBuilder::EarClipping(outlinePoints);
	TriangulationOutput triangulationResult = Box2DShapeBuilder::CDTTriangulation(outlinePoints);
	int triangleCount = triangulationResult.indices.size() / 3;
	if (!triangulationResult.indices.empty())
	{
		// ============== 绘制三角形（填充） ====================
		// 使用不同颜色渐变显示每个三角形
		for (int i = 0; i < triangleCount; ++i)
		{
			unsigned int idx0 = triangulationResult.indices[i * 3 + 0];
			unsigned int idx1 = triangulationResult.indices[i * 3 + 1];
			unsigned int idx2 = triangulationResult.indices[i * 3 + 2];
			Vec2 v0 = triangulationResult.vertices[idx0];
			Vec2 v1 = triangulationResult.vertices[idx1];
			Vec2 v2 = triangulationResult.vertices[idx2];

			Rgba8 colors[] = {
				Rgba8(255, 100, 100, 180),  // 红色
				Rgba8(100, 255, 100, 180),  // 绿色
				Rgba8(100, 100, 255, 180),  // 蓝色
				Rgba8(255, 255, 100, 180),  // 黄色
				Rgba8(255, 100, 255, 180),  // 品红
				Rgba8(100, 255, 255, 180)   // 青色
			};
			Rgba8 triangleColor = colors[i % 6];
			Vec3 centroidVec3 = Vec3(centroid_Cell.x, centroid_Cell.y, 0.f);
			m_triangleMeshVerts.push_back(Vertex_PCU(Vec3(v0.x, v0.y, 0.f) - centroidVec3, triangleColor, Vec2::ZERO));

			m_triangleMeshVerts.push_back(Vertex_PCU(Vec3(v1.x, v1.y, 0.f) - centroidVec3, triangleColor, Vec2::ZERO));

			m_triangleMeshVerts.push_back(Vertex_PCU(Vec3(v2.x, v2.y, 0.f) - centroidVec3, triangleColor, Vec2::ZERO));

		}
	}

	//===================== Create rigidbody ===========================
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = type;

	bodyDef.position = b2Vec2{
		centroid_Cell.x * METERS_PER_CELL,
		centroid_Cell.y * METERS_PER_CELL
	};

	// 根据body类型设置其他属性
	//if (type == b2_dynamicBody)
	//{
	//	bodyDef.linearDamping = 0.1f;   // 线性阻尼
	//	bodyDef.angularDamping = 0.1f;  // 角阻尼
	//	bodyDef.gravityScale = 1.0f;    // 重力缩放
	//}

	m_b2BodyId = b2CreateBody(m_b2WorldId, &bodyDef);
	if (B2_IS_NULL(m_b2BodyId))
	{
		ERROR_AND_DIE("Failed to create Box2D body!");
		return;
	}
	if (!b2Body_IsValid(m_b2BodyId)) {
		ERROR_AND_DIE("Created body is not valid!");
	}

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;				// 密度
	shapeDef.material.friction = 0.3f;		// 摩擦系数
	shapeDef.material.restitution = 0.2f;	// 弹性系数

	// generate triangle collider mesh shape
	for (int i = 0; i < triangleCount; ++i)
	{
		unsigned int idx0 = triangulationResult.indices[i * 3 + 0];
		unsigned int idx1 = triangulationResult.indices[i * 3 + 1];
		unsigned int idx2 = triangulationResult.indices[i * 3 + 2];

		Vec2 v0 = triangulationResult.vertices[idx0];
		Vec2 v1 = triangulationResult.vertices[idx1];
		Vec2 v2 = triangulationResult.vertices[idx2];

		// 将顶点转换为相对于body位置的局部坐标

		b2Vec2 localV0 = {
			(v0.x - centroid_Cell.x) * METERS_PER_CELL,
			(v0.y - centroid_Cell.y) * METERS_PER_CELL
		};
		b2Vec2 localV1 = {
			(v1.x - centroid_Cell.x) * METERS_PER_CELL,
			(v1.y - centroid_Cell.y) * METERS_PER_CELL
		};
		b2Vec2 localV2 = {
			(v2.x - centroid_Cell.x) * METERS_PER_CELL,
			(v2.y - centroid_Cell.y) * METERS_PER_CELL
		};

		// 创建hull（convex hull）用于构建polygon
		b2Vec2 points[3] = { localV2, localV1, localV0 };
		b2Hull hull = b2ComputeHull(points, 3);

		// 验证hull是否有效
		if (hull.count >= 3)
		{
			// 使用hull创建polygon
			b2Polygon polygon = b2MakePolygon(&hull, 0.0f);

			// 创建shape并附加到body
			b2ShapeId shapeId = b2CreatePolygonShape(m_b2BodyId, &shapeDef, &polygon);

			if (!B2_IS_NULL(shapeId)) {
				m_b2ShapeIds.push_back(shapeId);
			}
		}
	}
}
