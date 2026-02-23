#include "CellDataCollectJob.hpp"
#include "CellChunk.hpp"
#include "CellMatManager.hpp"
#include "Cell.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

CellDataCollectJob::CellDataCollectJob(CellChunk* chunk)
	: m_chunk(chunk)
{
	m_collectedData.reserve(4096);  // 预分配
}

void CellDataCollectJob::Execute()
{
	if (!m_chunk) return;

	// 并行收集这个Chunk的Cell数据
	for (int localY = 0; localY < 64; ++localY)
	{
		for (int localX = 0; localX < 64; ++localX)
		{
			const Cell& cell = m_chunk->GetLocalCell(localX, localY);
			if (cell.IsEmpty()) continue;

			IntVec2 worldPos = m_chunk->LocalToWorld(localX, localY);

			const CellMatDef& matDef = CellMatManager::GetMaterialDef(cell.m_type);
			uint8_t physicsType = (uint8_t)matDef.m_physicsType;
			uint8_t emission = matDef.m_isHighTemp ? 63 : matDef.m_emissionValue;

			//// ← 添加验证
			//CellMatType cellType = cell.m_type.load();
			//if ((uint8_t)cellType >= (uint8_t)CellMatType::COUNT)  // 假设有MAT_COUNT枚举
			//{
			//	DebuggerPrintf("WARNING: Invalid cell type %u at (%d, %d)\n",
			//		(uint8_t)cellType, worldPos.x, worldPos.y);
			//	continue;
			//}

			//if (physicsType == 4) physicsType=2;
			//if (physicsType == 4)  // PHY_CELLULAR_AUTOMATON
			//{
			//	DebuggerPrintf("CA Cell at (%d,%d): type=%u, color=(%u,%u,%u,%u), emission=%u\n",
			//		worldPos.x, worldPos.y,
			//		(uint8_t)cell.m_type.load(),
			//		cell.m_color.r, cell.m_color.g, cell.m_color.b, cell.m_color.a,
			//		emission);
			//}

			GPUCellData cellData = GPUCellData::Pack(
				worldPos,
				cell.m_color,
				physicsType,
				emission
			);

			m_collectedData.push_back(cellData);
		}
	}
}