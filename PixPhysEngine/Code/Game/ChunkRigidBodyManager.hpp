#pragma once
#include <vector>
#include "ThirdParty/box2d/include/box2d/types.h"
#include "Game/Cell.hpp"
#include "Game/RigidBodyPrecomputedData.hpp"

class CellChunk;
class RigidBodyObject;
class RigidBodyObjectPool;
class GameMap;

// ========================================
// ChunkRigidBodyManager
// Chunk级别的刚体管理器，管理单个CellChunk内部的刚体
// ========================================
class ChunkRigidBodyManager
{
public:
	ChunkRigidBodyManager(CellChunk* ownerChunk, RigidBodyObjectPool* objectPool);
	~ChunkRigidBodyManager();

	// === Initialization ===
	void Initialize();

	// === Rigid Body Detection & Creation ===
	void DetectAndCreateRigidBodies();             // 扫描并生成刚体

	// 新增：分离职责
	std::vector<std::vector<CellWithCoords>> DetectConnectedComponents();  // 线程安全，可在worker线程
	void CreateRigidBodiesFromComponents(std::vector<std::vector<CellWithCoords>>& components);  // 主线程

	// 新增：从预计算数据创建刚体
	void CreateRigidBodiesFromPrecomputedData(
		std::vector<RigidBodyPrecomputedData>& precomputedData);

	// === Update ===
	void UpdateRigidBodies(float deltaTime);       // 更新所有主刚体
	void UpdateDetectionInterval();                // 根据可见性调整检测间隔

	// === Activation Control ===
	void ActivateAll();                            // 激活所有刚体
	void DeactivateAll();                          // 失活所有刚体

	// === Cleanup ===
	void ClearAll();                               // 清空并归还所有刚体到池

	// === Visibility ===
	void SetVisible(bool visible);                 // 设置可见性（影响更新频率）
	bool IsVisible() const { return m_isVisible; }

	// === Dirty ===
	void SetRbDirty(bool isDirty);
	bool IsRbDirty() const { return m_rbDirty; }

	// === Accessors ===
	int GetLocalRigidBodyCount() const;            // 获取主刚体数量
	int GetBoundaryRefCount() const;               // 获取边界刚体引用数量
	bool IsActive() const { return m_isActive; }

	// === Boundary Management ===
	void AddBoundaryReference(RigidBodyObject* rb); // 添加边界刚体引用
	void AddLocalRigidBody(RigidBodyObject* rb);    // 添加主刚体

	bool ShouldDetectRigidBodies() const;

	enum class RbJobState
	{
		Idle,           // 空闲
		Processing,     // 正在处理（Job执行中）
		Completed       // 完成
	};

	void MarkAsProcessing() { m_jobState = RbJobState::Processing; }
	void MarkAsUpdated() 
	{
		m_framesSinceLastDetection = 0;
		m_rbDirty = false;
		m_jobState = RbJobState::Idle;
	}

	CellChunk* GetOwnerChunk() const { return m_ownerChunk; }

private:
	void CollectSolidCells(std::vector<struct CellWithCoords>& outCells);
	

private:
	// === Owner ===
	CellChunk* m_ownerChunk;                       // 所属chunk
	RigidBodyObjectPool* m_objectPool;             // 全局对象池引用

	// === Rigid Bodies ===
	std::vector<RigidBodyObject*> m_localRigidBodies;   // 主刚体列表（质心在本chunk）
	std::vector<RigidBodyObject*> m_boundaryRefs;       // 边界刚体引用（质心在邻接chunk）看起来没啥用

	// === State ===
	bool m_isActive;                               // 是否激活
	bool m_isVisible;                              // 是否在视锥内
	bool m_rbDirty = false;

	RbJobState m_jobState = RbJobState::Idle;

	// === Detection Timing ===
	int m_framesSinceLastDetection;                // 距离上次检测的帧数
	int m_detectionInterval;                       // 检测间隔（帧数）

	// === Configuration ===
	static constexpr int DETECTION_INTERVAL = 10;
	static constexpr int DETECTION_INTERVAL_VISIBLE = 60;    // 视锥内检测间隔
	static constexpr int DETECTION_INTERVAL_INVISIBLE = 120; // 视锥外检测间隔
	static constexpr int MIN_CELL_COUNT = 10;                // 最小cell数量
};