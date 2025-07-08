#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/TileLayer.hpp"
#include "Game/Tileset.hpp"
#include "Game/TileTypesInGame.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


class GroundObstacle;

class TileMap
{
public:
	TileMap(XmlElement* rootElement);
	~TileMap();

	void Update(float deltaSeconds);
	void Render() const;

	const std::vector<TileLayer*>& GetLayers() const { return m_layers; }
	const std::vector<Tileset*>& GetTilesets() const { return m_tilesets; }
	IntVec2 GetSize() const { return m_size; }
	IntVec2 GetTileSize() const { return m_tileSize; }
	bool IsInfinite() const { return m_isInfinite; }

	TileLayer* FindLayer(const std::string& name);
	TileLayer* FindLayer(int id);
	Tileset* FindTileset(int firstGid);

	uint32_t GetTileGidFromLayerID(int layerID,IntVec2 const& gridPos);
	static uint64_t GetTileKey(IntVec2 const& gridPos);
	static IntVec2 GetGridPosByTileKey(uint64_t tileKey);

	template<typename Func>
	void ForEachTileInLayer(int layerId, Func callback);

	TileLayer* FindLayerById(int layerId);

	void UpdateTransparentObject(IntVec2 const& aimGridPos);

private:
	IntVec2 m_lastTransparentTreePos = IntVec2(-999, -999);

public:
	uint32_t m_markLayerId = 0;
	TileLayer* m_markLayer = nullptr;
	std::unordered_map<uint64_t, DynamicTileData> m_dynamicTiles;
	RandomNumberGenerator m_rng;
	std::vector<GroundObstacle*> m_obstacles;

private:
	std::string m_name;
	std::vector<TileLayer*> m_layers;
	std::vector<Tileset*> m_tilesets;
	// use grid,z and grid.y to get a mask id for each tile 
	IntVec2 m_size;                        // Tile Map size (count of tile)
	IntVec2 m_tileSize;                    // tile size (example: 16x16)
	int m_nextLayerId = 1;
	int m_nextObjId = 1;
	bool m_isInfinite = false;
	std::string m_orientation = "orthogonal";
	std::string m_renderOrder = "right-down";
};

template<typename Func>
inline void TileMap::ForEachTileInLayer(int layerId, Func callback)
{
	TileLayer* layer = FindLayerById(layerId);
	if (!layer) return;

	for (const auto& chunk : layer->m_chunks) 
	{
		if (chunk.m_isEmpty) continue;

		IntVec2 chunkStart = chunk.m_startPosition;
		IntVec2 chunkSize = chunk.m_size;

		for (int localY = 0; localY < chunkSize.y; ++localY) 
		{
			for (int localX = 0; localX < chunkSize.x; ++localX) 
			{
				int dataIndex = localY * chunkSize.x + localX;
				if (dataIndex < chunk.m_terrianData.size()) 
				{
					uint32_t gid = chunk.m_terrianData.at(dataIndex);
					IntVec2 gridPos = chunkStart + IntVec2(localX, -localY);

					callback(gridPos, gid);
				}
			}
		}
	}
}
