#include "TileDefinition.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

std::vector<TileDefinition> TileDefinition::s_tileDefinitions;

TileDefinition::TileDefinition(XmlElement const* tileDefElement)
{
	m_name = ParseXmlAttribute(tileDefElement, "name", m_name);
	m_isSolid = ParseXmlAttribute(tileDefElement, "isSolid", m_isSolid);
	m_mapImagePixelColor= ParseXmlAttribute(tileDefElement, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords= ParseXmlAttribute(tileDefElement, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceilingSpriteCoords= ParseXmlAttribute(tileDefElement, "ceilingSpriteCoords", m_ceilingSpriteCoords);
	m_wallSpriteCoords= ParseXmlAttribute(tileDefElement, "wallSpriteCoords", m_wallSpriteCoords);
}

void TileDefinition::InitializeTileDefinitionFromFile()
{
	XmlDocument tileDefsXml;
	char const* filePath = "Data/Definitions/TileDefinitions.xml";
	XmlResult result = tileDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required tile  defs file \"%s\"", filePath));

	XmlElement* rootElement = tileDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find root element");

	XmlElement* tileDefElement = rootElement->FirstChildElement();
	while (tileDefElement)
	{
		std::string elementName = tileDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "TileDefinition", Stringf("Root child element in %s was <%s>, must be <TileDefinition>!", filePath, elementName.c_str()));
		TileDefinition* newTileDef = new TileDefinition(tileDefElement);
		s_tileDefinitions.push_back(*newTileDef);
		tileDefElement = tileDefElement->NextSiblingElement();
	}
}
