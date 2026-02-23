#pragma once
#include "Engine/JobSystem/Job.hpp"
#include <vector>
#include "RigidBodyPrecomputedData.hpp"

class ChunkRigidBodyManager;
struct CellWithCoords;

class ChunkRigidBodyDetectionJob : public Job
{
public:
	explicit ChunkRigidBodyDetectionJob(ChunkRigidBodyManager* manager);

	virtual void Execute() override;

	// 在主线程完成创建
	void FinishInMainThread();

private:
	ChunkRigidBodyManager* m_manager;
	std::vector<std::vector<CellWithCoords>> m_components;
	std::vector<RigidBodyPrecomputedData> m_precomputedData; // 新增：预计算数据
};
