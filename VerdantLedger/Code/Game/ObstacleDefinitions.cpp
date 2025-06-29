#include "ObstacleDefinitions.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


extern Renderer* g_theRenderer;

std::map<ObstacleType,ObstacleDefinition*> ObstacleDefinition::s_obstacleDefinitions;
Texture* ObstacleDefinition::s_obstacleTexture = nullptr;
//SpriteSheet* ObstacleDefinition::s_obstacleSpriteSheet = nullptr;

ObstacleDefinition::ObstacleDefinition(XmlElement const* obstacleDefElement)
{
	std::string typeStr = ParseXmlAttribute(obstacleDefElement, "type", std::string());
	GUARANTEE_OR_DIE(!typeStr.empty(), "ObstacleDefinition must have a 'type' attribute");

	if (typeStr == "ROCK")
	{
		m_obstacleType = ObstacleType::ROCK;
	}
	else if (typeStr == "LOG")
	{
		m_obstacleType = ObstacleType::LOG;
	}
	else if (typeStr == "WEED")
	{
		m_obstacleType = ObstacleType::WEED;
	}
	else if (typeStr == "TREE")
	{
		m_obstacleType = ObstacleType::TREE;
	}
	else
	{
		ERROR_AND_DIE(Stringf("Unknown obstacle type: %s", typeStr.c_str()));
	}

	m_maxDurability = ParseXmlAttribute(obstacleDefElement, "maxDurability", 0);

	XmlElement const* spriteVariantElement = obstacleDefElement->FirstChildElement("SpriteVariants");
	if (spriteVariantElement)
	{
		IntVec2 curSpriteSheetLayout= ParseXmlAttribute(spriteVariantElement, "spriteLayout",IntVec2(20,20));
		m_spriteSheet = new SpriteSheet(*s_obstacleTexture, curSpriteSheetLayout);
		for (XmlElement const* spriteElement = spriteVariantElement->FirstChildElement("SpriteGridPos");
			spriteElement != nullptr;
			spriteElement = spriteElement->NextSiblingElement("SpriteGridPos"))
		{
			IntVec2 gridPos= ParseXmlAttribute(spriteElement, "gridPos",IntVec2(0,0));
			m_spriteGridPos.push_back(gridPos);
		}
	}
}

ObstacleDefinition::~ObstacleDefinition()
{
 	if (m_spriteSheet)
 	{
 		delete m_spriteSheet;
 		m_spriteSheet = nullptr;
 	}
}

void ObstacleDefinition::InitializeObstacleDefinitionFromFile()
{
	XmlDocument obstacleDefsXml;
	char const* filePath = "Data/Definitions/ObstacleDefinitions.xml";
	XmlResult result = obstacleDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required obstacle defs file \"%s\"", filePath));

	XmlElement* rootElement = obstacleDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find obstacle root element");

	XmlElement* obstacleTextureElement = rootElement->FirstChildElement();
	std::string texturePath = ParseXmlAttribute(obstacleTextureElement, "path", "none");
	s_obstacleTexture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	//s_obstacleSpriteSheet = new SpriteSheet(*s_obstacleTexture, IntVec2(20, 20));

	XmlElement* obstacleDefElement = obstacleTextureElement->NextSiblingElement("ObstacleDefinition");
	while (obstacleDefElement)
	{
		std::string elementName = obstacleDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ObstacleDefinition", Stringf("Root child element in %s was <%s>, must be <ObstacleDefinition>!", filePath, elementName.c_str()));
		ObstacleDefinition* newObstacleDef = new ObstacleDefinition(obstacleDefElement);
		s_obstacleDefinitions[newObstacleDef->m_obstacleType]=newObstacleDef;
		obstacleDefElement = obstacleDefElement->NextSiblingElement();
	}
}

void ObstacleDefinition::ShutdownObstacleDefinition()
{
	for (auto& pair : ObstacleDefinition::s_obstacleDefinitions)
	{
		delete pair.second;  
	}
	ObstacleDefinition::s_obstacleDefinitions.clear();

// 	delete s_obstacleSpriteSheet;
// 	s_obstacleSpriteSheet = nullptr;
}

// ObstacleDefinition* ObstacleDefinition::GetObstacleDefFromType(ObstacleType type)
// {
// 	for (ObstacleDefinition* obstacleDef : s_obstacleDefinitions)
// 	{
// 		if (obstacleDef && obstacleDef->m_obstacleType == type)
// 		{
// 			return obstacleDef;
// 		}
// 	}
// 	return nullptr;
// }
