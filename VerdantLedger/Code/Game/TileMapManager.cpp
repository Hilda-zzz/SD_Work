#include "TileMapManager.hpp"
#include "Game/Tileset.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "TileTypesInGame.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "RuledTileset.hpp"
#include "Game/TileMap.hpp"

TileMapManager* TileMapManager::s_tileManagerInstance = nullptr;

TileMapManager& TileMapManager::GetInstance()
{
	if (!s_tileManagerInstance)
	{
		s_tileManagerInstance = new TileMapManager();
	}
	return *s_tileManagerInstance;
}

void TileMapManager::DestroyInstance()
{
	if (s_tileManagerInstance)
	{
		for (auto& pair : s_tileManagerInstance->m_loadedMaps)
		{
			delete pair.second;
		}
		s_tileManagerInstance->m_loadedMaps.clear();

		for (auto& pair : s_tileManagerInstance->m_loadedTilesetsByName)
		{
			delete pair.second;
		}
		s_tileManagerInstance->m_loadedTilesetsByName.clear();

		for (auto& pair : s_tileManagerInstance->m_loadedRuledTilesetsByName)
		{
			delete pair.second;
		}
		s_tileManagerInstance->m_loadedRuledTilesetsByName.clear();

		delete s_tileManagerInstance;
		s_tileManagerInstance = nullptr;
	}
}

std::string TileMapManager::ConvertTiledPathToGamePath(const std::string& tiledPath)
{
	if (tiledPath.substr(0, 3) == "../")
	{
		return "Data/" + tiledPath.substr(3);
	}

	if (tiledPath.substr(0, 5) == "Data/")
	{
		return tiledPath;
	}
	return "";
}

void TileMapManager::AddGidToPropertyMaskForEachTileset(Tileset* tileset)
{
	for (int i = 0; i < (int)tileset->m_properties.size(); i++)
	{
		uint32_t gid = tileset->m_properties[i].m_localID + tileset->GetFirstGid();
		uint32_t flags = 0;

		if (GetIsPropertyTrue(tileset->m_properties[i], "IsSolid"))
		{
			flags |= static_cast<uint32_t>(TerrainType::SOLID);
		}
		if (GetIsPropertyTrue(tileset->m_properties[i], "IsFarmable"))
		{
			flags |= static_cast<uint32_t>(TerrainType::FARMABLE);
		}
		if (GetIsPropertyTrue(tileset->m_properties[i], "IsWaater"))
		{
			flags |= static_cast<uint32_t>(TerrainType::WATER);
		}

		s_tileManagerInstance->m_gidToTilePropertyFlag[gid] = flags;
	}
}

bool TileMapManager::GetIsPropertyTrue(TileProperty const& property, std::string propertyName)
{
	if (property.m_propertyName == propertyName&&property.m_value==true)
	{
		return true;
	}
	return false;
}

void TileMapManager::InitAllTilemapResources()
{
	LoadAllTilesets();
	LoadAllRuedTilesets();
	LoadAllMaps();
}

void TileMapManager::LoadAllTilesets()
{
	LoadTileset("Data/Tiled/GrassSpring.tsx");
	LoadTileset("Data/Tiled/TileMark.tsx");
	LoadTileset("Data/Tiled/Road.tsx");
	LoadTileset("Data/Tiled/OutsideTileset.tsx");
	LoadTileset("Data/Tiled/InnerHouse.tsx");
}

void TileMapManager::LoadAllRuedTilesets()
{
	XmlDocument ruledTileset;
	XmlResult result = ruledTileset.LoadFile("Data/Definitions/RuledTilesets.xml");
	std::string errorMsg = Stringf("Failed to open required ruled tileset defs file \"%s\"", "Data/Definitions/RuledTilesets.xml");
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, errorMsg.c_str());

	XmlElement* rootElement = ruledTileset.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Faile to find root element");

	for (XmlElement* tilesetDefElement = rootElement->FirstChildElement("RuledTilesetDefinition");
		tilesetDefElement != nullptr;
		tilesetDefElement = tilesetDefElement->NextSiblingElement("RuledTilesetDefinition"))
	{
		RuledTileset* newRuledTileset = new RuledTileset(tilesetDefElement);

		std::string tilesetName = newRuledTileset->GetName();

		if (m_loadedRuledTilesetsByName.find(tilesetName) != m_loadedRuledTilesetsByName.end())
		{
			std::string warningMsg = Stringf("Duplicate ruled tileset name \"%s\" found, overwriting previous definition", tilesetName.c_str());
			DebuggerPrintf(warningMsg.c_str());
			delete m_loadedRuledTilesetsByName[tilesetName]; 
		}

		m_loadedRuledTilesetsByName[tilesetName] = newRuledTileset;
	}
}

Tileset* TileMapManager::LoadTileset(const std::string& tilesetPath)
{
	Tileset* newTileset = m_loader.LoadTilesetFromFile(tilesetPath);
	if (newTileset)
	{
		m_loadedTilesetsByName[newTileset->GetName()] = newTileset;
	}
	return newTileset;
}

void TileMapManager::LoadAllMaps()
{
	// change to load and generateAllMaps
// 	LoadMap("Data/Tiled/HouseMap.tmx");
// 	m_dynamicGenerator.GenerateAllDynamicContentForTheMap(m_loadedMaps["Data/Tiled/HouseMap.tmx"]);

	LoadMap("Data/Tiled/MyFarmMap.tmx");
	m_dynamicGenerator.GenerateAllDynamicContentForTheMap(m_loadedMaps["Data/Tiled/MyFarmMap.tmx"]);
// save dynamic data for each tile grid
// 	IntVec2 curGridPos = chunk.GetGridPos(i);
// 	uint64_t tilePosKey = GetTileKey(curGridPos);
// 	DynamicTileData curDynamicTileData;
// 	float possiblity = m_rng.RollRandomFloatZeroToOne();
// 	if (possiblity > 0.4f)
// 	{
// 		curDynamicTileData.m_obstacleType == ObstacleType::ROCK;
// 	}
// 	m_dynamicTiles[tilePosKey] = curDynamicTileData;
	LoadMap("Data/Tiled/InnerHouse.tmx");
}

TileMap* TileMapManager::LoadMap(const std::string& mapPath)
{
	TileMap* newMap=m_loader.LoadTileMapFromFile(mapPath);
	if (newMap)
	{
		m_loadedMaps[mapPath] = newMap;
	}
	return newMap;
}

TileMap* TileMapManager::GetMap(const std::string& mapName)
{
	UNUSED(mapName);
	return nullptr;
}

void TileMapManager::UnloadAllMaps()
{
}

void TileMapManager::UnloadMap(const std::string& mapName)
{
	UNUSED(mapName);
}

TileMap* TileMapManager::GetCurrentMap()
{
	return m_curMap;
}

void TileMapManager::SetCurrentMap(const std::string& mapName)
{
	UNUSED(mapName);
}

Tileset const* TileMapManager::FindTilesetByGid(uint32_t gid) const
{
	// first tileset which bigger than gid
	auto it = s_tileManagerInstance->m_loadedTilesetsByGID.upper_bound(gid); 
	if (it != s_tileManagerInstance->m_loadedTilesetsByGID.begin())
	{
		--it;
		Tileset const* tileset = it->second;
		if (tileset->ContainsGid(gid)) {
			return tileset;
		}
	}
	return nullptr;
}

void TileMapManager::LoadTilesetProperties(const Tileset& tileset)
{
	UNUSED(tileset);
}
