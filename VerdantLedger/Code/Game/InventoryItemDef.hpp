#pragma once
#include <string>
#include <map>
#include "Engine/Core/XmlUtils.hpp"
#include "Player.hpp"

class Texture;
class SpriteSheet;

enum class ItemType
{
	ITEM_TYPE_NONE,
	ITEM_TYPE_TOOL,
	ITEM_TYPE_CROP,
	ITEM_TYPE_MAT,
	ITEM_TYPE_FURNITURE
};

class InventoryItemDef
{
public:
	InventoryItemDef(XmlElement const* itemDefElement);
	~InventoryItemDef();

	static void InitializeInventoryItemDefinitionFromFile();
	static void ShutdownInventoryItemDefinition();
	static InventoryItemDef* GetItemDefFromName(const std::string& name);

	AABB2 GetIconUV() const;

public:
	static std::map<std::string, InventoryItemDef*> s_itemDefinitions;

	std::string m_name = "";
	std::string m_description = "";
	ItemType m_itemType;
	PlayerTools m_toolType = PlayerTools::NONE;

	Texture* m_iconTexture = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_spriteGridPos = IntVec2(0, 0);
	IntVec2 m_spriteLayout = IntVec2(16, 16);
	
	int m_coinValue = 0;
};