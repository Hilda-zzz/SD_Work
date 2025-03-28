#pragma once
#include <vector>
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"

class Player;
class Game;

class Map
{
public:
	Map(Game* game, MapDefinition const& mapDef);
	~Map();
	void InitializeTileMapFromImage(std::string const& imagePath,std::vector<Tile>* tiles);
	void GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType, std::vector<Tile>* tiles);
	void AddVertsForAllTiles();
	void AddVertsForAllWalls();


	void Update(float deltaSeconds);
	void Render() const;

	std::string const&	GetTileTypeNameByCoords(int coor_x, int coor_y);
	std::string const&	GetTileTypeNameByColor(Rgba8 const& pixelColor);
	int					GetTileDefIndexByName(std::string const& typeName);
	int					GetTileIndexByCoords(int coor_x, int coor_y);
	Tile*				GetTileByCoords(int coor_x, int coor_y);
	IntVec2				GetTileCoordsFromPoint(Vec2 const& point);
	TileDefinition const& GetTileDefByCoords(int coor_x, int coor_y);

	bool IsPointInSolid(Vec2 const& point); 
	bool IsTileSolid(int coor_x, int coor_y);
	bool IsCoordsLegal(int coordsX, int coordsY);
	
	RaycastResult2D FastRaycastVsTiles(Ray const& ray, bool treatWaterAsSolid = false, bool treatScorpioAsSolid = false);

private:
	
	

public:
	MapDefinition const&	m_mapDef;
	IntVec2                 m_dimensions;
	std::vector<Tile>       m_tiles; 
	std::vector<Vertex_PCU> m_tilemapVerts;
	std::vector<Tile>       m_walls;
	std::vector<Vertex_PCU> m_wallVerts;

	Game*					m_game = nullptr;
	Player*					m_player=nullptr;

private:
	int		m_viewTileNumX;
	int		m_viewTileNumY;
	float	m_tileSizeX;
	float   m_tileSizeY;
};