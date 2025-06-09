#include "TileManager.hpp"

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
	LoadAllMaps();
}

void TileMapManager::LoadAllTilesets()
{
	LoadTileset("Data/GrassSpring.tsx");
}

Tileset* TileMapManager::LoadTileset(const std::string& tilesetPath)
{
	Tileset* newTileset = m_loader.LoadTileset(tilesetPath);
	if (!newTileset)
	{
		m_loadedTilesets[tilesetPath] = newTileset;
	}
	return newTileset;
}

void TileMapManager::LoadAllMaps()
{
	LoadMap("Data/Tiled/HouseMap.tmx");
}

TileMap* TileMapManager::LoadMap(const std::string& mapPath)
{
	TileMap* newMap=m_loader.LoadFromFile(mapPath);
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

TileMap* TileMapManager::GetCurrentMap()
{
	return m_curMap;
}

void TileMapManager::SetCurrentMap(const std::string& mapName)
{
}
