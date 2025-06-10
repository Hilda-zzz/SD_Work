#include "TileMap.hpp"
#include "TileManager.hpp"
#include "Engine/Core/StringUtils.hpp"

TileMap::TileMap(XmlElement* rootElement)
{
	int mapWidth = ParseXmlAttribute(rootElement, "width", 0);
	int mapHeight = ParseXmlAttribute(rootElement, "height", 0);
	m_size = IntVec2(mapWidth, mapHeight);

	int tileWidth = ParseXmlAttribute(rootElement, "tilewidth", 16);
	int tileHeight = ParseXmlAttribute(rootElement, "tileheight", 16);
	m_tileSize = IntVec2(tileWidth, tileHeight);

	m_isInfinite = ParseXmlAttribute(rootElement, "infinite", 0) == 1;

	// tileset
	for (XmlElement* tilesetElement = rootElement->FirstChildElement("tileset");
		tilesetElement; tilesetElement = tilesetElement->NextSiblingElement("tileset")) 
	{
		std::string tilesetSource = ParseXmlAttribute(tilesetElement, "source", "");
		Strings sourceArr=SplitStringOnDelimiter(tilesetSource, '.');
		Tileset* tilesetRef=TileMapManager::GetInstance().m_loadedTilesets[sourceArr[0]];
		int firstGid = ParseXmlAttribute(tilesetElement, "firstgid", 1);
		tilesetRef->SetFirstGid(firstGid);
		m_tilesets.push_back(tilesetRef);
	}

// 	for (XmlElement* layerElement = rootElement->FirstChildElement("layer");
// 		layerElement; layerElement = layerElement->NextSiblingElement("layer")) 
// 	{
// 		MapLayer layer;
// 		layer.name = ParseXmlAttribute(layerElement, "name", std::string());
// 		layer.width = ParseXmlAttribute(layerElement, "width", 0);
// 		layer.height = ParseXmlAttribute(layerElement, "height", 0);
// 
// 		XmlElement* dataElement = layerElement->FirstChildElement("data");
// 		if (dataElement) 
// 		{
// 			for (XmlElement* chunkElement = dataElement->FirstChildElement("chunk");
// 				chunkElement; chunkElement = chunkElement->NextSiblingElement("chunk")) {
// 				MapChunk chunk;
// 				chunk.x = ParseXmlAttribute(chunkElement, "x", 0);
// 				chunk.y = ParseXmlAttribute(chunkElement, "y", 0);
// 				chunk.width = ParseXmlAttribute(chunkElement, "width", 0);
// 				chunk.height = ParseXmlAttribute(chunkElement, "height", 0);
// 
// 				const char* csvText = chunkElement->GetText();
// 				if (csvText) 
// 				{
// 					std::string data(csvText);
// 					chunk.tileData = ParseCsvToIntArray(data);
// 				}
// 				layer.chunks.push_back(chunk);
// 			}
// 		}
// 		m_layers.push_back(layer);
// 	}
}

TileMap::~TileMap()
{
}
