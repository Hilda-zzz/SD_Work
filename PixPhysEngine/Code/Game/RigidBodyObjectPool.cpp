#include "RigidBodyObjectPool.hpp"
#include "RigidBodyObject.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <ThirdParty/box2d/include/box2d/box2d.h>

RigidBodyObjectPool::RigidBodyObjectPool()
	: m_b2WorldId(b2_nullWorldId)
	, m_initialCapacity(500)
	, m_maxCapacity(2000)
	, m_nextId(0)
	, m_isInitialized(false)
{
}

RigidBodyObjectPool::~RigidBodyObjectPool()
{
	Shutdown();
}

void RigidBodyObjectPool::Initialize(b2WorldId worldId, int initialCapacity)
{
	if (m_isInitialized) 
	{
		DebuggerPrintf("Warning: RigidBodyObjectPool already initialized\n");
		return;
	}

	GUARANTEE_OR_DIE(!B2_IS_NULL(worldId), "Cannot initialize pool with null Box2D world");

	m_b2WorldId = worldId;
	m_initialCapacity = initialCapacity;
	m_isInitialized = true;

	// 预分配对象池
	m_pool.reserve(m_initialCapacity);

	for (int i = 0; i < m_initialCapacity; ++i) 
	{
		// 注意：这里创建对象时，RigidBodyManager传nullptr是因为对象池本身不需要manager
		// 实际使用时会在Acquire()时设置正确的manager
		RigidBodyObject* obj = new RigidBodyObject(m_nextId++, m_b2WorldId, nullptr);
		m_pool.push_back(obj);
		m_availableIndices.push(i);
	}

	DebuggerPrintf("RigidBodyObjectPool initialized with %d objects\n", m_initialCapacity);
}

void RigidBodyObjectPool::Shutdown()
{
	if (!m_isInitialized) 
	{
		return;
	}

	// 删除所有对象
	for (RigidBodyObject* obj : m_pool) 
	{
		delete obj;
	}

	m_pool.clear();

	// 清空队列和集合
	while (!m_availableIndices.empty()) 
	{
		m_availableIndices.pop();
	}
	m_activeIndices.clear();

	m_isInitialized = false;

	DebuggerPrintf("RigidBodyObjectPool shut down\n");
}

RigidBodyObject* RigidBodyObjectPool::Acquire()
{
	if (!m_isInitialized) 
	{
		DebuggerPrintf("Error: Pool not initialized\n");
		return nullptr;
	}

	// 如果没有可用对象，尝试扩容
	if (m_availableIndices.empty()) 
	{
		if (m_pool.size() >= m_maxCapacity) 
		{
			DebuggerPrintf("Warning: Pool reached max capacity (%d), cannot acquire more objects\n", m_maxCapacity);
			return nullptr;
		}

		// 扩容25%或至少10个
		int additionalCapacity = std::max(10, (int)(m_pool.size() * 0.25f));
		ExpandPool(additionalCapacity);
	}

	// 从可用队列中取出一个索引
	int index = m_availableIndices.front();
	m_availableIndices.pop();
	m_activeIndices.insert(index);

	RigidBodyObject* obj = m_pool[index];

	// 对象已经在构造时创建，这里只需要返回
	return obj;
}

void RigidBodyObjectPool::Release(RigidBodyObject* obj)
{
	if (!obj)
	{
		return;
	}

	if (!m_isInitialized) 
	{
		DebuggerPrintf("Warning: Trying to release object to uninitialized pool\n");
		return;
	}

	// 查找对象在池中的索引
	int index = FindObjectIndex(obj);

	if (index == -1) 
	{
		DebuggerPrintf("Warning: Trying to release object that doesn't belong to this pool\n");
		return;
	}

	// 检查是否已经在可用列表中
	if (m_activeIndices.find(index) == m_activeIndices.end()) 
	{
		DebuggerPrintf("Warning: Trying to release object that is already available\n");
		return;
	}

	// 重置对象状态
	obj->Reset();

	// 从激活集合移除，加入可用队列
	m_activeIndices.erase(index);
	m_availableIndices.push(index);
}

int RigidBodyObjectPool::GetActiveCount() const
{
	return static_cast<int>(m_activeIndices.size());
}

int RigidBodyObjectPool::GetAvailableCount() const
{
	return static_cast<int>(m_availableIndices.size());
}

int RigidBodyObjectPool::GetTotalCapacity() const
{
	return static_cast<int>(m_pool.size());
}

void RigidBodyObjectPool::ExpandPool(int additionalCapacity)
{
	int currentSize = static_cast<int>(m_pool.size());
	int newSize = std::min(currentSize + additionalCapacity, m_maxCapacity);

	DebuggerPrintf("Expanding pool from %d to %d objects\n", currentSize, newSize);

	for (int i = currentSize; i < newSize; ++i) 
	{
		RigidBodyObject* obj = new RigidBodyObject(m_nextId++, m_b2WorldId, nullptr);
		m_pool.push_back(obj);
		m_availableIndices.push(i);
	}
}

int RigidBodyObjectPool::FindObjectIndex(RigidBodyObject* obj) const
{
	for (int i = 0; i < m_pool.size(); ++i) 
	{
		if (m_pool[i] == obj) 
		{
			return i;
		}
	}
	return -1;
}