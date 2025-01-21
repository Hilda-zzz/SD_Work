#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
class Map;
class Entity;
class TileHeatMap;
struct RaycastResult2D;
enum EntityFaction
{
    FACTION_UNSURE = -1,
    FACTION_GOOD, 
    FACTION_NEUTRAL, 
    FACTION_EVIL,
    NUM_FACTION_TYPES
};

enum EntityType
{
    ENTITY_TYPE_UNSURE=-1,
    ENTITY_TYPE_NEU_RUBBLE,
    ENTITY_TYPE_GOOD_PLAYER, 
    ENTITY_TYPE_EVIL_SCORPIO, 
    ENTITY_TYPE_EVIL_LEO, 
    ENTITY_TYPE_EVIL_ARIES,
    ENTITY_TYPE_EVIL_CAPRICORN,
    ENTITY_TYPE_GOOD_BULLET, 
    ENTITY_TYPE_GOOD_FLAMEBULLET,
    ENTITY_TYPE_EVIL_BULLET, 
    ENTITY_TYPE_GOOD_BOLT,
    ENTITY_TYPE_EVIL_BOLT,
    ENTITY_TYPE_EVIL_GDMISSILE,
    ENTITY_TYPE_NEU_EXPLOSION,
    ENTITY_TYPE_NEU_TREE,
    NUM_ENTITY_TYPES
};

typedef std::vector<Entity*> EntityList;

class Entity 
{
    friend Map;
protected:
    Entity(float x, float y, Map* map);
    Entity(EntityType type, EntityFaction faction);
    virtual ~Entity();
    virtual void Update(float deltaTime) = 0;
    virtual void Render() const = 0;
    virtual void Die() = 0;
    virtual void DrawDebugMode() const;

    Vec2 GetForwardNormal() const;
    bool IsDead() const;
    bool IsGarbage() const;
    void GetHurt(float hurtPoint);
    void RenderHeatMap() const;
    void PathFindingToPlayer(bool isTargetVisible, Entity* player, int& pathState, Vec2& lastTargetPos, Vec2& nextWayPointPosition);
    void ImprovedPathFindingToPlayer(bool isTargetVisible, Entity* player, int& pathState, Vec2& lastTargetPos, Vec2& nextWayPointPosition);
    bool IsPassToNextPos();

    bool IsEdgeRayPassThrough(RaycastResult2D& resultVsSolidTiles);

    Vec2 PickNextWaypointPosition();
    void RenderUI();

    bool CheckOverlapVsRubble();
public:
    Vec2        m_position;
    EntityType m_type;
    EntityFaction m_faction;
    bool        m_isDead = false;
protected:
    Map*        m_map;
    Vec2        m_velocity;
    float       m_orientationDegrees;
    float       m_angularVelocity;
    float       m_physicsRadius;
    float       m_cosmeticRadius;
    std::vector<Vertex_PCU> m_heatmapVerts;
    
    bool        m_isGarbage=false;
    float       m_health;
    float       m_totalHealth;

    bool m_isPushedByEntities = false;
    bool m_doesPushEntities = false;
    bool m_isPushedByWalls = false;
    bool m_isHitByBullets = false;    
    
    bool    m_canSwim = false;
    TileHeatMap* m_heatMapToPlayer = nullptr;

    int m_maxSteps = 0;

    Vec2 m_goalPosition;
    std::vector<Vec2> m_pathPoints;

    RaycastResult2D m_lRayResult;
    RaycastResult2D m_rRayResult;
};