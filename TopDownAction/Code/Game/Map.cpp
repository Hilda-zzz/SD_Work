#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

extern SpriteSheet* g_terrianSpriteSheet;
extern SpriteSheet* g_wallSpriteSheet;

Map::Map(Game* game, MapDefinition const& mapDef) :m_game(game),m_dimensions(mapDef.m_dimensions),m_mapDef(mapDef)
{
	m_viewTileNumX = g_gameConfigBlackboard.GetValue("viewTileNumX", 16);
	m_viewTileNumY = g_gameConfigBlackboard.GetValue("viewTileNumY", 8);
	m_tileSizeX = g_gameConfigBlackboard.GetValue("tileSizeX", 1.f);
	m_tileSizeY = g_gameConfigBlackboard.GetValue("tileSizeY", 1.f);

	if (m_mapDef.m_mapImageName != "")
	{
		InitializeFloorMapFromImage(m_mapDef.m_mapImageName, & m_tiles);
	}
	else
	{
		ERROR_AND_DIE("Map Constructor: m_mapDef.m_mapImageName is null. ");
	}

	if (m_mapDef.m_mapWallImageName != "")
	{
		InitializeWallMapFromImage(m_mapDef.m_mapWallImageName,&m_walls);
	}
	else
	{
		ERROR_AND_DIE("Map Constructor: m_mapDef.m_mapImageName is null. ");
	}
}

Map::~Map()
{
}

void Map::InitializeFloorMapFromImage(std::string const& imagePath, std::vector<Tile>* tiles)
{
	Image* mapImage = g_theRenderer->CreateImageFromFile(imagePath.c_str());
	m_dimensions = mapImage->GetDimensions();
	tiles->reserve(m_dimensions.x * m_dimensions.y);
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Rgba8 pixelColor=mapImage->GetTexelColor(IntVec2(columnIndex, rowIndex));
			std::string thisTileType=GetTileTypeNameByColor(pixelColor);
			if (thisTileType.c_str()!="")
			{
				GenerateEachTile(columnIndex, rowIndex, thisTileType, tiles);
			}
		}
	}
	delete mapImage;
	mapImage = nullptr;
	AddVertsForAllFloors();
}

void Map::InitializeWallMapFromImage(std::string const& imagePath, std::vector<Tile>* tiles)
{
	Image* mapImage = g_theRenderer->CreateImageFromFile(imagePath.c_str());
	tiles->reserve(m_dimensions.x * m_dimensions.y);
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Rgba8 pixelColor = mapImage->GetTexelColor(IntVec2(columnIndex, rowIndex));
			std::string thisTileType = GetWallTypeNameByColor(pixelColor);
			if (thisTileType!= "NONE")
			{
				GenerateEachWall(columnIndex, rowIndex, thisTileType, tiles);
			}
		}
	}
	delete mapImage;
	mapImage = nullptr;
	AddVertsForAllWalls();
}

void Map::GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType, std::vector<Tile>* tiles)
{
	Tile newTile = Tile(columnIndex, rowIndex, GetTileDefIndexByName(thisTileType));
	tiles->push_back(newTile);
}

void Map::GenerateEachWall(int columnIndex, int rowIndex, std::string& thisTileType, std::vector<Tile>* tiles)
{
	Tile newTile = Tile(columnIndex, rowIndex, GetWallDefIndexByName(thisTileType));
	tiles->push_back(newTile);
}

void Map::AddVertsForAllFloors()
{
	for (int i = 0; i < m_tiles.size(); i++)
	{
		Vec2 worldBL_ofThisTile = Vec2(m_tiles[i].m_tileCoordinates.x * m_tileSizeX, (float)m_tiles[i].m_tileCoordinates.y * m_tileSizeY);
		AABB2 aabb_thisTile = AABB2(worldBL_ofThisTile, worldBL_ofThisTile + Vec2(m_tileSizeX, m_tileSizeY));
		if (m_tiles[i].m_tileDefIndex != -1)
		{
			TileDefinition curDef = TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex];
			int spriteIndex = curDef.m_spriteCoords.x
				+ (curDef.m_spriteCoords.y * g_terrianSpriteSheet->m_gridLayout.x);
			AABB2 tileUV = g_terrianSpriteSheet->GetSpriteUVs(spriteIndex);
			AddVertsForAABB2D(m_tilemapVerts,
				aabb_thisTile,
				Rgba8::WHITE,
				tileUV.m_mins,
				tileUV.m_maxs);
		}
	}
}

void Map::AddVertsForAllWalls()
{
	for (int i = 0; i < m_walls.size(); i++)
	{
		Vec2 worldBL_ofThisTile = Vec2(m_walls[i].m_tileCoordinates.x * m_tileSizeX, (float)m_walls[i].m_tileCoordinates.y * m_tileSizeY);
		AABB2 aabb_thisTile = AABB2(worldBL_ofThisTile, worldBL_ofThisTile + Vec2(m_tileSizeX, m_tileSizeY));
		if (m_walls[i].m_tileDefIndex != -1)
		{
			TileDefinition curDef = TileDefinition::s_wallDefinitions[m_walls[i].m_tileDefIndex];
			int spriteIndex = curDef.m_spriteCoords.x
				+ (curDef.m_spriteCoords.y * g_terrianSpriteSheet->m_gridLayout.x);
			AABB2 tileUV = g_wallSpriteSheet->GetSpriteUVs(spriteIndex);
			AddVertsForAABB2D(m_wallVerts,
				aabb_thisTile,
				Rgba8::WHITE,
				tileUV.m_mins,
				tileUV.m_maxs);
		}

	}
}

void Map::Update(float deltaSeconds)
{
	
}



void Map::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	g_theRenderer->BindTexture(&g_terrianSpriteSheet->GetTexture());
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(m_tilemapVerts);
	g_theRenderer->BindTexture(&g_wallSpriteSheet->GetTexture());
	g_theRenderer->DrawVertexArray(m_wallVerts);
}

std::string const& Map::GetTileTypeNameByCoords(int coor_x, int coor_y)
{
	int index = coor_x + coor_y * m_dimensions.x;
	return TileDefinition::s_tileDefinitions[m_tiles[index].m_tileDefIndex].m_name;
}

std::string const& Map::GetTileTypeNameByColor(Rgba8 const& pixelColor)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (pixelColor == TileDefinition::s_tileDefinitions[i].m_mapImagePixelColor)
		{
			return TileDefinition::s_tileDefinitions[i].m_name;
		}
	}
	std::string result = "";
	return result;
	//ERROR_AND_DIE("image's color is not appear in tile defs");
}

std::string const& Map::GetWallTypeNameByColor(Rgba8 const& pixelColor)
{
	for (int i = 0; i < (int)TileDefinition::s_wallDefinitions.size(); i++)
	{
		if (pixelColor == TileDefinition::s_wallDefinitions[i].m_mapImagePixelColor)
		{
			return TileDefinition::s_wallDefinitions[i].m_name;
		}
	}
	return TileDefinition::s_wallDefinitions[0].m_name;
}

int Map::GetTileDefIndexByName(std::string const& typeName)
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (typeName == TileDefinition::s_tileDefinitions[i].m_name)
		{
			return i;
		}
	}
	return -1;
}

int Map::GetWallDefIndexByName(std::string const& typeName)
{
	for (int i = 0; i < (int)TileDefinition::s_wallDefinitions.size(); i++)
	{
		if (typeName == TileDefinition::s_wallDefinitions[i].m_name)
		{
			return i;
		}
	}
	return -1;
}

int Map::GetTileIndexByCoords(int coor_x, int coor_y)
{
	return coor_x + coor_y * m_dimensions.x;
}

Tile* Map::GetTileByCoords(int coor_x, int coor_y)
{
	int index = coor_x + coor_y * m_dimensions.x;
	return &m_tiles[index];
}

IntVec2 Map::GetTileCoordsFromPoint(Vec2 const& point)
{
	return IntVec2(static_cast<int>(floorf(point.x)), static_cast<int>(floorf(point.y)));
}

TileDefinition const& Map::GetTileDefByCoords(int coor_x, int coor_y)
{
	int tileIndex = coor_x + coor_y * m_dimensions.x;
	return TileDefinition::s_tileDefinitions[m_tiles[tileIndex].m_tileDefIndex];
}

bool Map::IsPointInSolid(Vec2 const& point)
{
	IntVec2 coords=GetTileCoordsFromPoint(point);
	bool isSolid=GetTileDefByCoords(coords.x, coords.y).m_isSolid;
	return isSolid;
}

bool Map::IsTileSolid(int x, int y)
{
	return GetTileDefByCoords(x, y).m_isSolid;
}

bool Map::IsCoordsLegal(int coordsX, int coordsY)
{
	if (coordsX >= 0 && coordsX <= m_dimensions.x - 1 && coordsY >= 0 && coordsY <= m_dimensions.y - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

RaycastResult2D Map::FastRaycastVsTiles(Ray const& ray,bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	RaycastResult2D result = RaycastResult2D(ray);
	IntVec2 startTileCoords=GetTileCoordsFromPoint(ray.m_rayStartPos);

	if (GetTileDefByCoords(startTileCoords.x, startTileCoords.y).m_isSolid)
	{
		result.m_didImpact = true;
		result.m_impactPos = ray.m_rayStartPos;
		result.m_impactDist =0.f;
		result.m_impactNormal = Vec2::ZERO;
		return result;
	}
	float fwdDistPerXCrossing =m_tileSizeX/ abs(ray.m_rayFwdNormal.x);
	int tileStepDirectionX = (ray.m_rayFwdNormal.x < 0) ? -1 : 1;
	float xAtFirstXCrossing = startTileCoords.x + (tileStepDirectionX + 1.f) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - ray.m_rayStartPos.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = m_tileSizeY / abs(ray.m_rayFwdNormal.y);
	int tileStepDirectionY = (ray.m_rayFwdNormal.y < 0) ? -1 : 1;
	float yAtFirstYCrossing = startTileCoords.y + (tileStepDirectionY + 1.f) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - ray.m_rayStartPos.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	while (true)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) //touch x sooner
		{
			if (fwdDistAtNextXCrossing > ray.m_rayMaxLength)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.x += tileStepDirectionX;
			if (IsTileSolid(startTileCoords.x,startTileCoords.y))
			{
				result.m_didImpact = true;
				result.m_impactPos = ray.m_rayStartPos+ray.m_rayFwdNormal*fwdDistAtNextXCrossing;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactNormal = Vec2((float)-(float)tileStepDirectionX, 0.f);
				return result;
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else
		{
			if (fwdDistAtNextYCrossing > ray.m_rayMaxLength)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.y += tileStepDirectionY;
			if (IsTileSolid(startTileCoords.x, startTileCoords.y))
			{
				result.m_didImpact = true;
				result.m_impactPos = ray.m_rayStartPos + ray.m_rayFwdNormal * fwdDistAtNextYCrossing;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactNormal = Vec2(0.f, (float)-tileStepDirectionY);
				return result;
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}










