#include "BlockDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

extern Renderer* g_theRenderer;

std::vector<BlockDefinition> BlockDefinition::s_blockDefs;
std::unordered_map<std::string, uint8_t> BlockDefinition::s_nameToIndexMap;
SpriteSheet* BlockDefinition::s_blockSheet;

BlockDefinition::BlockDefinition(XmlElement const* blockDefElement)
{
	m_name = ParseXmlAttribute(blockDefElement, "name", std::string());
	GUARANTEE_OR_DIE(!m_name.empty(), "BlockDefinition must have a 'name' attribute");

	m_isVisible = ParseXmlAttribute(blockDefElement, "isVisible", false);
	m_isSolid = ParseXmlAttribute(blockDefElement, "isSolid", false);
	m_isOpaque = ParseXmlAttribute(blockDefElement, "isOpaque", false); 

	m_topSpriteCoords = ParseXmlAttribute(blockDefElement, "topSpriteCoords", IntVec2(0, 0));
	m_bottomSpriteCoords = ParseXmlAttribute(blockDefElement, "bottomSpriteCoords", IntVec2(0, 0));
	m_sideSpriteCoords = ParseXmlAttribute(blockDefElement, "sideSpriteCoords", IntVec2(0, 0));

	int topSpriteIndex = s_blockSheet->GetSpriteIndexFromGridPos(m_topSpriteCoords);
	m_topUVs = s_blockSheet->GetSpriteUVs(topSpriteIndex);

	int bottomSpriteIndex = s_blockSheet->GetSpriteIndexFromGridPos(m_bottomSpriteCoords);
	m_bottomUVs = s_blockSheet->GetSpriteUVs(bottomSpriteIndex);

	int sideSpriteIndex = s_blockSheet->GetSpriteIndexFromGridPos(m_sideSpriteCoords);
	m_sideUVs = s_blockSheet->GetSpriteUVs(sideSpriteIndex);

	// dealing with invisible block ?

}

BlockDefinition::~BlockDefinition()
{

}

void BlockDefinition::InitializeBlockDefinitionsFromFile()
{
	Texture* blockTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BlockSpriteSheet_128px.png");
	s_blockSheet = new SpriteSheet(*blockTexture, IntVec2(8, 8));

	XmlDocument blockDefsXml;
	char const* filePath = "Data/Definitions/BlockSpriteSheet_BlockDefinitions.xml";
	XmlResult result = blockDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required BlockDefinitions file \"%s\"", filePath));

	XmlElement* rootElement = blockDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Failed to find BlockDefinitions root element");

	XmlElement* blockDefElement = rootElement->FirstChildElement("BlockDefinition");
	while (blockDefElement) {
		std::string elementName = blockDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "BlockDefinition",
			Stringf("Root child element in %s was <%s>, must be <BlockDefinition>!",
				filePath, elementName.c_str()));

		BlockDefinition newBlockDef = BlockDefinition(blockDefElement);
		s_blockDefs.push_back(newBlockDef);
		s_nameToIndexMap[newBlockDef.m_name] = s_blockDefs.size()-1;

		blockDefElement = blockDefElement->NextSiblingElement("BlockDefinition");
	}
}

void BlockDefinition::ShutdownBlockDefinitions()
{
	delete s_blockSheet;
	s_blockSheet = nullptr;
}
