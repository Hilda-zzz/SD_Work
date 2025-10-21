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
	//for (const CellWithCoords& cellData : cells) 
	//{
	//	m_cells.push_back(cellData.m_cell);
	//	Vec2 cellWorldPos = Vec2((float)cellData.m_worldCoords.x, (float)cellData.m_worldCoords.y)+Vec2(0.5f,0.5f);
	//	Vec2 localPos = WorldToLocal(cellWorldPos);
	//	m_cellToLocal[cellData.m_cell] = localPos;
	//	m_cellToWorldCoords[cellData.m_cell] = cellData.m_worldCoords;

	//	cellData.m_cell->m_isBelongRb = true;
	//	cellData.m_cell->m_rigidBodyId = m_rbID;
	//}

	m_cellBlueprint.clear();
	m_cells.clear();
	//m_cellToWorldCoords.clear();

	for (const CellWithCoords& cellData : cells)
	{
		m_cells.push_back(cellData.m_cell);
		Vec2 cellWorldPos = Vec2((float)cellData.m_worldCoords.x, (float)cellData.m_worldCoords.y) + Vec2(0.5f, 0.5f);
		//m_cellToWorldCoords[cellData.m_cell] = cellData.m_worldCoords;

		Vec2 localPos = WorldToLocal(cellWorldPos);
		IntVec2 localKey((int)floorf(localPos.x), (int)floorf(localPos.y));
		//m_cellToLocal[cellData.m_cell] = localPos;
		// ⭐ 保存到蓝图（这张图永远不变！）
		m_cellBlueprint[localKey] = *cellData.m_cell;

		cellData.m_cell->m_isBelongRb = true;
		cellData.m_cell->m_rigidBodyId = m_rbID;
	}

}

void RigidBodyObject::Update()
{
	SyncFromBox2D();

	m_positionVerts.clear();
	PlaceCellsToNewPositions();
	//AddVertsForDisc2D(m_positionVerts, m_position, 2.f, Rgba8::CYAN);
}

void RigidBodyObject::ValidateAndCollectCells()
{
	std::vector<Cell*> validCells;
	//std::unordered_map<Cell*, Vec2> validMapping;

	// 遍历所有cell指针
	for (Cell* cellPtr : m_cells) 
	{
		// 1. 验证指针指向的cell是否还属于此刚体
		if (cellPtr->m_rigidBodyId == m_rbID && !cellPtr->IsEmpty()) 
		{
			// ✅ 有效：备份cell数据
			validCells.push_back(cellPtr);
			//validMapping[cellPtr] = m_cellToLocal[cellPtr];

			// 清空旧位置（准备移动）
			//cellPtr->SetEmpty();
		}
	}

	// 2. 更新列表（只保留有效的）
	//m_cellToLocal.clear();
	m_cells.clear();
	m_cells = validCells;
	//m_cellToLocal = validMapping;

	// 3. 检查是否需要销毁刚体
	//if (m_cells.size() < m_minCellCount) {
	//	m_manager->DestoryRigidBody(this);
	//}
}

void RigidBodyObject::PlaceCellsToNewPositions()
{
	SandboxMap* map = m_manager->GetOwnerMap();
	if (!map) return;

	// 计算覆盖范围
	AABB2 coverageBounds = CalculateRotatedCellCoverage();
	int minX = (int)floorf(coverageBounds.m_mins.x);
	int minY = (int)floorf(coverageBounds.m_mins.y);
	int maxX = (int)ceilf(coverageBounds.m_maxs.x);
	int maxY = (int)ceilf(coverageBounds.m_maxs.y);
	AddVertsForAABB2D(m_positionVerts, coverageBounds, Rgba8(255,0,0,100));

	// ==========================================
	// 阶段1：构建 local 坐标到 cell 数据的查找表
	// ==========================================
	//std::unordered_map<IntVec2, Cell, IntVec2Hash> localCellLookup;
	//for (Cell* oldCellPtr : m_cells) 
	//{
	//	auto it_local = m_cellToLocal.find(oldCellPtr);
	//	auto it_world = m_cellToWorldCoords.find(oldCellPtr);

	//	if (it_local == m_cellToLocal.end() || it_world == m_cellToWorldCoords.end()) {
	//		continue;
	//	}

	//	Vec2 localPos = it_local->second;
	//	IntVec2 localKey((int)floorf(localPos.x), (int)floorf(localPos.y));
	//	IntVec2 oldWorldCoords = it_world->second;

	//	// 保存cell数据快照和旧世界坐标
	//	localCellLookup[localKey] = *oldCellPtr;
	//}

	std::vector<Cell*> validCells;
	//std::unordered_map<Cell*, Vec2> validMapping;
	std::unordered_map<Cell*, IntVec2> validCellToWorldCoords;
	std::unordered_map<IntVec2, Cell, IntVec2Hash> newWorldToOldCellData;  // 新世界坐标 -> 旧cell数据
	std::unordered_map<Cell*, Cell> newCellPtrToOldCellData;


	// === 遍历框中应该被填充的cell
	for (int worldY = minY; worldY <= maxY; worldY++) 
	{
		for (int worldX = minX; worldX <= maxX; worldX++)
		{
			// 边界检查
			if (!map->IsInBounds(worldX, worldY)) 
			{
				continue;
			}

			// === 逆向变换：世界坐标 → local 坐标 ===
			Vec2 worldPos((float)worldX+0.5f, (float)worldY + 0.5f);
			Vec2 localPos = WorldToLocal(worldPos);

			IntVec2 localKey(
				(int)floorf(localPos.x),
				(int)floorf(localPos.y)
			);

			// 检查该 local 坐标是否有 cell
			//auto it_cellData = localCellLookup.find(localKey);
			//if (it_cellData == localCellLookup.end()) {
			//	continue;  // 该位置没有 cell，跳过
			//}
			auto it_cellData2 = m_cellBlueprint.find(localKey);
			if (it_cellData2 == m_cellBlueprint.end()) {
				continue;  // 该位置没有 cell，跳过
			}

			// 找到了
			//AddVertsForDisc2D(m_positionVerts, Vec2((float)worldX, (float)worldY) + Vec2(0.5f, 0.5f), 0.5f,Rgba8::CYAN);
			Cell& newCell = map->GetCellInChunk(worldX, worldY);

			validCells.push_back(&newCell);
			//validMapping[&newCell]= localPos;
			validCellToWorldCoords[&newCell] = IntVec2(worldX,worldY);

			//newWorldToOldCellData[IntVec2(worldX, worldY)] = it_cellData->second;
			newCellPtrToOldCellData[&newCell] = it_cellData2->second;

			map->GetChunkByWorldPos(worldX, worldY)->MarkDirty();
		}
	}

	for (Cell* cellPtr : validCells) 
	{
		auto it_world = validCellToWorldCoords.find(cellPtr);
		if (it_world != validCellToWorldCoords.end()) 
		{
			IntVec2 worldCoords = it_world->second;
			Vec2 worldPos((float)worldCoords.x + 0.5f, (float)worldCoords.y + 0.5f);

			// 画黄色方框（略大于cell，便于区分）
			AABB2 cellBounds(
				Vec2((float)worldCoords.x + 0.1f, (float)worldCoords.y + 0.1f),
				Vec2((float)worldCoords.x + 0.9f, (float)worldCoords.y + 0.9f)
			);
			Cell cellData = newCellPtrToOldCellData[cellPtr];
			//AddVertsForAABB2D(m_positionVerts, cellBounds, cellData.m_color);
		}
	}

	//// 清空旧的位置
	for (Cell* oldCellPtr : m_cells)
	{
		oldCellPtr->SetEmpty();
	}

	m_cells.clear();
	m_cells = validCells;
	//////// 赋值新的位置
	for (Cell* newCellPtr : m_cells)
	{
		*newCellPtr = newCellPtrToOldCellData[newCellPtr];
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

AABB2 RigidBodyObject::CalculateRotatedCellCoverage() const
{
	//if (m_cells.empty()) 
	//{
	//	return AABB2(m_position, m_position);
	//}

	//float minX = FLT_MAX;
	//float minY = FLT_MAX;
	//float maxX = -FLT_MAX;
	//float maxY = -FLT_MAX;

	//for (const auto& pair : m_cellToLocal) 
	//{
	//	Vec2 localPos = pair.second;
	//	Vec2 worldPos = LocalToWorld(localPos);

	//	minX = std::min(minX, worldPos.x);
	//	minY = std::min(minY, worldPos.y);
	//	maxX = std::max(maxX, worldPos.x);
	//	maxY = std::max(maxY, worldPos.y);
	//}

	//// 向外扩展半个cell,确保覆盖边界情况
	//return AABB2(
	//	Vec2(minX - 0.5f, minY - 0.5f),
	//	Vec2(maxX + 0.5f, maxY + 0.5f)
	//);

	if (m_cellBlueprint.empty())
	{
		return AABB2(m_position, m_position);
	}

	float minX = FLT_MAX;
	float minY = FLT_MAX;
	float maxX = -FLT_MAX;
	float maxY = -FLT_MAX;

	// ⭐ 遍历蓝图，计算每个cell在当前旋转下的世界位置
	for (const auto& pair : m_cellBlueprint)
	{
		IntVec2 localKey = pair.first;

		// 将localKey转换回浮点local坐标（中心位置）
		Vec2 localPos((float)localKey.x + 0.5f, (float)localKey.y + 0.5f);

		// 转换到世界坐标
		Vec2 worldPos = LocalToWorld(localPos);

		minX = std::min(minX, worldPos.x);
		minY = std::min(minY, worldPos.y);
		maxX = std::max(maxX, worldPos.x);
		maxY = std::max(maxY, worldPos.y);
	}

	// 向外扩展半个cell，确保覆盖边界情况
	return AABB2(
		Vec2(minX - 0.5f, minY - 0.5f),
		Vec2(maxX + 0.5f, maxY + 0.5f)
	);
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
	//m_cellToLocal[cell] = localPos;

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

	//m_cellToLocal.erase(cell);

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
