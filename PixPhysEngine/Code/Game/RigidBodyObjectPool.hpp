#pragma once
#include <vector>
#include <queue>
#include <unordered_set>
#include "ThirdParty/box2d/include/box2d/types.h"

class RigidBodyObject;

// ========================================
// RigidBodyObjectPool
// 全局刚体对象池，管理所有RigidBodyObject的分配和回收
// ========================================
class RigidBodyObjectPool
{
public:
	RigidBodyObjectPool();
	~RigidBodyObjectPool();

	// === Initialization ===
	void Initialize(b2WorldId worldId, int initialCapacity = 500);
	void Shutdown();

	// === Object Acquisition ===
	RigidBodyObject* Acquire();                    // 获取一个可用对象
	void Release(RigidBodyObject* obj);            // 归还对象到池

	// === Statistics ===
	int GetActiveCount() const;                    // 获取当前激活对象数
	int GetAvailableCount() const;                 // 获取可用对象数
	int GetTotalCapacity() const;                  // 获取池总容量

	// === Accessors ===
	b2WorldId GetPhysicsWorldId() const { return m_b2WorldId; }

private:
	void ExpandPool(int additionalCapacity);       // 扩容（如果需要）
	int FindObjectIndex(RigidBodyObject* obj) const; // 查找对象在池中的索引

private:
	// === Pool Data ===
	std::vector<RigidBodyObject*> m_pool;          // 所有对象（池的本体）
	std::queue<int> m_availableIndices;            // 可用对象的索引队列
	std::unordered_set<int> m_activeIndices;       // 当前激活的对象索引

	// === Physics World ===
	b2WorldId m_b2WorldId;                         // Box2D物理世界引用

	// === Configuration ===
	int m_initialCapacity;                         // 初始容量
	int m_maxCapacity;                             // 最大容量（防止无限扩张）

	// === ID Management ===
	int m_nextId;                                  // 用于生成唯一ID

	// === State ===
	bool m_isInitialized;                          // 是否已初始化
};