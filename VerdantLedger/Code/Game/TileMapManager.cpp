#include "TileMapManager.hpp"
#include "Game/Tileset.hpp"

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
	delete s_tileManagerInstance;
	s_tileManagerInstance = nullptr;
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
}

void TileMapManager::AddGidToPropertyMaskForEachTileset(Tileset* tileset)
{
	for (int i = 0; i < tileset->m_properties.size(); i++)
	{
		uint32_t gid = tileset->m_properties[i].m_localID + tileset->GetFirstGid();
		uint32_t flags = 0;

		if (GetIsPropertyTrue(tileset->m_properties[i], "IsSolid"))
		{
			flags |= static_cast<uint32_t>(TileFlags::SOLID);
		}
		if (GetIsPropertyTrue(tileset->m_properties[i], "IsFarmable"))
		{
			flags |= static_cast<uint32_t>(TileFlags::FARMABLE);
		}
		if (GetIsPropertyTrue(tileset->m_properties[i], "IsWaater"))
		{
			flags |= static_cast<uint32_t>(TileFlags::WATER);
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
	LoadAllMaps();
}

void TileMapManager::LoadAllTilesets()
{
	Tileset* newTileset=LoadTileset("Data/Tiled/GrassSpring.tsx");
	Tileset* newTileset2 = LoadTileset("Data/Tiled/TileMark.tsx");
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
	TileMap* newMap=LoadMap("Data/Tiled/HouseMap.tmx");
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
	return nullptr;
}

void TileMapManager::UnloadAllMaps()
{
}

void TileMapManager::UnloadMap(const std::string& mapName)
{
}

TileMap* TileMapManager::GetCurrentMap()
{
	return m_curMap;
}

void TileMapManager::SetCurrentMap(const std::string& mapName)
{
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
}
