#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/TileTypesInGame.hpp"
#include <unordered_map>

class RuledTileset;
class GroundObstacle;

class TileChunk
{
public:
	TileChunk();
	~TileChunk();

	void Update(float deltaSeconds);

	void InitializeChunkVerts();
	void UpdateObstacleVerts();
	void UpdateFarmlandVerts();

	void RenderDynamicContent() const;

	IntVec2 GetPosition() const { return m_startPosition; }
	IntVec2 GetSize() const { return m_size; }
	bool IsEmpty() const { return m_isEmpty; }

	uint32_t GetTile(IntVec2 const& gridPos) const;
	IntVec2 GetGridPos(int tileIndex) const;
	void SetTile(int localX, int localY, uint32_t tileId);

	IntVec2 ToWorldPos(int localX, int localY) const {
		return IntVec2(m_startPosition.x * m_size.x + localX, m_startPosition.y * m_size.y + localY);
	}

	template<typename Func>
	void ForEachTileInChunk(Func callback);

private:
	uint8_t GetPlowedNeighborMask(IntVec2 const& tileGridPos);

public:
	IntVec2 m_startPosition;     
	IntVec2 m_size; 
	bool m_isEmpty = false;           // may be deleted later
	std::vector<uint32_t> m_terrianData;
	std::vector<Vertex_PCU> m_terrianVerts;

	std::unordered_map<uint64_t, DynamicTileData> m_keyToDynamicTileData; //grid pos key to data
	std::vector<Vertex_PCU> m_plowedFarmlandVerts;
	std::vector<Vertex_PCU> m_wateredFarmlandVerts;
	std::vector<Vertex_PCU> m_staticObstacleVerts;
	
	std::vector<GroundObstacle*> m_obstacleWithAnimation;
	std::unordered_map<uint64_t, GroundObstacle*> m_gridPosToGroundObstacle;

	bool m_isDirtyObstacle = false;
	bool m_isDirtyFarmland = false;
private:
	RuledTileset* m_plowedSoilRuleSet = nullptr;
	RuledTileset* m_waterSoilRuleSet = nullptr;
};

template<typename Func>
inline void TileChunk::ForEachTileInChunk(Func callback)
{
	for (int localY = 0; localY < m_size.y; ++localY)
	{
		for (int localX = 0; localX < m_size.x; ++localX)
		{
			int dataIndex = localY * m_size.x + localX;
			if (dataIndex < (int)m_terrianData.size())
			{
				uint32_t gid = m_terrianData.at(dataIndex);
				IntVec2 gridPos = m_startPosition + IntVec2(localX, -localY);

				callback(gridPos, gid);
			}
		}
	}
}
