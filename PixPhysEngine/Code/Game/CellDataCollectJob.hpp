#pragma once
#include "Engine/JobSystem/Job.hpp"
#include "GPUCellData.hpp"
#include <vector>

class CellChunk;

class CellDataCollectJob : public Job
{
public:
	CellDataCollectJob(CellChunk* chunk);

	virtual void Execute() override;

	std::vector<GPUCellData> const& GetCollectedData() const { return m_collectedData; }

private:
	CellChunk* m_chunk;
	std::vector<GPUCellData> m_collectedData;
};