#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "TileLayer.hpp"
#include "Tileset.hpp"
class TileMap
{
public:
	TileMap(XmlElement* rootElement);
	~TileMap();

	void Render() const;

	const std::vector<TileLayer>& GetLayers() const { return m_layers; }
	const std::vector<Tileset*>& GetTilesets() const { return m_tilesets; }
	IntVec2 GetSize() const { return m_size; }
	IntVec2 GetTileSize() const { return m_tileSize; }
	bool IsInfinite() const { return m_isInfinite; }

	TileLayer* FindLayer(const std::string& name);
	TileLayer* FindLayer(int id);
	Tileset* FindTileset(int firstGid);

	uint32_t GetTileGidFromLayerID(int layerID,IntVec2 const& gridPos);

private:

public:
	uint32_t m_markLayerIndex = 0;
private:
	std::string m_name;
	std::vector<TileLayer> m_layers;
	std::vector<Tileset*> m_tilesets;
	IntVec2 m_size;                          // Tile Map size (count of tile)
	IntVec2 m_tileSize;                         // tile size (example: 16x16)
	int m_nextLayerId = 1;
	int m_nextObjId = 1;

	

	bool m_isInfinite = false;
	std::string m_orientation = "orthogonal";
	std::string m_renderOrder = "right-down";
};