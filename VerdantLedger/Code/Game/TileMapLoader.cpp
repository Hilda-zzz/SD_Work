#include "TileMapLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Tileset.hpp"
#include "TileMap.hpp"

TileMapLoader::TileMapLoader()
{
}

TileMapLoader::~TileMapLoader()
{
}

TileMap* TileMapLoader::LoadTileMapFromFile(const std::string& filePath)
{
	XmlDocument tileMapTMX;
	XmlResult result = tileMapTMX.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS,
		Stringf("Failed to open required tile  defs file \"%s\"", filePath));

	XmlElement* rootElement = tileMapTMX.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find root element");

	TileMap* newTileMap = new TileMap(rootElement);
	return newTileMap;
}

Tileset* TileMapLoader::LoadTilesetFromFile(const std::string& tilesetPath)
{
	XmlDocument tilesetTSX;
	XmlResult result = tilesetTSX.LoadFile(tilesetPath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, 
		Stringf("Failed to open required tile  defs file \"%s\"", tilesetPath));

	XmlElement* rootElement = tilesetTSX.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find root element");

	Tileset* newTileset = new Tileset(rootElement);
	return newTileset;
}
