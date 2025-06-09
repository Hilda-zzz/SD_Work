#pragma once
#include <unordered_map>
#include <string>
#include "Game/TileMapLoader.hpp"

class TileMap;

class TileMapManager 
{
private:
	// Singleton
	static TileMapManager* s_tileManagerInstance;
	TileMapManager() = default;    // constructor is private
	~TileMapManager() = default;
	TileMapManager(const TileMapManager&) = delete;  // delete copy constructor
	TileMapManager& operator=(const TileMapManager&) = delete;

	std::unordered_map<std::string, TileMap*> m_loadedMaps;
	std::unordered_map<std::string, Tileset*> m_loadedTilesets;

	TileMap* m_curMap=nullptr;
	TileMapLoader m_loader;

public:
	static TileMapManager& GetInstance();
	static void DestroyInstance();

	void InitAllTilemapResources();
	void LoadAllTilesets();
	Tileset* LoadTileset(const std::string& tilesetPath);
	void LoadAllMaps();
	TileMap* LoadMap(const std::string& mapPath);
	TileMap* GetMap(const std::string& mapName);

	void UnloadAllMaps();
	void UnloadMap(const std::string& mapName);

	TileMap* GetCurrentMap();                       //maybe no need it 
	void SetCurrentMap(const std::string& mapName);
};