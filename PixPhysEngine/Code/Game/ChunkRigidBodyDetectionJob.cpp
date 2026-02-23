#include "ChunkRigidBodyDetectionJob.hpp"
#include "Game/ChunkRigidBodyManager.hpp"
#include <ThirdParty/tracy/tracy/Tracy.hpp>
#include "Box2DShapeBuilder.hpp"

ChunkRigidBodyDetectionJob::ChunkRigidBodyDetectionJob(ChunkRigidBodyManager* manager)
	: m_manager(manager)
{
}

void ChunkRigidBodyDetectionJob::Execute()
{
	ZoneScoped;

	// Worker线程：只做计算
	m_components = m_manager->DetectConnectedComponents();


	//-------------------new -----------------------
	if (m_components.empty()) {
		return;
	}

	// 步骤2：为每个组件预计算所有数据（耗时操作都在这里！）
	m_precomputedData.reserve(m_components.size());

	for (auto& component : m_components)
	{
		RigidBodyPrecomputedData data;
		data.originalCells = std::move(component);
		data.bodyType = b2_staticBody;  // 从manager获取

		if (data.originalCells.size() < 10)
		{
			data.isValid = false;
			m_precomputedData.push_back(std::move(data));
			continue;
		}

		// =================== 虚拟填充空洞 ===================
		{
			ZoneScopedN("Virtual Fill Holes");

			// 检测空洞
			data.virtualFilledCoords = Box2DShapeBuilder::DetectHolesToFill(data.originalCells);

			// 创建完整的cell列表（用于刚体生成）
			data.cellsForRigidBody = data.originalCells;  // 先复制原始cells

			if (!data.virtualFilledCoords.empty())
			{
				// 添加虚拟填充的cells
				auto virtualCells = Box2DShapeBuilder::CreateVirtualFilledCells(
					data.originalCells,
					data.virtualFilledCoords
				);

				data.cellsForRigidBody.insert(
					data.cellsForRigidBody.end(),
					virtualCells.begin(),
					virtualCells.end()
				);

				data.hasVirtualFill = true;
			}
		}

		// =================== 基于完整列表生成刚体数据 ===================
		{
			ZoneScopedN("Precompute RigidBody Data");

			// 使用填充后的完整列表提取轮廓
			data.outlinePoints = Box2DShapeBuilder::ExtractOutlineFromCellsSimple(
				data.cellsForRigidBody  // 使用填充后的！
			);

			if (data.outlinePoints.empty())
			{
				data.isValid = false;
				m_precomputedData.push_back(std::move(data));
				continue;
			}

			// 计算质心
			data.centroid = Box2DShapeBuilder::CalculateCentroid(data.outlinePoints);

			// 三角剖分
			TriangulationOutput triangulation = Box2DShapeBuilder::CDTTriangulation(
				data.outlinePoints
			);

			// 转换为局部坐标
			data.triangleVertices.reserve(triangulation.vertices.size());
			for (const Vec2& v : triangulation.vertices)
			{
				data.triangleVertices.push_back(v - data.centroid);
			}
			data.triangleIndices = std::move(triangulation.indices);

			// 构建静态cell坐标映射（只包含真实的cells！）
			if (data.bodyType == b2_staticBody)
			{
				data.staticCellCoords.reserve(data.originalCells.size());
				for (const CellWithCoords& cellData : data.originalCells)
				{
					data.staticCellCoords.emplace(cellData.m_cell, cellData.m_worldCoords);
				}
				// 注意：虚拟填充的坐标不加入这个映射
			}

			data.isValid = true;
		}

		m_precomputedData.push_back(std::move(data));
	}
}

void ChunkRigidBodyDetectionJob::FinishInMainThread()
{
	ZoneScoped;

	// 主线程：清空旧数据
	//m_manager->ClearAll();

	// 主线程：创建Box2D对象
	m_manager->CreateRigidBodiesFromPrecomputedData(m_precomputedData);
	m_manager->MarkAsUpdated();
}