#include "InventoryItemDef.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

std::map<std::string, InventoryItemDef*> InventoryItemDef::s_itemDefinitions;

InventoryItemDef::InventoryItemDef(XmlElement const* itemDefElement)
{
	m_name = ParseXmlAttribute(itemDefElement, "name", std::string());
	GUARANTEE_OR_DIE(!m_name.empty(), "InventoryItemDef must have a 'name' attribute");

	std::string typeStr = ParseXmlAttribute(itemDefElement, "type", std::string());
	GUARANTEE_OR_DIE(!typeStr.empty(), "InventoryItemDef must have a 'type' attribute");

	if (typeStr == "ITEM_TYPE_TOOL")
	{
		m_itemType = ItemType::ITEM_TYPE_TOOL;
	}
	else if (typeStr == "ITEM_TYPE_CROP")
	{
		m_itemType = ItemType::ITEM_TYPE_CROP;
	}
	else if (typeStr == "ITEM_TYPE_MAT")
	{
		m_itemType = ItemType::ITEM_TYPE_MAT;
	}
	else if (typeStr == "ITEM_TYPE_FURNITURE")
	{
		m_itemType = ItemType::ITEM_TYPE_FURNITURE;
	}
	else
	{
		ERROR_AND_DIE(Stringf("Unknown item type: %s", typeStr.c_str()));
	}

	m_coinValue = ParseXmlAttribute(itemDefElement, "coinValue", 0);
	m_description = ParseXmlAttribute(itemDefElement, "description", std::string());

	XmlElement const* textureElement = itemDefElement->FirstChildElement("ItemTexture");
	if (textureElement)
	{
		std::string texturePath = ParseXmlAttribute(textureElement, "path", std::string());
		GUARANTEE_OR_DIE(!texturePath.empty(), "ItemTexture must have a 'path' attribute");
		m_iconTexture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	}

	XmlElement const* iconElement = itemDefElement->FirstChildElement("Icon");
	if (iconElement)
	{
		m_spriteLayout = ParseXmlAttribute(iconElement, "spriteLayout", IntVec2(16, 16));
		m_spriteGridPos = ParseXmlAttribute(iconElement, "gridPos", IntVec2(0, 0));

		if (m_iconTexture)
		{
			m_spriteSheet = new SpriteSheet(*m_iconTexture, m_spriteLayout);
		}
	}

	// Tools
	if (m_itemType == ItemType::ITEM_TYPE_TOOL)
	{
		XmlElement const* toolTypeElement = itemDefElement->FirstChildElement("ToolType");
		if (toolTypeElement)
		{
			std::string toolTypeStr = ParseXmlAttribute(toolTypeElement, "type", std::string());
			if (toolTypeStr == "AXE")
			{
				m_toolType = PlayerTools::AXE;
			}
			else if (toolTypeStr == "HOE")
			{
				m_toolType = PlayerTools::HOE;
			}
			else if (toolTypeStr == "PICKAXE")
			{
				m_toolType = PlayerTools::PICKAXE;
			}
			else if (toolTypeStr == "SHOVEL")
			{
				m_toolType = PlayerTools::SHOVEL;
			}
			else if (toolTypeStr == "SICKLE")
			{
				m_toolType = PlayerTools::SICKLE;
			}
			else if (toolTypeStr == "WATER")
			{
				m_toolType = PlayerTools::WATER;
			}
			else
			{
				ERROR_AND_DIE(Stringf("Unknown tool type: %s", toolTypeStr.c_str()));
			}
		}
	}
}

InventoryItemDef::~InventoryItemDef()
{
	if (m_spriteSheet)
	{
		delete m_spriteSheet;
		m_spriteSheet = nullptr;
	}
}

void InventoryItemDef::InitializeInventoryItemDefinitionFromFile()
{
	XmlDocument itemDefsXml;
	char const* filePath = "Data/Definitions/InventoryItemDefinitions.xml";
	XmlResult result = itemDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required item defs file \"%s\"", filePath));

	XmlElement* rootElement = itemDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Failed to find item definitions root element");

	XmlElement* itemDefElement = rootElement->FirstChildElement("ItemDefinition");
	while (itemDefElement)
	{
		std::string elementName = itemDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ItemDefinition", Stringf("Root child element in %s was <%s>, must be <ItemDefinition>!", filePath, elementName.c_str()));

		InventoryItemDef* newItemDef = new InventoryItemDef(itemDefElement);
		s_itemDefinitions[newItemDef->m_name] = newItemDef;

		itemDefElement = itemDefElement->NextSiblingElement("ItemDefinition");
	}
}

void InventoryItemDef::ShutdownInventoryItemDefinition()
{
	for (auto& pair : s_itemDefinitions)
	{
		delete pair.second;
		pair.second = nullptr;
	}
	s_itemDefinitions.clear();
}

InventoryItemDef* InventoryItemDef::GetItemDefFromName(const std::string& name)
{
	auto iter = s_itemDefinitions.find(name);
	if (iter != s_itemDefinitions.end())
	{
		return iter->second;
	}
	return nullptr;
}

AABB2 InventoryItemDef::GetIconUV() const
{
	int index = m_spriteSheet->GetSpriteIndexFromGridPos(m_spriteGridPos);
	return m_spriteSheet->GetSpriteUVs(index);
}
