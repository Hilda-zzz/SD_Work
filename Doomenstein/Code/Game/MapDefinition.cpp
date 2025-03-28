#include "MapDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

std::vector<MapDefinition> MapDefinition::s_mapDefinitions;

MapDefinition::MapDefinition(XmlElement const* mapDefElement)
{
	m_name = ParseXmlAttribute(mapDefElement, "name", m_name);
	m_mapImageName = ParseXmlAttribute(mapDefElement, "image", m_mapImageName);
	std::string shaderName = ParseXmlAttribute(mapDefElement, "shader", shaderName);
	m_shader = g_theRenderer->CreateShaderFromFile(shaderName.c_str(),VertexType::VERTEX_PCUTBN);
	std::string texName= ParseXmlAttribute(mapDefElement, "spriteSheetTexture", texName);
	m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(texName.c_str());
	m_spriteSheetCellCount = ParseXmlAttribute(mapDefElement, "spriteSheetCellCount", m_spriteSheetCellCount);
}

MapDefinition::~MapDefinition()
{

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
