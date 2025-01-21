#pragma once
#include <vector>
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/Ray.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/EventSystem.hpp"
class Player;
//class EventArgs;


class Map
{
public:
	Map(MapDefinition const& mapDef);
	~Map();
	void InitializeTileMapFromDef(MapDefinition const& mapDef);
	void InitializeTileMapFromImage(std::string const& imagePath);
	std::string const& GetTileTypeFromColor(Rgba8 const& pixelColor);
	void GenerateEachTile(int columnIndex, int rowIndex, std::string& thisTileType);
	void InitializeWorms(int wormsCount, int wormsLen, std::string wormsType);
	void ChangeUnreachTileToWall();
	void AddVertsForAllTiles();
	void InitializeAgentsFromDef(MapDefinition const& mapDef);
	Entity* InitializeEachAgent(EntityType entityType, EntityFaction entityFaction);
	
	void Update(float deltaSeconds);
	void UpdateMapArray();
	void Render() const;
	void RenderHeatMap() const;
	void RenderEntityUI() const;
	void RenderDebugDraw();
	
	std::string const& GetTileType(int coor_x, int coor_y);
	Tile* GetTile(int coor_x, int coor_y);
	int GetTileIndex(int coor_x, int coor_y);

	Entity* SpawnNewEntity(EntityType type, EntityFaction faction, Vec2 const& position, float orientationDegrees);
	Entity* CreateNewEntityOfType(EntityType type, EntityFaction faction);
	void AddEntityToMap(Entity& thisEntity, Vec2 position, float orientation);
	void RemoveEntityFromMap(Entity& thisEntity);
	void AddEntityToList(Entity& thisEntity, EntityList& list);
	void RemoveEntityFromList(Entity& thisEntity, EntityList& list);

	void DeleteGarbageEntity();

	bool IsBullet(Entity* entity);
	bool IsAgent(Entity* entity);
	bool IsObstacle(Entity* entity);
	bool IsGarbage(Entity* entity);

	IntVec2 GetTileCoordsFromPoint(Vec2 const& point);
	bool IsPointInSolid(Vec2 const& point); 
	bool IsTileSolid(IntVec2 const& coordinate); 
	bool IsTileSolid(int x, int y);
	bool IsTileWater(int x, int y);
	bool IsObstacleTile(int x, int y);
	void CheckEntityCollWithEntity();
	void CheckEntityCollWithSolidTiles();
	void PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius,bool canSwim);
	void CheckBulletWithSolidTiles();
	bool BulletVsEachTile(IntVec2& entityTileCoordsPos, IntVec2& nextTileCoordsPos, Entity* bullet);
	void CheckBulletWithAriesSheild();
	void CheckBulletWithEntities();
	RaycastResult2D RaycastVsTiles(Ray const& ray);
	RaycastResult2D FastRaycastVsTiles(Ray const& ray, bool treatWaterAsSolid = false, bool treatScorpioAsSolid = false);
	bool HasLineOfSight(Ray const& ray,Player*entity);
	void CheckPlayerToExitPoint();
	void AgentShooting(EntityType bulletType, EntityFaction faction,Vec2 const& tipPoint, float orientation,float explosionSize);

	void SpawnExplosion(Vec2 const& tipPoint, float explosionSize,SpriteAnimDefinition* animDef);

	int PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid = true, bool treatScorpioAsSolid = true);
	void PopulateSolidField(TileHeatMap& out_distanceField, float maxCost, bool treatWaterAsSolid = true, bool treatScorpioAsSolid = true);
	void GenerateEntityPathToGoal(TileHeatMap& refer_distanceField,std::vector<Vec2>& pathPoints,IntVec2 const& startPoint, IntVec2 const& goalPoint);
	Vec2 ImprovedPickNextWayPointPosition(TileHeatMap& heatmap, Vec2& curPos);

	static bool GenerateFindOutEffect(EventArgs& args);

	
public:
	std::vector<Tile>       m_tiles; 
	IntVec2                 m_dimensions; 
	std::vector<Vertex_PCU> m_tilemapVerts;
	std::vector<Vertex_PCU> m_heatmapVerts;

	EntityList  m_allEntities;
	EntityList	m_allBullets;
	EntityList	m_allObstacles;
	EntityList	m_entityListsByType[NUM_ENTITY_TYPES];
	EntityList	m_entityListsByFaction[NUM_FACTION_TYPES];
	EntityList	m_agentListByFaction[NUM_FACTION_TYPES];
	EntityList	m_bulletListByFaction[NUM_FACTION_TYPES];
	Player*		m_player=nullptr;

	IntVec2 m_exitCoords;
	IntVec2 m_startCoords;
	MapDefinition const& m_mapDef;
private:
	void CameraFollowPlayer();
	TileDefinition const& MatchTileDef(std::string const& tileName);
	
	bool IsCoordsLegal(int coordsX, int coordsY);
	int GetTileDefIndex(std::string const& typeName);

	bool m_isMapVertexDirty = false;
private:
	int m_viewTileNumX;
	int m_viewTileNumY;
	float m_tileSizeX;
	float m_tileSizeY;
	Vec2 m_viewLen;

	//Metadata management
	TileHeatMap* m_heatMapDistance=nullptr;
	TileHeatMap* m_heatMapSolidForAmph = nullptr;
	TileHeatMap* m_heatMapSolidForLand = nullptr;
};