#pragma once
#include "Game.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Game/Tile.hpp"
#include "ActorHandle.hpp"
#include "SpawnInfo.hpp"
#include "Engine/Core/Timer.hpp"


class Actor;
class MapDefinition;
class Projectile;

class Map
{
public:
	Map(Game* game, const MapDefinition* definition);	
	~Map();

	void InitializeMap();
	void InitializeTileMapFromImage(std::string const& imagePath);
	void InitializeActors();
	void CreateEachTile(int columnIndex, int rowIndex, std::string& tileTypeName);
	void CreateGeometry();
	void AddGeometryForWall(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs,
		const AABB3& bounds, const AABB2& UVs,int wallFace) const;
	void AddGeometryForFloor(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs,
		const AABB3& bounds, const AABB2& UVs) const;
	void AddGeometryForCeiling(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs,
		const AABB3& bounds, const AABB2& UVs) const;
	void CreateBuffers();

// 	void InitializeEnemies();
// 	void SetPlayer(Player* player);

	//-----------------
	const Tile* GetTile(int x, int y) const;
	std::string const& GetTileTypeNameFromCoords(int coor_x, int coor_y) const;
	std::string const& GetTileTypeNameFromColor(Rgba8 const& pixelColor) const;
	int	GetTileDefIndexFromName(std::string const& typeName) const;
	IntVec2 GetTileCoordsFromPoint(Vec2 const& point) const;
	bool IsTileSolid(IntVec2 const& coordinate) const;
	TileDefinition const& MatchTileDef(std::string const& tileName) const;
	bool IsPositionInBounds(Vec3 position, const float tolerance = 0.01f) const;
	bool AreCoordsInBounds(int x, int y) const;
	//------------------
	ActorHandle* SpawnActor(const SpawnInfo& spawnInfo);
	ActorHandle* SpawnProjectile(const SpawnInfo& spawnInfo,ActorHandle* ownerHandle);
	ActorHandle* SpawnPlayer(PlayerController* curPlayerController,int viewPortType);
	Actor* GetActorByHandle(const ActorHandle handle) const;
	void DeleteDestroyedActors();
	//------------------
	void Update(float deltaTime);
	void FixedUpdate(float deltaTime);
 	void CollideActors();
 	void CollideActorsWithMap();
	void PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius);
	//---------------------------------------------------------------------------------
 	RaycastResult3D RaycastAll(const Vec3& start, const Vec3& direction, float distance,ActorHandle* sourceActorHandle);
 	RaycastResult3D RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, ActorHandle* sourceActorHandle, ActorHandle& resultActor) const;
 	RaycastResult3D RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const;
 	RaycastResult3D RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance) const;
	bool IsEyeSightBlock(const Vec3& start, const Vec3& direction, float distance, ActorHandle* sourceActorHandle, float aimDist);
	//---------------------------------------------------------------------------------
	void UpdateLight();
	//void UpdateSprint();
	//void UpdatePlayerMode(float deltaSeconds);
	//void UpdateRaycastTest();
	//void CalculateAndDrawRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist);
	//------------------
	void Render() const;
	//------------------
	ActorHandle* GetClosestVisibleEnemy(Actor* myActor, Faction const& aimFaction);
	ActorHandle* GetMeleeTarget(Actor* myActor, Faction const& aimFaction, float meleeRadius, float meleeDegrees);
	//------------------
	void DebugPossessNext();
public:
	Game* m_game = nullptr;
	//Actor* m_player = nullptr;
	//---------------------------------------
	bool m_isPlayerMode = false;
	bool m_isSprint = false;

	Projectile* m_projectile = nullptr;
	std::vector<Actor*> m_actors;
	std::vector<Actor*> m_spawnPoint;
	unsigned int m_nextActorUID = 1;

protected:
	const MapDefinition*	m_definition = nullptr;
	std::vector<Tile>		m_tiles;
	IntVec2					m_dimensions;

	std::vector<Vertex_PCUTBN>	m_vertexs;
	std::vector<unsigned int>	m_indexs;
	Texture*					m_texture = nullptr;
	Shader*						m_shader = nullptr;
	VertexBuffer*				m_vertexBuffer = nullptr;
	IndexBuffer*				m_indexBuffer = nullptr;

	EulerAngles m_lightOrientation = EulerAngles(45.f, 45.f, 45.f);
	Vec3 m_sunDirection = Vec3(2.f, 1.f, -1.f);
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;


	Timer m_physicsTimer;

	int m_curPlayerActorIndex = 0;
	
	float m_physicsUpdateAccumulator = 0.f;
	float m_fixedTimeStep = 1.0f / 500.0f;

	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;
};