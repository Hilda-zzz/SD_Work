#include "ChunkRigidBodyManager.hpp"
#include "CellChunk.hpp"
#include "RigidBodyObject.hpp"
#include "RigidBodyObjectPool.hpp"
#include "Box2DShapeBuilder.hpp"
#include "GameMap.hpp"
#include "Cell.hpp"
#include "CellMatManager.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>
#include <ThirdParty/tracy/tracy/Tracy.hpp>
#include "RigidBodyPrecomputedData.hpp"

ChunkRigidBodyManager::ChunkRigidBodyManager(CellChunk* ownerChunk, RigidBodyObjectPool* objectPool)
	: m_ownerChunk(ownerChunk)
	, m_objectPool(objectPool)
	, m_isActive(false)
	, m_isVisible(false)
	, m_framesSinceLastDetection(0)
	, m_detectionInterval(DETECTION_INTERVAL_VISIBLE)
{
	GUARANTEE_OR_DIE(m_ownerChunk != nullptr, "ChunkRigidBodyManager: ownerChunk cannot be null");
	GUARANTEE_OR_DIE(m_objectPool != nullptr, "ChunkRigidBodyManager: objectPool cannot be null");
}

ChunkRigidBodyManager::~ChunkRigidBodyManager()
{
	ClearAll();
}

void ChunkRigidBodyManager::Initialize()
{
	// 初始化时可以执行一些设置
	// 目前为空，预留接口
}

void ChunkRigidBodyManager::DetectAndCreateRigidBodies()
{
	ZoneScoped;
	ClearAll();

	//---------------------------------------------
	// 1. 收集本chunk内的所有solid cells
	//std::vector<CellWithCoords> solidCells;
	//CollectSolidCells(solidCells);

	//if (solidCells.empty()) {
	//	return;
	//}

	//// 2. 分离连通组件（只在本chunk范围内）
	//auto components = Box2DShapeBuilder::SeparateConnectedComponentsMove(std::move(solidCells));

	//{
	//	ZoneScopedN("create rb");
	//	// 3. 为每个组件创建刚体
	//	for (auto& component : components) 
	//	{
	//		// 跳过太小的组件
	//		if (component.size() < MIN_CELL_COUNT) 
	//		{
	//			continue;
	//		}

	//		// 从对象池获取刚体
	//		RigidBodyObject* rb = m_objectPool->Acquire();
	//		if (!rb) 
	//		{
	//			DebuggerPrintf("Warning: Failed to acquire rigid body from pool\n");
	//			continue;
	//		}

	//		// 初始化刚体（GameMap模式中主要是静态地形）
	//		rb->Initialize(component, b2_staticBody);

	//		// TODO: 计算质心，确定所属chunk
	//		// 目前简化处理：假设所有组件的质心都在本chunk
	//		// Vec2 centerOfMass = rb->CalculateCenterOfMass();
	//		// CellChunk* ownerChunk = m_ownerChunk->GetMap()->GetChunkByWorldPos(...)

	//		rb->SetOwnerChunk(m_ownerChunk);
	//		rb->AddAffectedChunk(m_ownerChunk);

	//		// 添加到本chunk的主刚体列表
	//		m_localRigidBodies.push_back(rb);
	//	}
	//}
	//---------------------------------------------

	auto components = DetectConnectedComponents();  // ✅ 调用新函数
	
	CreateRigidBodiesFromComponents(components);    // ✅ 调用新函数
}

std::vector<std::vector<CellWithCoords>> ChunkRigidBodyManager::DetectConnectedComponents()
{
	ZoneScoped;

	std::vector<CellWithCoords> solidCells;
	CollectSolidCells(solidCells);

	if (solidCells.empty()) {
		return {};
	}

	// 纯计算，线程安全
	return Box2DShapeBuilder::SeparateConnectedComponentsMove(std::move(solidCells));
}

void ChunkRigidBodyManager::CreateRigidBodiesFromComponents(std::vector<std::vector<CellWithCoords>>& components)
{
	ZoneScopedN("create rb");

	for (auto& component : components)
	{
		if (component.size() < MIN_CELL_COUNT) {
			continue;
		}

		RigidBodyObject* rb = m_objectPool->Acquire();
		if (!rb) {
			DebuggerPrintf("Warning: Failed to acquire rigid body from pool\n");
			continue;
		}

		rb->Initialize(std::move(component), b2_staticBody);
		rb->SetOwnerChunk(m_ownerChunk);
		rb->AddAffectedChunk(m_ownerChunk);

		m_localRigidBodies.push_back(rb);
	}
}

void ChunkRigidBodyManager::CreateRigidBodiesFromPrecomputedData(std::vector<RigidBodyPrecomputedData>& precomputedData)
{
	ZoneScopedN("Create RB from Precomputed");

	for (auto& data : precomputedData)
	{
		if (!data.isValid) {
			continue;
		}

		RigidBodyObject* rb = m_objectPool->Acquire();
		if (!rb) {
			DebuggerPrintf("Warning: Failed to acquire rigid body from pool\n");
			continue;
		}

		// 使用预计算的数据快速初始化（不需要重新计算！）
		rb->InitializeFromPrecomputedData(std::move(data));

		rb->SetOwnerChunk(m_ownerChunk);
		rb->AddAffectedChunk(m_ownerChunk);

		m_localRigidBodies.push_back(rb);
	}
}

void ChunkRigidBodyManager::UpdateRigidBodies(float deltaTime)
{
	if (!m_isActive) {
		return;
	}

	// 1. 更新检测计数器
	m_framesSinceLastDetection++;

	// 2. 检查是否需要重新检测刚体
	if (ShouldDetectRigidBodies()) 
	{
		DetectAndCreateRigidBodies();
		m_framesSinceLastDetection = 0;
		m_rbDirty = false; //清楚dirty标志
	}

	// 3. 更新所有主刚体（静态刚体的Update实际上什么都不做）
	for (auto* rb : m_localRigidBodies) 
	{
		if (rb && rb->IsActive()) 
		{
			rb->Update();
		}
	}
}

void ChunkRigidBodyManager::UpdateDetectionInterval()
{
	if (m_isVisible) {
		m_detectionInterval = DETECTION_INTERVAL_VISIBLE;
	}
	else {
		m_detectionInterval = DETECTION_INTERVAL_INVISIBLE;
	}
}

void ChunkRigidBodyManager::ActivateAll()
{
	m_isActive = true;

	for (auto* rb : m_localRigidBodies) 
	{
		if (rb) 
		{
			rb->SetActive(true);
		}
	}
}

void ChunkRigidBodyManager::DeactivateAll()
{
	m_isActive = false;

	for (auto* rb : m_localRigidBodies) {
		if (rb) {
			rb->SetActive(false);
		}
	}
}

void ChunkRigidBodyManager::ClearAll()
{
	// 归还所有主刚体到对象池

	for (auto* rb : m_localRigidBodies) {
		if (rb && m_objectPool) {
			m_objectPool->Release(rb);
		}
	}

	m_localRigidBodies.clear();

	// 清空边界引用（这些刚体由其他chunk管理，我们只清除引用）
	m_boundaryRefs.clear();
}

void ChunkRigidBodyManager::SetVisible(bool visible)
{
	m_isVisible = visible;
	UpdateDetectionInterval();
}

void ChunkRigidBodyManager::SetRbDirty(bool isDirty)
{
	m_rbDirty = isDirty;
}

int ChunkRigidBodyManager::GetLocalRigidBodyCount() const
{
	return static_cast<int>(m_localRigidBodies.size());
}

int ChunkRigidBodyManager::GetBoundaryRefCount() const
{
	return static_cast<int>(m_boundaryRefs.size());
}

void ChunkRigidBodyManager::AddBoundaryReference(RigidBodyObject* rb)
{
	if (rb) {
		m_boundaryRefs.push_back(rb);
	}
}

void ChunkRigidBodyManager::AddLocalRigidBody(RigidBodyObject* rb)
{
	if (rb) {
		m_localRigidBodies.push_back(rb);
	}
}

void ChunkRigidBodyManager::CollectSolidCells(std::vector<CellWithCoords>& outCells)
{
	ZoneScoped;

	outCells.clear();
	outCells.reserve(CHUNK_SIZE* CHUNK_SIZE);
	// 遍历chunk内的所有cells
	for (int localY = 0; localY < CHUNK_SIZE; ++localY) 
	{
		for (int localX = 0; localX < CHUNK_SIZE; ++localX) 
		{
			Cell& cell = m_ownerChunk->GetLocalCell(localX, localY);

			// 检查是否是solid类型且不属于其他刚体
			if (!cell.IsEmpty() && !cell.m_isBelongRb) {
				const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);

				// 只收集固体类型的cells
				if (matDef.m_physicsType == PhyType::PHY_STATIC_SOLID)  //  || (matDef.m_physicsType == PhyType::PHY_MOVE_SOLID && !cell.m_isFreeFalling)
				{
					IntVec2 worldCoords = m_ownerChunk->LocalToWorld(localX, localY);
					outCells.push_back(CellWithCoords(&cell, worldCoords));
				}
			}
		}
	}
}

bool ChunkRigidBodyManager::ShouldDetectRigidBodies() const
{
	// 检测条件：
	// 1. 已激活
	// 2. 到达检测间隔
	//return m_isActive && m_rbDirty;


	//if (!m_isActive) return false;
	//if (!m_rbDirty) return false;
	//return true;
	//m_framesSinceLastDetection >= DETECTION_INTERVAL;
	if (m_jobState == RbJobState::Processing) return false;
	return m_rbDirty; //&&m_ownerChunk->GetIsVisible()
}