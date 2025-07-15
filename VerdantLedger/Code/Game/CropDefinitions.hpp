#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include <map>

class SpriteSheet;
class Texture;
class InventoryItemDef;

class CropDefinitions
{
public:
	CropDefinitions(XmlElement const* cropDefElement);
	~CropDefinitions();
	static void InitializeCropDefinitionsFromFile();
	static void ShutdownCropDefinitions();

	static std::map<std::string, CropDefinitions*> s_cropDefinitions;
	std::string m_name = "";
	std::string m_description = "";

	Texture* m_cropTexture = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_spriteLayout = IntVec2(16, 16);

	IntVec2 m_spriteStartGridPos = IntVec2(0, 0);
	int m_spriteStateNumCount = 0;

	int m_matureDay = 0;

	InventoryItemDef* m_seedItemDef = nullptr;       // Find by name_Seed
	InventoryItemDef* m_harvestItemDef = nullptr;    // Find by name_Harvest
};