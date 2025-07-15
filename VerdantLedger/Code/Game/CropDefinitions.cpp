#include "CropDefinitions.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "InventoryItemDef.hpp"

extern Renderer* g_theRenderer;
std::map<std::string, CropDefinitions*> CropDefinitions::s_cropDefinitions;

CropDefinitions::CropDefinitions(XmlElement const* cropDefElement)
{
	m_name = ParseXmlAttribute(cropDefElement, "name", std::string());
	GUARANTEE_OR_DIE(!m_name.empty(), "CropDefinition must have a 'name' attribute");


	m_description = ParseXmlAttribute(cropDefElement, "description", std::string());
	m_matureDay = ParseXmlAttribute(cropDefElement, "matureDay", 1);
	m_spriteStateNumCount = ParseXmlAttribute(cropDefElement, "spriteStateNumCount", 6);

	XmlElement const* textureElement = cropDefElement->FirstChildElement("CropTexture");
	if (textureElement) {
		std::string texturePath = ParseXmlAttribute(textureElement, "path", std::string());
		GUARANTEE_OR_DIE(!texturePath.empty(), "CropTexture must have a 'path' attribute");
		m_cropTexture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	}

	XmlElement const* spriteInfoElement = cropDefElement->FirstChildElement("SpriteInfo");
	if (spriteInfoElement) {
		m_spriteLayout = ParseXmlAttribute(spriteInfoElement, "spriteLayout", IntVec2(16, 16));
		m_spriteStartGridPos = ParseXmlAttribute(spriteInfoElement, "spriteStartGridPos", IntVec2(0, 0));
		if (m_cropTexture) {
			m_spriteSheet = new SpriteSheet(*m_cropTexture, m_spriteLayout);
		}
	}

	XmlElement const* seedItemElement = cropDefElement->FirstChildElement("SeedItem");
	if (seedItemElement) {
		std::string seedItemName = ParseXmlAttribute(seedItemElement, "name", std::string());
		if (!seedItemName.empty()) {
			m_seedItemDef = InventoryItemDef::GetItemDefFromName(seedItemName);
			if (!m_seedItemDef) {
				DebuggerPrintf("Warning: Seed item '%s' not found for crop '%s'\n",
					seedItemName.c_str(), m_name.c_str());
			}
		}
	}

	XmlElement const* harvestItemElement = cropDefElement->FirstChildElement("HarvestItem");
	if (harvestItemElement) {
		std::string harvestItemName = ParseXmlAttribute(harvestItemElement, "name", std::string());
		if (!harvestItemName.empty()) {
			m_harvestItemDef = InventoryItemDef::GetItemDefFromName(harvestItemName);
			if (!m_harvestItemDef) {
				DebuggerPrintf("Warning: Harvest item '%s' not found for crop '%s'\n",
					harvestItemName.c_str(), m_name.c_str());
			}
		}
	}
}

CropDefinitions::~CropDefinitions()
{
	if (m_spriteSheet)
	{
		delete m_spriteSheet;
		m_spriteSheet = nullptr;
	}
}

void CropDefinitions::InitializeCropDefinitionsFromFile()
{
	XmlDocument cropDefsXml;
	char const* filePath = "Data/Definitions/CropDefinitions.xml";
	XmlResult result = cropDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required crop defs file \"%s\"", filePath));

	XmlElement* rootElement = cropDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Failed to find crop definitions root element");

	XmlElement* cropDefElement = rootElement->FirstChildElement("CropDefinition");
	while (cropDefElement) {
		std::string elementName = cropDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "CropDefinition",
			Stringf("Root child element in %s was <%s>, must be <CropDefinition>!",
				filePath, elementName.c_str()));

		CropDefinitions* newCropDef = new CropDefinitions(cropDefElement);
		s_cropDefinitions[newCropDef->m_name] = newCropDef;

		cropDefElement = cropDefElement->NextSiblingElement("CropDefinition");
	}

	DebuggerPrintf("Loaded %d crop definitions\n", (int)s_cropDefinitions.size());
}

void CropDefinitions::ShutdownCropDefinitions()
{
	for (auto& pair : s_cropDefinitions)
	{
		delete pair.second;
		pair.second = nullptr;
	}
	s_cropDefinitions.clear();
}
