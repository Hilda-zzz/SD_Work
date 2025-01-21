#include "MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<MapDefinition> MapDefinition::s_mapDefinitions;

MapDefinition::MapDefinition(XmlElement const* mapDefElement)
{
	m_name = ParseXmlAttribute(mapDefElement, "name", m_name);
	m_mapImageName= ParseXmlAttribute(mapDefElement, "mapImageName", m_mapImageName);
	m_dimensions = ParseXmlAttribute(mapDefElement, "dimensions", m_dimensions);
	m_startCoords = ParseXmlAttribute(mapDefElement, "startCoords", m_startCoords);
	m_exitCoords= ParseXmlAttribute(mapDefElement, "exitCoords", m_exitCoords);

	m_fillTileType = ParseXmlAttribute(mapDefElement, "fillTileType", m_fillTileType);
	m_edgeTileType = ParseXmlAttribute(mapDefElement, "edgeTileType", m_edgeTileType);

	m_worm1TileType = ParseXmlAttribute(mapDefElement, "worm1TileType", m_worm1TileType); 
	m_worm1Count = ParseXmlAttribute(mapDefElement, "worm1Count", m_worm1Count);
	m_worm1MaxLen = ParseXmlAttribute(mapDefElement, "worm1MaxLen", m_worm1MaxLen);

	m_worm2TileType = ParseXmlAttribute(mapDefElement, "worm2TileType", m_worm2TileType); 
	m_worm2Count = ParseXmlAttribute(mapDefElement, "worm2Count", m_worm2Count);
	m_worm2MaxLen = ParseXmlAttribute(mapDefElement, "worm2MaxLen", m_worm2MaxLen);

	m_waterTileType = ParseXmlAttribute(mapDefElement, "waterTileType", m_waterTileType);
	m_wormWaterCount = ParseXmlAttribute(mapDefElement, "wormWaterCount", m_wormWaterCount);
	m_wormWaterMaxLen = ParseXmlAttribute(mapDefElement, "wormWaterMaxLen", m_wormWaterMaxLen);

	m_startFloorTileType = ParseXmlAttribute(mapDefElement, "startFloorTileType", m_startFloorTileType);
	m_startBunkerTileType = ParseXmlAttribute(mapDefElement, "startBunkerTileType", m_startBunkerTileType);
	m_endFloorTileType = ParseXmlAttribute(mapDefElement, "endFloorTileType", m_endFloorTileType);
	m_endBunkerTileType = ParseXmlAttribute(mapDefElement, "endBunkerTileType", m_endBunkerTileType);
	m_startTileType = ParseXmlAttribute(mapDefElement, "startTileType", m_startTileType);
	m_endTileType = ParseXmlAttribute(mapDefElement, "endTileType", m_endTileType);

	m_rubbleCount= ParseXmlAttribute(mapDefElement, "rubbleCount", m_rubbleCount);
	m_leoCount = ParseXmlAttribute(mapDefElement, "leoCount", m_leoCount);
	m_ariesCount = ParseXmlAttribute(mapDefElement, "ariesCount", m_ariesCount);
	m_scorpioCount = ParseXmlAttribute(mapDefElement, "scorpioCount", m_scorpioCount);
	m_capricornCount = ParseXmlAttribute(mapDefElement, "capricornCount", m_capricornCount);
	m_treeCount = ParseXmlAttribute(mapDefElement, "treeCount", m_treeCount);
}

void MapDefinition::InitializeMapDefinitionFromFile()
{
	XmlDocument mapDefsXml;
	char const* filePath = "Data/Definitions/MapDefinitions.xml";
	XmlResult result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required map defs file \"%s\"", filePath));

	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find map root element");

	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement)
	{
		std::string elementName = mapDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "MapDefinition", Stringf("Root child element in %s was <%s>, must be <MapDefinition>!", filePath, elementName.c_str()));
		MapDefinition* newMapDef = new MapDefinition(mapDefElement);
		s_mapDefinitions.push_back(*newMapDef);
		mapDefElement = mapDefElement->NextSiblingElement();
	}
}
