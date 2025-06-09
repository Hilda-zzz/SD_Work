#pragma once
#include <unordered_map>
#include <string>

class TileMap;

class TileMapManager 
{
private:
	static std::unordered_map<std::string, TileMap*> s_loadedMaps;
	static TileMap* s_currentMap;

public:
	static TileMap* LoadMap(const std::string& mapPath);
	static TileMap* GetMap(const std::string& mapName);
	static TileMap* GetCurrentMap() { return s_currentMap; }
	static void SetCurrentMap(const std::string& mapName);
	static void UnloadMap(const std::string& mapName);
	static void UnloadAllMaps();
};