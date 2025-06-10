#include "TileManager.hpp"
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

void TileMapManager::InitAllTilemapResources()
{
	LoadAllTilesets();
	LoadAllMaps();
}

void TileMapManager::LoadAllTilesets()
{
	Tileset* newTileset=LoadTileset("Data/Tiled/GrassSpring.tsx");
}

Tileset* TileMapManager::LoadTileset(const std::string& tilesetPath)
{
	Tileset* newTileset = m_loader.LoadTilesetFromFile(tilesetPath);
	if (newTileset)
	{
		m_loadedTilesets[newTileset->GetName()] = newTileset;
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
	if (!newMap)
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
