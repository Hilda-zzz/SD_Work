#pragma once
#include <string>
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
class TileMap;
class TileLayer;
class Tileset;

class TileMapLoader 
{
public:
	TileMapLoader();
	~TileMapLoader();

	TileMap* LoadTileMapFromFile(const std::string& filePath);
	Tileset* LoadTilesetFromFile(const std::string& tilesetPath);

private:
	//std::string m_mapDirectory; 

	bool ParseMapHeader(XmlElement* mapElement, TileMap& map);
	bool ParseTilesets(XmlElement* mapElement, TileMap& map);
	bool ParseLayers(XmlElement* mapElement, TileMap& map);
	bool ParseTileLayer(XmlElement* layerElement, TileLayer& layer);
	bool ParseObjectLayer(XmlElement* layerElement, TileLayer& layer);
	bool ParseChunk(XmlElement* chunkElement, TileLayer& layer);

	std::vector<uint32_t> DecodeLayerData(const std::string& encoding,
		const std::string& compression,
		const std::string& data,
		int expectedCount);
};