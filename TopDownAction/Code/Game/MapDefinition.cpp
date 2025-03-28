#include "MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<MapDefinition> MapDefinition::s_mapDefinitions;

MapDefinition::MapDefinition(XmlElement const* mapDefElement)
{
	//m_name = ParseXmlAttribute(mapDefElement, "name", m_name);
	m_mapImageName= ParseXmlAttribute(mapDefElement, "mapFloorImageName", m_mapImageName);
	m_mapWallImageName = ParseXmlAttribute(mapDefElement, "mapWallImageName", m_mapImageName);
	m_dimensions = ParseXmlAttribute(mapDefElement, "dimensions", m_dimensions);
	//m_startCoords = ParseXmlAttribute(mapDefElement, "startCoords", m_startCoords);
	//m_exitCoords= ParseXmlAttribute(mapDefElement, "exitCoords", m_exitCoords);
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
