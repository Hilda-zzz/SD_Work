#pragma once
#include <unordered_map>
#include <string>
#include <map>
#include "Game/TileMapLoader.hpp"
#include "DynamicContentGenerator.hpp"

class TileMap;
class Tileset;
struct TileProperty;

class TileMapManager 
{
private:
	// Singleton
	static TileMapManager* s_tileManagerInstance;
	TileMapManager() = default;    // constructor is private
	~TileMapManager() = default;
	TileMapManager(const TileMapManager&) = delete;  // delete copy constructor
	TileMapManager& operator=(const TileMapManager&) = delete;

public:
	static TileMapManager& GetInstance();
	static void DestroyInstance();

	static std::string ConvertTiledPathToGamePath(const std::string& tiledPath);
	static void AddGidToPropertyMaskForEachTileset(Tileset* tileset);
	static bool GetIsPropertyTrue(TileProperty const& property, std::string propertyName);

	void InitAllTilemapResources();
	void LoadAllTilesets();
	Tileset* LoadTileset(const std::string& tilesetPath);
	void LoadAllMaps();
	TileMap* LoadMap(const std::string& mapPath);
	TileMap* GetMap(const std::string& mapName);

	void UnloadAllMaps();
	void UnloadMap(const std::string& mapName);

	TileMap* GetCurrentMap();                       // maybe no need it 
	void SetCurrentMap(const std::string& mapName);

	Tileset const* FindTilesetByGid(uint32_t gid) const;

private:
	void LoadTilesetProperties(const Tileset& tileset);

public:
	std::unordered_map<std::string, TileMap*> m_loadedMaps;
	std::unordered_map<std::string, Tileset*> m_loadedTilesetsByName;
	std::map<uint32_t, Tileset*> m_loadedTilesetsByGID;
	std::unordered_map<uint32_t, uint32_t> m_gidToTilePropertyFlag;

	TileMap* m_curMap = nullptr;
	TileMapLoader m_loader;
	DynamicContentGenerator m_dynamicGenerator;
};