#include "Map.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/TileDefinition.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include <Engine/Renderer/SpriteSheet.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Game/MapDefinition.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Input/InputSystem.cpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "Game/PlayerController.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/CubeSkyBox.hpp"
#include "FieldObject.hpp"
#include "PointLightAsset.hpp"
extern SpriteSheet* g_terrianSpriteSheet;

static const Vec3 s_presetPositions[10] = {
	Vec3(13.5f, 8.5f, 0.6f),
	Vec3(15.5f, 8.5f, 0.6f),
	Vec3(11.5f, 21.5f, 0.6f),
	Vec3(18.5f, 21.5f, 0.6f),
	Vec3(13.5f, 39.5f, 0.6f),
	Vec3(17.5f, 39.5f, 0.6f),

	Vec3(-3.5f, 3.0f, 1.0f),
	Vec3(3.0f, -3.0f, 1.0f),
	Vec3(-3.0f, -3.0f, 1.0f),
	Vec3(0.0f, 0.0f, 5.0f)
};


Map::Map(Game* game, const MapDefinition* definition, bool isGold)
	:m_game(game), m_definition(definition),m_physicsTimer(Timer(1.f/60.f,game->m_gameClock)),m_isGold(isGold)
{
	m_texture = m_definition->m_spriteSheetTexture;
	m_shader = m_definition->m_shader;
	m_shadowShader = g_theRenderer->CreateShaderFromFile("Data/Shaders/ShadowMap", VertexType::VERTEX_PCUTBN);

	m_pointLights.reserve(20);
// 	PointLight pointLight = PointLight(
// 		Vec3(3.f, 3.f, 0.5f),  
// 		20.f,                  
// 		Rgba8::CYAN,           
// 		2.0f,                  
// 		Vec3(1.0f, 0.09f, 0.032f) 
// 	);
// 	m_pointLights.push_back(pointLight);


	m_spotLights.reserve(10);
	SpotLight spotLight1 = SpotLight(
		Vec3(13.5f, 9.5f, 0.6f),
		20.f,
		Rgba8::BLUE,
		2.5f,
		Vec3(1.0f, 0.02f, 0.01f),
		10.f,Vec3(1.2f,0.f,-1.f).GetNormalized(),20.f
	);
	SpotLight spotLight2 = SpotLight(
		Vec3(15.5f, 9.5f, 0.6f),
		20.f,
		Rgba8::MAGNETA,
		2.5f,
		Vec3(1.0f, 0.02f, 0.01f),
		10.f, Vec3(-1.2f, 0.f, -1.f).GetNormalized(), 20.f
	);
	m_spotLights.push_back(spotLight1);
	m_spotLights.push_back(spotLight2);
	//m_mapClock = new Clock(*m_game->m_gameClock);

	//----------sky box--------------------
	std::string skyboxPaths[] = {
	"Data/Images/SkyBox1/skyhsky_lf.png",
	"Data/Images/SkyBox1/skyhsky_rt.png",
	"Data/Images/SkyBox1/skyhsky_dn.png",
	"Data/Images/SkyBox1/skyhsky_up.png",
	"Data/Images/SkyBox1/skyhsky_ft.png",
	"Data/Images/SkyBox1/skyhsky_bk.png",
	};

	std::string skyBoxShaderPath = "Data/Shaders/CubeSkyBox";

	m_cubeSkybox = new CubeSkyBox(g_theRenderer, skyboxPaths, &skyBoxShaderPath);

	m_field.reserve(12);

	m_lanterns.reserve(10);
	for (int i = 0; i < 10; ++i)
	{
		PointLightAsset* lantern = new PointLightAsset(s_presetPositions[i], this);
		m_lanterns.push_back(lantern);
	}
}

Map::~Map()
{
// 	delete m_mapClock;
// 	m_mapClock = nullptr;

	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;

	for (Actor* actor : m_actors)
	{
		delete actor;
		actor = nullptr;
	}
	m_actors.clear();

	for (Actor* spawnPoint : m_spawnPoint)
	{
		spawnPoint = nullptr;
	}

	delete m_cubeSkybox;
	m_cubeSkybox = nullptr;

	for (FieldObject* field : m_field)
	{
		if (field)
		{
			delete field;
			field = nullptr;
		}
	}
}

void Map::InitializeMap()
{
	CreateBuffers();
	InitializeTileMapFromImage(m_definition->m_mapImageName);
	//InitializeEnemies();
	InitializeActors();
// 	m_player = new Player(1,this);
// 	m_player->m_curMap = this;
//  m_game->m_playerController0->m_playerCam.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
}

void Map::InitializeTileMapFromImage(std::string const& imagePath)
{
	Image* curMapImage = g_theRenderer->CreateImageFromFile(imagePath.c_str());
	m_dimensions = curMapImage->GetDimensions();
	m_tiles.reserve(m_dimensions.x * m_dimensions.y);
	for (int rowIndex = 0; rowIndex < m_dimensions.y; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < m_dimensions.x; columnIndex++)
		{
			Rgba8 pixelColor = curMapImage->GetTexelColor(IntVec2(columnIndex, rowIndex));
			std::string thisTileType = GetTileTypeNameFromColor(pixelColor);
			CreateEachTile(columnIndex, rowIndex, thisTileType);
		}
	}
	delete curMapImage;
	curMapImage = nullptr;

	CreateGeometry();

	//AddVertsForAllTiles();

	// 	int maxSteps = PopulateDistanceField(*m_heatMapDistance, m_startCoords, 999999.f, true, false);
	// 	m_heatMapDistance->AddVertsForDebugDraw(m_heatmapVerts,
	// 		AABB2(0.f, 0.f, m_dimensions.x * m_tileSizeX, m_dimensions.y * m_tileSizeY),
	// 		FloatRange(0.f, (float)maxSteps));
}

void Map::InitializeActors()
{
	for (int i = 0; i < (int)m_definition->m_spawnInfos.size(); i++)
	{
		SpawnActor(m_definition->m_spawnInfos[i]);
	}
	if (m_game->m_playerController0)
	{
		if (m_game->m_playerController1)
		{
			SpawnPlayer(m_game->m_playerController0,0);
		}
		else
		{
			SpawnPlayer(m_game->m_playerController0, -1);
		}
	}
	if (m_game->m_playerController1)
	{
		if (m_game->m_playerController0)
		{
			SpawnPlayer(m_game->m_playerController1, 1);
		}
		else
		{
			SpawnPlayer(m_game->m_playerController1, -1);
		}
	}
}

void Map::CreateEachTile(int columnIndex, int rowIndex, std::string& tileTypeName)
{
	IntVec2 dimensions = IntVec2(columnIndex, rowIndex);
	Tile newTile =Tile(GetTileDefIndexFromName(tileTypeName), dimensions);
	m_tiles.push_back(newTile);
}

void Map::CreateGeometry()
{
	for (int i = 0; i < (int)m_tiles.size(); i++)
	{
		TileDefinition& curTileDef = TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex];
		if (curTileDef.m_floorSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex= curTileDef.m_floorSpriteCoords.x 
				+ (curTileDef.m_floorSpriteCoords.y * g_terrianSpriteSheet->m_gridLayout.x);
			AABB2 floorUv = g_terrianSpriteSheet->GetSpriteUVs(spriteIndex);
			AddGeometryForFloor(m_vertexs, m_indexs, m_tiles[i].m_bounds, floorUv);

		}
		if (curTileDef.m_ceilingSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex = curTileDef.m_ceilingSpriteCoords.x
				+ (curTileDef.m_ceilingSpriteCoords.y * g_terrianSpriteSheet->m_gridLayout.x);
			AABB2 ceilingUv = g_terrianSpriteSheet->GetSpriteUVs(spriteIndex);
			AddGeometryForCeiling(m_vertexs, m_indexs, m_tiles[i].m_bounds, ceilingUv);
		}
		if (curTileDef.m_wallSpriteCoords != IntVec2(-1, -1))
		{
			int spriteIndex = curTileDef.m_wallSpriteCoords.x
				+ (curTileDef.m_wallSpriteCoords.y * g_terrianSpriteSheet->m_gridLayout.x);
			AABB2 wallUv = g_terrianSpriteSheet->GetSpriteUVs(spriteIndex);
			for (int j = 0; j < 4; j++) 
			{
				AddGeometryForWall(m_vertexs, m_indexs, m_tiles[i].m_bounds, wallUv, j);
			}
		}
	}
}

void Map::AddGeometryForWall(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs, const AABB3& bounds, const AABB2& UVs, int wallFace) const
{
	Vec3 bottomLeft, bottomRight, topRight, topLeft;

	switch (wallFace)
	{
	case 0: // Y MAX
		bottomLeft = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
		bottomRight = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
		topRight = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
		topLeft = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
		break;

	case 1: // Y MIN
		bottomLeft = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
		bottomRight = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
		topRight = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
		topLeft = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
		break;

	case 2: // X MAX
		bottomLeft = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
		bottomRight = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
		topRight = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
		topLeft = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
		break;

	case 3: // X MIN
		bottomLeft = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
		bottomRight = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
		topRight = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
		topLeft = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
		break;

	default:
		return;
	}
	AddVertsForQuad3D_WithTBN(vertexs, indexs, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, UVs);
}

void Map::AddGeometryForFloor(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs, const AABB3& bounds, const AABB2& UVs) const
{
	Vec3 bottomLeft(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 bottomRight(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 topRight(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 topLeft(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	AddVertsForQuad3D_WithTBN(vertexs, indexs, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, UVs);
}

void Map::AddGeometryForCeiling(std::vector<Vertex_PCUTBN>& vertexs, std::vector<unsigned int>& indexs, const AABB3& bounds, const AABB2& UVs) const
{
	Vec3 bottomLeft(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 bottomRight(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 topRight(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 topLeft(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	AddVertsForQuad3D_WithTBN(vertexs, indexs, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, UVs);
}

void Map::CreateBuffers()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(24, sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(24);
}

//void Map::InitializeEnemies()
//{
//	m_actors.reserve(10);
//	Enemy* enemy1 = new Enemy(Vec3(7.5f, 8.5f, 0.25f), EulerAngles(0.f, 0.f, 0.f), Rgba8(255, 0, 0));
//	Enemy* enemy2 = new Enemy(Vec3(8.5f, 8.5f, 0.125f), EulerAngles(0.f, 0.f, 0.f), Rgba8(255, 0, 0));
//	Enemy* enemy3 = new Enemy(Vec3(9.5f, 8.5f, 0.0f), EulerAngles(0.f, 0.f, 0.f), Rgba8(255, 0, 0));
//	Projectile* projectile1 = new Projectile(Vec3(5.5f, 8.5f, 0.0f), EulerAngles(0.f, 0.f, 0.f), Rgba8::BLUE);
//	m_actors.push_back(enemy1);
//	m_actors.push_back(enemy2);
//	m_actors.push_back(enemy3);
//	m_actors.push_back(projectile1);
//	m_projectile = projectile1;
//}
//
//void Map::SetPlayer(Player* player)
//{
//	m_player = player;
//	m_player->m_position = Vec3(2.5f, 8.5f, 0.5f);
//}

const Tile* Map::GetTile(int x, int y) const
{
	int index = x + y * m_dimensions.x;
	return &m_tiles[index];
}

std::string const& Map::GetTileTypeNameFromCoords(int coor_x, int coor_y) const
{
	int index = coor_x + coor_y * m_dimensions.x;
	return TileDefinition::s_tileDefinitions[m_tiles[index].m_tileDefIndex].m_name;
}

std::string const& Map::GetTileTypeNameFromColor(Rgba8 const& pixelColor) const
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (pixelColor == TileDefinition::s_tileDefinitions[i].m_mapImagePixelColor)
		{
			return TileDefinition::s_tileDefinitions[i].m_name;
		}
	}
	ERROR_AND_DIE("image's color is not appear in tile defs");
}

int Map::GetTileDefIndexFromName(std::string const& typeName) const
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

IntVec2 Map::GetTileCoordsFromPoint(Vec2 const& point) const
{
	return IntVec2(static_cast<int>(floorf(point.x)), static_cast<int>(floorf(point.y)));
}

bool Map::IsTileSolid(IntVec2 const& coordinate) const
{
	return MatchTileDef(GetTileTypeNameFromCoords(coordinate.x, coordinate.y)).m_isSolid;
}

TileDefinition const& Map::MatchTileDef(std::string const& tileName) const
{
	for (int i = 0; i < (int)TileDefinition::s_tileDefinitions.size(); i++)
	{
		if (tileName == TileDefinition::s_tileDefinitions[i].m_name)
		{
			return TileDefinition::s_tileDefinitions[i];
		}
	}
	ERROR_AND_DIE("can not find the tile def");
}

bool Map::IsPositionInBounds(Vec3 position, const float tolerance) const
{
	if (position.x < -tolerance || position.x > m_dimensions.x + tolerance)
		return false;

	if (position.y < -tolerance || position.y > m_dimensions.y + tolerance)
		return false;

	return true;
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	if (x < 0 || x >= m_dimensions.x)
		return false;

	if (y < 0 || y >= m_dimensions.y)
		return false;

	return true;
}

ActorHandle* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	int defIndex = -1;
	for (int i = 0; i < (int)ActorDefinition::s_actorDefinitions.size(); i++)
	{
		if (ActorDefinition::s_actorDefinitions[i]->m_name == spawnInfo.m_typeName)
		{
			defIndex = i;
			break; 
		}
	}
	if (defIndex == -1)
	{
		return nullptr;
	}

	Actor* newActor = new Actor(defIndex, this, spawnInfo.m_position, spawnInfo.m_orientation);

	bool isSpawnPoint = (ActorDefinition::s_actorDefinitions[defIndex]->m_name == "SpawnPoint");

	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			m_actors[i] = newActor;
			m_actors[i]->m_actorHandle = new ActorHandle(m_nextActorUID, i);
			if (m_actors[i]->m_controller)
			{
				m_actors[i]->m_controller->Possess(*m_actors[i]->m_actorHandle);
			}

			m_nextActorUID = (m_nextActorUID >= ActorHandle::MAX_ACTOR_UID) ? 1 : m_nextActorUID + 1;

			return m_actors[i]->m_actorHandle;
		}
	}

	int newIndex = static_cast<int>(m_actors.size());
	newActor->m_actorHandle = new ActorHandle(m_nextActorUID, newIndex);
	m_actors.push_back(newActor);
	if (newActor->m_controller)
	{
		newActor->m_controller->Possess(*newActor->m_actorHandle);
	}
	
	if (isSpawnPoint)
	{
		m_spawnPoint.push_back(newActor);
	}

	m_nextActorUID = (m_nextActorUID >= ActorHandle::MAX_ACTOR_UID) ? 1 : m_nextActorUID + 1;

	return newActor->m_actorHandle;
}

ActorHandle* Map::SpawnProjectile(const SpawnInfo& spawnInfo, ActorHandle* ownerHandle)
{
	int defIndex = -1;
	for (int i = 0; i < (int)ActorDefinition::s_projectileActorDefinitions.size(); i++)
	{
		if (ActorDefinition::s_projectileActorDefinitions[i]->m_name == spawnInfo.m_typeName)
		{
			defIndex = i;
			break;
		}
	}
	if (defIndex == -1)
	{
		return nullptr;
	}

	Actor* newActor = new Actor(defIndex, this, spawnInfo.m_position, spawnInfo.m_orientation,true);

	//bool isSpawnPoint = (ActorDefinition::s_actorDefinitions[defIndex].m_name == "SpawnPoint");

	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			m_actors[i] = newActor;
			newActor->m_actorHandle = new ActorHandle(m_nextActorUID, i);
			m_nextActorUID = (m_nextActorUID >= ActorHandle::MAX_ACTOR_UID) ? 1 : m_nextActorUID + 1;
			newActor->m_isProjectile = true;
			newActor->m_owner = ownerHandle;
			return newActor->m_actorHandle;
		}
	}

	int newIndex = static_cast<int>(m_actors.size());
	newActor->m_actorHandle = new ActorHandle(m_nextActorUID, newIndex);
	newActor->m_isProjectile = true;
	newActor->m_owner = ownerHandle;
	m_actors.push_back(newActor);
	m_nextActorUID = (m_nextActorUID >= ActorHandle::MAX_ACTOR_UID) ? 1 : m_nextActorUID + 1;

	return newActor->m_actorHandle;
}

ActorHandle* Map::SpawnPlayer(PlayerController* curPlayerController,int viewPortType)
{
	int spawnPointIndex = m_game->m_rng.RollRandomIntInRange(0, (int)m_spawnPoint.size() - 1);
	SpawnInfo playerSpawnInfo = SpawnInfo("Marine", Faction::GOOD,
		m_spawnPoint[spawnPointIndex]->m_position, m_spawnPoint[spawnPointIndex]->m_orientation);
	ActorHandle* newPlayerHandle=SpawnActor(playerSpawnInfo);
	curPlayerController->Possess(*newPlayerHandle,viewPortType);

	return newPlayerHandle;
}

Actor* Map::GetActorByHandle(const ActorHandle handle) const
{
	if (!handle.IsValid())
	{
		return nullptr;
	}
	unsigned int index = handle.GetIndex();
	if (index >= m_actors.size())
	{
		return nullptr;
	}

	Actor* actor = m_actors[index];

	if (actor == nullptr)
	{
		return nullptr;
	}

	if (handle != *m_actors[index]->m_actorHandle)
	{
		return nullptr;
	}
	return m_actors[index];
}

void Map::DeleteDestroyedActors()
{
	for (size_t i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] && m_actors[i]->m_isDestroy)
		{
			delete m_actors[i];
			m_actors[i] = nullptr; 
		}
	}
}



void Map::Update(float deltaTime)
{
	DeleteDestroyedActors();

	UpdateLight();

	if (m_isGold)
	{
		UpdateGold(deltaTime);
	}

	if (!(m_game->m_playerController0 && m_game->m_playerController1))
	{
		if (g_theInput->WasKeyJustPressed('N'))
		{
			DebugPossessNext();
		}
	}


	if (m_game->m_playerController0)
	{
		if (m_game->m_playerController0->GetActor())
		{
			m_game->m_playerController0->Update(deltaTime);
		}
		else
		{
			if (m_game->m_playerController1)
			{
				SpawnPlayer(m_game->m_playerController0, 0);
			}
			else
			{
				SpawnPlayer(m_game->m_playerController0, -1);
			}
			
		}
	}

	if (m_game->m_playerController1)
	{
		if (m_game->m_playerController1->GetActor())
		{
			m_game->m_playerController1->Update(deltaTime);
		}
		else
		{
			if (m_game->m_playerController0)
			{
				SpawnPlayer(m_game->m_playerController1, 1);
			}
			else
			{
				SpawnPlayer(m_game->m_playerController1, -1);
			}
		}
	}


	for (Actor* actor : m_actors)
	{
		if (actor)
		{
			actor->Update(deltaTime);
		}
	}

	CollideActors();
	CollideActorsWithMap();


	//FixedUpdate(deltaTime);

	for (Actor* actor : m_actors)
	{
		if (actor)
		{
			if (actor->m_actorDef->m_simulated && !actor->m_isDead)
			{
				actor->UpdatePhysics(deltaTime);
			}
		}
	}

	//UpdatePlayerMode(deltaTime);
	//UpdateRaycastTest();
}

void Map::UpdateGold(float deltaTime)
{
	m_curDayTime += deltaTime;
	if (m_curDayTime > m_totalDayTime)
	{
		m_isSun = true;
		m_curDayTime = 0.f;
	}
	else if (m_curDayTime > m_totalDayTime / 2.f)
	{
		m_isSun = false;
	}

// 	for (FieldObject* field : m_field)
// 	{
// 		if (field)
// 		{
// 			if (field->m_isDie)
// 			{
// 				delete field;
// 				field = nullptr;
// 			}
// 		}
// 
// 	}
// 	for (FieldObject* field : m_field)
// 	{
// 		if (field)
// 		{
// 			field->Update(deltaTime);
// 		}
// 	}

	for (auto it = m_field.begin(); it != m_field.end();)
	{
		if ((*it) && (*it)->m_isDie)
		{
			delete* it;
			it = m_field.erase(it); 
		}
		else
		{
			++it;
		}
	}

	for (FieldObject* field : m_field)
	{
		if (field)
		{
			field->Update(deltaTime);
		}
	}
}

void Map::FixedUpdate(float deltaTime)
{
	m_physicsUpdateAccumulator += deltaTime;
	while (m_physicsUpdateAccumulator >= m_fixedTimeStep)
	{
		for (Actor* actor : m_actors)
		{
			if (actor)
			{
				if (actor->m_actorDef->m_simulated && !actor->m_isDead)
				{
					actor->UpdatePhysics(m_fixedTimeStep);
				}
			}
		}
		m_physicsUpdateAccumulator -= m_fixedTimeStep;
	}
}


void Map::CollideActors()
{
	for (Actor* actorA : m_actors)
	{
		if (actorA == nullptr) continue;
		if (actorA->m_isDead) continue;
		if (!actorA->m_actorDef->m_collideWithActors) continue;
		for (Actor* actorB : m_actors)
		{
			if (actorB == nullptr) continue;
			if (actorB->m_isDead) continue;
			if (!actorB->m_actorDef->m_collideWithActors) continue;
			if (actorA == actorB)
			{
				continue;
			}
			if (actorA->m_actorHandle == actorB->m_owner || actorA->m_owner == actorB->m_actorHandle)
			{
				continue;
			}
			bool canPushEachOther = actorA->m_actorDef->m_collideWithActors && actorB->m_actorDef->m_collideWithActors;
			Vec3 aColliderCenter = actorA->m_position + actorA->m_physicCollider.m_center;
			Vec3 bColliderCenter = actorB->m_position + actorB->m_physicCollider.m_center;
 			if (abs(aColliderCenter.z - bColliderCenter.z) >= actorA->m_physicCollider.m_halfHeight + actorB->m_physicCollider.m_halfHeight)
 			{
 				continue;
 			}
			Vec2 aCCenterXY = Vec2(aColliderCenter.x, aColliderCenter.y);
			Vec2 bCCenterXY = Vec2(bColliderCenter.x, bColliderCenter.y);
//  			if (!canAPushB && !canBPushA)
//  			{
//  				continue;
//  			}
			if (canPushEachOther)
			{
				PushDiscsOutOfEachOther2D(aCCenterXY, actorA->m_physicCollider.m_radius, bCCenterXY, actorB->m_physicCollider.m_radius);
				if (actorA->m_position.x != aCCenterXY.x ||
					actorA->m_position.y != aCCenterXY.y ||
					actorB->m_position.x != bCCenterXY.x ||
					actorB->m_position.y != bCCenterXY.y)
				{
					actorA->OnCollideOtherActor(actorB->m_actorHandle);
					actorB->OnCollideOtherActor(actorA->m_actorHandle);
				}
				actorA->m_position.x = aCCenterXY.x;
				actorA->m_position.y = aCCenterXY.y;
				actorB->m_position.x = bCCenterXY.x;
				actorB->m_position.y = bCCenterXY.y;
			}
// 			else if (!canAPushB && canBPushA)
// 			{
// 				PushDiscOutOfDisc2D(aCCenterXY, actorA->m_physicCollider.m_radius, bCCenterXY, actorB->m_physicCollider.m_radius);
// 				actorA->m_position.x = aCCenterXY.x;
// 				actorA->m_position.y = aCCenterXY.y;
// 				actorB->m_position.x = bCCenterXY.x;
// 				actorB->m_position.y = bCCenterXY.y;
// 			}
// 			else if (canAPushB && !canBPushA)
// 			{
// 				PushDiscOutOfDisc2D(bCCenterXY, actorB->m_physicCollider.m_radius, aCCenterXY, actorA->m_physicCollider.m_radius);
// 				actorA->m_position.x = aCCenterXY.x;
// 				actorA->m_position.y = aCCenterXY.y;
// 				actorB->m_position.x = bCCenterXY.x;
// 				actorB->m_position.y = bCCenterXY.y;
// 			}
		}
	}
}

// void Map::CollideActor(Actor* actorA, Actor* actorB)
// {
// }

void Map::CollideActorsWithMap()
{
	for (Actor* actor : m_actors)
	{
		if (actor == nullptr) continue;

		if (!actor->m_isDead&&actor->m_actorDef->m_collideWithWorld)
		{
			IntVec2 entityTileCoordsPos = GetTileCoordsFromPoint(Vec2(actor->m_position.x,actor->m_position.y));
			Vec2 actorPosFromZ = Vec2(actor->m_position.x,actor->m_position.y);
			//N
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y + 1), actorPosFromZ, actor->m_physicCollider.m_radius);
			//S
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x, entityTileCoordsPos.y - 1), actorPosFromZ, actor->m_physicCollider.m_radius);
			//W																							
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y), actorPosFromZ, actor->m_physicCollider.m_radius);
			//E																							
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y), actorPosFromZ, actor->m_physicCollider.m_radius);
			//NE
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y + 1), actorPosFromZ, actor->m_physicCollider.m_radius);
			//NW																							
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x + 1, entityTileCoordsPos.y - 1), actorPosFromZ, actor->m_physicCollider.m_radius);
			//SE																							
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y + 1), actorPosFromZ, actor->m_physicCollider.m_radius);
			//SW																							
			PushOutOfEachTile(IntVec2(entityTileCoordsPos.x - 1, entityTileCoordsPos.y - 1), actorPosFromZ, actor->m_physicCollider.m_radius);

			float newZ = actor->m_position.z;
			if (actor->m_position.z < 0.f)
			{
				newZ = 0.f;
			}

			if (actor->m_position.z > 1.f - actor->m_actorDef->m_physicsHeight)
			{
				newZ = 1.f - actor->m_actorDef->m_physicsHeight;
			}

			if (actor->m_position.x != actorPosFromZ.x ||
				actor->m_position.y != actorPosFromZ.y||
				actor->m_position.z!=newZ)
			{
				actor->OnCollideWorld();
			}
			actor->m_position.x = actorPosFromZ.x;
			actor->m_position.y = actorPosFromZ.y;
			actor->m_position.z = newZ;
		}
	}
}

// void Map::CollideActorWithMap(Actor* actor)
// {
//}

void Map::PushOutOfEachTile(IntVec2 tileCoords, Vec2& entityPos, float entityPhyRadius)
{
	if (IsTileSolid(tileCoords))
	{
		AABB2 thisTileBox = AABB2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y),
			static_cast<float>(tileCoords.x + 1.f), static_cast<float>(tileCoords.y + 1));
		PushDiscOutOfAABB2D(entityPos, entityPhyRadius, thisTileBox);
	}
}

RaycastResult3D Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance, ActorHandle* sourceActorHandle) 
{
	RaycastResult3D finalResult;

	if (!IsPositionInBounds(start))
	{
		return finalResult;
	}

	ActorHandle resulatActorHandle;
	RaycastResult3D resultActors=RaycastWorldActors(start, direction, distance,sourceActorHandle, resulatActorHandle);
	RaycastResult3D resultXY = RaycastWorldXY(start, direction, distance);
	RaycastResult3D resultZ = RaycastWorldZ(start, direction, distance);
	
	float minDistance = FLT_MAX;

	if (resultActors.m_didImpact && resultActors.m_impactDist < minDistance) {
		minDistance = resultActors.m_impactDist;
		finalResult = resultActors;
	}

	if (resultXY.m_didImpact && resultXY.m_impactDist < minDistance) {
		minDistance = resultXY.m_impactDist;
		finalResult = resultXY;
	}

	if (resultZ.m_didImpact && resultZ.m_impactDist < minDistance) {
		minDistance = resultZ.m_impactDist;
		finalResult = resultZ;
	}

	PlayerController* listener = nullptr;
	if (m_game->m_playerController0)
	{
		listener = m_game->m_playerController0;
	}
	else
	{
		listener = m_game->m_playerController1;
	}

	if (finalResult.m_impactPos== resultActors.m_impactPos)
	{
		GetActorByHandle(resulatActorHandle)->OnCollideRay(sourceActorHandle,finalResult.m_rayFwdNormal);
		SpawnInfo bloodSpawnInfo = SpawnInfo("BloodSplatter", Faction::NEUTRAL,
			finalResult.m_impactPos, EulerAngles(0.f,0.f,0.f));
		ActorHandle* blood=SpawnActor(bloodSpawnInfo);
		GetActorByHandle(*blood)->Die(listener);
	}
	else
	{
		SpawnInfo hitSpawnInfo = SpawnInfo("BulletHit", Faction::NEUTRAL,
			finalResult.m_impactPos, EulerAngles(0.f, 0.f, 0.f));
		ActorHandle* hit = SpawnActor(hitSpawnInfo);
		
		GetActorByHandle(*hit)->Die(listener);
	}

	return finalResult;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, ActorHandle* sourceActorHandle,ActorHandle& resultActor) const
{
	RaycastResult3D result = RaycastResult3D(direction, start, distance);
	float impactDist = FLT_MAX;
	for (Actor* actor : m_actors)
	{
		if (!actor) continue;
		if (sourceActorHandle == actor->m_actorHandle) continue;
		//if (actor->m_actorDef->m_flying) continue;
		Vec3 colliderCenter = actor->m_position+ Vec3(0.f, 0.f, actor->m_physicCollider.m_halfHeight);
		RaycastResult3D curResult=RaycastVsZCylinder3D(start, direction, distance,
			colliderCenter, actor->m_physicCollider.m_radius, actor->m_physicCollider.m_halfHeight);
		if (curResult.m_didImpact && curResult.m_impactDist < impactDist)
		{
			result = curResult;
			impactDist = curResult.m_impactDist;
			resultActor = *actor->m_actorHandle;
		}
	}
	return result;
}

RaycastResult3D Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult3D result = RaycastResult3D(direction,start,distance);
	IntVec2 startTileCoords = GetTileCoordsFromPoint(Vec2(start.x,start.y));

	if (MatchTileDef(GetTileTypeNameFromCoords(startTileCoords.x, startTileCoords.y)).m_isSolid)
	{
		result.m_didImpact = true;
		result.m_impactPos = start;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -direction;
		return result;
	}
	Vec3 rayEnd = start + direction * distance;

	//assume tile size is 1.f
	Vec2 directionXY = Vec2(direction.x, direction.y).GetNormalized();
	if (directionXY.GetLengthSquared() < 0.001f)
	{
		result.m_didImpact = false;
		return result;
	}
	float fwdDistPerXCrossing = 1.f / abs(directionXY.x);
	int tileStepDirectionX = (directionXY.x < 0) ? -1 : 1;
	float xAtFirstXCrossing = startTileCoords.x + (tileStepDirectionX + 1.f) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float fwdDistPerYCrossing = 1.f / abs(directionXY.y);
	int tileStepDirectionY = (directionXY.y < 0) ? -1 : 1;
	float yAtFirstYCrossing = startTileCoords.y + (tileStepDirectionY + 1.f) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	float rayDistXY = DotProduct3D(rayEnd - start, Vec3(directionXY.x, directionXY.y, 0.f));
	while (true)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) //touch x sooner
		{
			if (fwdDistAtNextXCrossing > rayDistXY)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.x += tileStepDirectionX;
			if (IsTileSolid(startTileCoords))
			{
				result.m_didImpact = true;
				Vec2 imapactPosXY= Vec2(start.x,start.y) + directionXY * fwdDistAtNextXCrossing;
				//float impactDistXY = fwdDistAtNextXCrossing;
				float t = (imapactPosXY.x - start.x) / (rayEnd.x - start.x);
				result.m_impactPos = Vec3(imapactPosXY.x, imapactPosXY.y, start.z+t*(rayEnd.z - start.z));
				result.m_impactDist = t * distance;
				result.m_impactNormal = Vec3((float)-tileStepDirectionX, 0.f,0.f);
				return result;
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else
		{
			if (fwdDistAtNextYCrossing > rayDistXY)
			{
				result.m_didImpact = false;
				return result;
			}
			startTileCoords.y += tileStepDirectionY;
			if (IsTileSolid(startTileCoords))
			{
				result.m_didImpact = true;
				Vec2 imapactPosXY = Vec2(start.x, start.y) + directionXY * fwdDistAtNextYCrossing;
				//float impactDistXY = fwdDistAtNextYCrossing;
				float t = (imapactPosXY.y - start.y) / (rayEnd.y - start.y);
				result.m_impactPos = Vec3(imapactPosXY.x, imapactPosXY.y, start.z + t * (rayEnd.z - start.z));
				result.m_impactDist = t * distance;
				result.m_impactNormal = Vec3(0.f, (float)-tileStepDirectionY, 0.f);
				return result;
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
	return RaycastResult3D();
}

RaycastResult3D Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance) const
{
	Vec3 rayEnd = start + direction * distance;
	RaycastResult3D result = RaycastResult3D(direction, start, distance);
	if (direction.z > 0.f)
	{
		float impactPosZ = floor(start.z) + 1.f;
		float t = (impactPosZ - start.z) / (rayEnd.z - start.z);
		if (t > 0.f && t < 1.f)
		{
			result.m_didImpact = true;
			result.m_impactDist = t * distance;
			result.m_impactPos = start+t * (rayEnd - start);
			result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
			return result;
		}
		else
		{
			result.m_didImpact = false;
			return result;
		}
	}
	else if (direction.z < 0.f)
	{
		float impactPosZ = floor(start.z);
		float t = (impactPosZ - start.z) / (rayEnd.z - start.z);
		if (t > 0.f && t < 1.f)
		{
			result.m_didImpact = true;
			result.m_impactDist = t * distance;
			result.m_impactPos = start + t * (rayEnd - start);
			result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
			return result;
		}
		else
		{
			result.m_didImpact = false;
			return result;
		}
	}
	else
	{
		result.m_didImpact = false;
		return result;
	}
}

bool Map::IsEyeSightBlock(const Vec3& start, const Vec3& direction, float distance, ActorHandle* sourceActorHandle,float aimDistSq)
{
	RaycastResult3D finalResult;

	if (!IsPositionInBounds(start))
	{
		return true;
	}

	ActorHandle resulatActorHandle;
	RaycastResult3D resultActors = RaycastWorldActors(start, direction, distance, sourceActorHandle, resulatActorHandle);
	RaycastResult3D resultXY = RaycastWorldXY(start, direction, distance);
	RaycastResult3D resultZ = RaycastWorldZ(start, direction, distance);

	float minDistance = FLT_MAX;

	if (resultActors.m_didImpact && resultActors.m_impactDist < minDistance) {
		minDistance = resultActors.m_impactDist;
		finalResult = resultActors;
	}

	if (resultXY.m_didImpact && resultXY.m_impactDist < minDistance) {
		minDistance = resultXY.m_impactDist;
		finalResult = resultXY;
	}

	if (resultZ.m_didImpact && resultZ.m_impactDist < minDistance) {
		minDistance = resultZ.m_impactDist;
		finalResult = resultZ;
	}

	if (finalResult.m_impactPos == resultXY.m_impactPos|| finalResult.m_impactPos == resultZ.m_impactPos)
	{
		if(aimDistSq>finalResult.m_impactDist* finalResult.m_impactDist)
			return true;
	}
	return false;
}

FieldObject* Map::RaycastAllFields(const Vec3& start, const Vec3& direction, float distance) const
{
	for (FieldObject* field : m_field)
	{
		if (field)
		{
			if (field->RaycastAgainst(start, direction, distance))
			{
				return field;
			}
		}
	}
	return nullptr;
}

void Map::UpdateLight()
{
	m_lightOrientation.m_pitchDegrees = (m_curDayTime / m_totalDayTime) * 360.f;
	if (m_lightOrientation.m_pitchDegrees >= 180.f)
	{
		m_lightOrientation.m_pitchDegrees -= 180.f;
		m_sunIntensity = 0.2f;
	}
	else
	{
		m_sunIntensity = 0.85f;
	}
	Vec3 right, up;
	m_lightOrientation.GetAsVectors_IFwd_JLeft_KUp(m_sunDirection,right,up);
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
// 	{
// 		m_sunDirection.x -= 1.f;
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightDirection x: %.2f",
// 			m_sunDirection.x);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
// 	{
// 		m_sunDirection.x += 1.f;
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightDirection x: %.2f",
// 			m_sunDirection.x);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
// 	{
// 		m_sunDirection.y -= 1.f;
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightDirection y: %.2f",
// 			m_sunDirection.y);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
// 	{
// 		m_sunDirection.y += 1.f;
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightDirection y: %.2f",
// 			m_sunDirection.y);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
// 	{
// 		m_sunIntensity -= 0.05f;
// 		m_sunIntensity = GetClamped(m_sunIntensity, 0.f, 1.f);
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightIntensity: %.2f",
// 			m_sunIntensity);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
// 	{
// 		m_sunIntensity += 0.05f;
// 		m_sunIntensity = GetClamped(m_sunIntensity, 0.f, 1.f);
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"LightIntensity: %.2f",
// 			m_sunIntensity);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
// 	{
// 		m_ambientIntensity -= 0.05f;
// 		m_ambientIntensity = GetClamped(m_ambientIntensity, 0.f, 1.f);
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"AmbientIntensity: %.2f",
// 			m_ambientIntensity);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
// 	{
// 		m_ambientIntensity += 0.05f;
// 		m_ambientIntensity = GetClamped(m_ambientIntensity, 0.f, 1.f);
// 		char buffer[256];
// 		snprintf(buffer, sizeof(buffer),
// 			"AmbientIntensity: %.2f",
// 			m_ambientIntensity);
// 		DebugAddMessage(std::string(buffer), 2.f, Rgba8::HILDA, Rgba8::RED);
// 	}
}

//void Map::UpdateSprint()
//{
//	m_isSprint = !m_isSprint;
//}

// void Map::UpdatePlayerMode(float deltaSeconds)
// {
// 	if (m_isSprint)
// 	{
// 		m_player->m_moveSpeed = 15.f;
// 	}
// 	else
// 	{
// 		m_player->m_moveSpeed = 1.f;
// 	}
// 	if (g_theInput->WasKeyJustPressed('F'))
// 	{
// 		m_player->m_playerMode=PlayerMode::FREE_CAMERA;
// 	}
// 	if (m_isPlayerMode)
// 	{
// 		DebugAddScreenText("F1: ControlMode: Actor", AABB2(Vec2(1100.f, 730.f), Vec2(1590.f, 750.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::BLUE, Rgba8::BLUE);
// 		Vec3 fwdDirection;
// 		Vec3 leftDirection;
// 		Vec3 upDirection;
// 		m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
// 		fwdDirection.z = 0.f;
// 		leftDirection.z = 0.f;
// 		upDirection.z = 0.f;
// 		float curMoveSpeed;
// 		if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
// 		{
// 			curMoveSpeed = m_player->m_moveSpeed * 10.f;
// 		}
// 		else
// 		{
// 			curMoveSpeed = m_player->m_moveSpeed;
// 		}
// 
// 		m_projectile->m_velocity = Vec3(0.f, 0.f, 0.f);
// 		Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
// 		if (g_theInput->IsKeyDown('W'))
// 		{
// 			aimDirection += fwdDirection;
// 		}
// 		if (g_theInput->IsKeyDown('S'))
// 		{
// 			aimDirection -= fwdDirection;
// 		}
// 		if (g_theInput->IsKeyDown('A'))
// 		{
// 			aimDirection += leftDirection;
// 		}
// 		if (g_theInput->IsKeyDown('D'))
// 		{
// 			aimDirection -= leftDirection;
// 		}
// 
// 		aimDirection.Normalized();
// 		aimDirection.SetLength(curMoveSpeed);
// 		m_projectile->m_velocity = aimDirection;
// 		m_projectile->m_position += m_projectile->m_velocity * deltaSeconds;
// 	}
// 	else
// 	{
// 		DebugAddScreenText("F1: ControlMode: Camera", AABB2(Vec2(1100.f, 730.f), Vec2(1590.f, 750.f)), 20.f, Vec2(1.f, 1.f), 0.f, Rgba8::WHITE, Rgba8::WHITE);
// 	}
// }

//void Map::UpdateRaycastTest()
//{
//	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
//	{
//		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
//		Vec3 endPoint = m_player->m_position + dir * 10.f;
//		DebugAddWorldLine(m_player->m_position, endPoint, 0.02f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
//		//calculate and debug draw
//		CalculateAndDrawRaycastResult(m_player->m_position, dir, 10.f);
//	}
//	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
//	{
//		Vec3 dir = m_player->m_playerCam.GetOrientation().GetForward_IFwd().GetNormalized();
//		Vec3 endPoint = m_player->m_position + dir * 0.25f;
//		DebugAddWorldLine(m_player->m_position, endPoint, 0.02f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
//		//calculate and debug draw
//		CalculateAndDrawRaycastResult(m_player->m_position, dir, 0.25f);
//	}
//}
//
//void Map::CalculateAndDrawRaycastResult(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
//{
//	RaycastResult3D result = RaycastAll(startPos, fwdNormal, maxDist);
//	if (result.m_didImpact)
//	{
//		DebugAddWorldPoint(result.m_impactPos, 0.06f, 10.f, Rgba8::YELLOW);
//		DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + 0.3f * result.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
//	}
//	////actor
//	//RaycastResult3D resultActors=RaycastWorldActors(startPos, fwdNormal, maxDist);
//	//if (resultActors.m_didImpact)
//	//{
//	//	DebugAddWorldPoint(resultActors.m_impactPos, 0.06f, 10.f, Rgba8::YELLOW);
//	//	DebugAddWorldArrow(resultActors.m_impactPos, resultActors.m_impactPos + 0.3f * resultActors.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
//	//}
//	////world grid
//	//RaycastResult3D resultGridXY=RaycastWorldXY(startPos, fwdNormal, maxDist);
//	//if (resultGridXY.m_didImpact)
//	//{
//	//	DebugAddWorldPoint(resultGridXY.m_impactPos,0.06f,10.f,Rgba8::YELLOW);
//	//	DebugAddWorldArrow(resultGridXY.m_impactPos, resultGridXY.m_impactPos + 0.3f*resultGridXY.m_impactNormal,0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
//	//}
//	////ceiling and floor
//	//RaycastResult3D resultGridZ = RaycastWorldZ(startPos, fwdNormal, maxDist);
//	//if (resultGridZ.m_didImpact)
//	//{
//	//	DebugAddWorldPoint(resultGridZ.m_impactPos, 0.06f, 10.f, Rgba8::YELLOW);
//	//	DebugAddWorldArrow(resultGridZ.m_impactPos, resultGridZ.m_impactPos + 0.3f * resultGridZ.m_impactNormal, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
//	//}
//	//find shortest
//
//	//debug draw
//}

void Map::Render() const
{
	//-----------------------------------------------------------------------
	if (m_game->m_playerController0)
	{
		
		//generate shadow map
		Mat44 lightViewProjection=g_theRenderer->GetDirectLightProjectionMat(m_sunDirection,Vec3(m_dimensions.x*0.5f-1.f,m_dimensions.y*0.5f-1.f,0.f), m_dimensions.GetLength() * 0.5f);

 		g_theRenderer->BeginShadowMapRender(lightViewProjection);
 		for (Actor* actor : m_actors)
 		{
			if (actor)
			{
				if (actor->m_isVisible)
				{
					//actor->UpdateAnimation(m_game->m_playerController0);
					actor->RenderShadowTexture(m_game->m_playerController0);
				}
			}
 			
 		}
		for (FieldObject* field : m_field)
		{
			if (field)
			{
				field->RenderShadowTexture(m_game->m_playerController0);
			}
		}
		g_theRenderer->BindTexture(m_texture);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_FRONT);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_theRenderer->BindShader(m_shadowShader);

		g_theRenderer->DrawVertexArray_WithTBN(m_vertexs, m_indexs, m_vertexBuffer, m_indexBuffer);

 		g_theRenderer->EndShadowMapRender();

		//--------------------------------------------------------------------

		g_theRenderer->BeginCamera(m_game->m_playerController0->m_playerCam);

		m_cubeSkybox->Render();

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
		g_theRenderer->BindTexture(m_texture);
		g_theRenderer->BindShader(m_shader);
		Vec3 normalSunDirection = m_sunDirection.GetNormalized();
		g_theRenderer->SetLightConstants(normalSunDirection, m_sunIntensity, m_ambientIntensity);
		g_theRenderer->SetPointLightsConstants(m_pointLights);
		g_theRenderer->SetSpotLightsConstants(m_spotLights);
		
		//shadow
		g_theRenderer->SetShadowConstants(lightViewProjection);
		g_theRenderer->SetShadowSampleState();
		g_theRenderer->BindShadowTexture();

		g_theRenderer->DrawVertexArray_WithTBN(m_vertexs, m_indexs, m_vertexBuffer, m_indexBuffer);

		if (m_game->m_playerController1)
		{
			m_game->m_playerController1->GetActor()->UpdateAnimation(m_game->m_playerController0);
			m_game->m_playerController1->GetActor()->Render(m_game->m_playerController0);
		}
		for (Actor* actor : m_actors)
		{
			if (actor)
			{
				if (actor->m_isVisible)
				{
					actor->UpdateAnimation(m_game->m_playerController0);
					actor->Render(m_game->m_playerController0);
				}
			}

		}

		for (FieldObject* field : m_field)
		{
			if (field)
			{
				field->Render(m_game->m_playerController0);
			}
		}

		for (PointLightAsset* lantern : m_lanterns)
		{
			if (lantern)
			{
				lantern->Render(m_game->m_playerController0);
			}
		}

		g_theRenderer->EndCamera(m_game->m_playerController0->m_playerCam);

		if (!m_isGold)
		{
			m_game->m_playerController0->RenderPlayerHUD();
		}
		else
		{
			m_game->m_playerController0->RenderGoldHUD();
		}
	}
	
	//-----------------------------------------------------------------------
	if (m_game->m_playerController1)
	{
		m_cubeSkybox->Render();
		g_theRenderer->BeginCamera(m_game->m_playerController1->m_playerCam);

		m_cubeSkybox->Render();

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
		g_theRenderer->BindTexture(m_texture);
		g_theRenderer->BindShader(m_shader);
		Vec3 normalSunDirection = m_sunDirection.GetNormalized();
		g_theRenderer->SetLightConstants(normalSunDirection, m_sunIntensity, m_ambientIntensity);
		g_theRenderer->DrawVertexArray_WithTBN(m_vertexs, m_indexs, m_vertexBuffer, m_indexBuffer);
		//m_player->Render();
		if (m_game->m_playerController0)
		{
			m_game->m_playerController0->GetActor()->UpdateAnimation(m_game->m_playerController1);
			m_game->m_playerController0->GetActor()->Render(m_game->m_playerController1);
		}
		for (Actor* actor : m_actors)
		{
			if (actor)
			{
				if (actor->m_isVisible)
				{
					actor->UpdateAnimation(m_game->m_playerController1);
					actor->Render(m_game->m_playerController1);
				}
			}
		}
		for (FieldObject* field : m_field)
		{
			if (field)
			{
				field->Render(m_game->m_playerController1);
			}
		}

		for (PointLightAsset* lantern : m_lanterns)
		{
			if (lantern)
			{
				lantern->Render(m_game->m_playerController1);
			}
		}

		g_theRenderer->EndCamera(m_game->m_playerController1->m_playerCam);


		if (!m_isGold)
		{
			m_game->m_playerController0->RenderPlayerHUD();
		}
		else
		{
			m_game->m_playerController0->RenderGoldHUD();
		}
	}
}

ActorHandle* Map::GetClosestVisibleEnemy(Actor* myActor, Faction const& aimFaction)
{
	if (!myActor) return nullptr;
	float closestDistanceSq = myActor->m_actorDef->m_sightRadius* myActor->m_actorDef->m_sightRadius;
	ActorHandle* nearestEnemyHandle = nullptr;

	Vec3 myDirection = myActor->m_orientation.GetForward_IFwd();

	for (Actor* otherActor : m_actors)
	{
		if (!otherActor) continue;
		if (otherActor == myActor) continue;

		if (otherActor->m_faction== aimFaction)
		{
			Vec3 otherPosition = otherActor->m_position;

			if(!IsPointInsideDirectedSector2D(Vec2(otherPosition.x, otherPosition.y),
					Vec2(myActor->m_position.x, myActor->m_position.y),
					Vec2(myDirection.x, myDirection.y),
					myActor->m_actorDef->m_cameraFOVDegrees, myActor->m_actorDef->m_sightRadius)) 
				continue;

			if (
				IsEyeSightBlock(myActor->m_position,(otherPosition-myActor->m_position).GetNormalized(), myActor->m_actorDef->m_sightRadius, myActor->m_actorHandle,
					GetDistanceSquared3D(myActor->m_position, otherPosition))
				)
				continue;
			
			float distSq = GetDistanceSquared2D(Vec2(myActor->m_position.x, myActor->m_position.y), 
				Vec2(otherPosition.x, otherPosition.y));

			if (distSq < closestDistanceSq)
			{
				closestDistanceSq = distSq;
				nearestEnemyHandle = otherActor->m_actorHandle;
			}
		}		
	}
	return nearestEnemyHandle;
}

ActorHandle* Map::GetMeleeTarget(Actor* myActor, Faction const& aimFaction, float meleeRadius, float meleeDegrees)
{
	if (!myActor) return nullptr;

	float meleeRangeSq = meleeRadius * meleeRadius;
	ActorHandle* nearestMeleeTargetHandle = nullptr;

	for (Actor* otherActor : m_actors)
	{
		if (!otherActor) continue;
		if (otherActor == myActor) continue;
		if (otherActor->m_faction == aimFaction)
		{
			Vec3 otherPosition = otherActor->m_position;

			if (!IsPointInsideDirectedSector2D(Vec2(otherPosition.x, otherPosition.y),
				Vec2(myActor->m_position.x, myActor->m_position.y),
				Vec2(myActor->m_orientation.GetForward_IFwd().x, myActor->m_orientation.GetForward_IFwd().y),
				meleeDegrees, meleeRadius)) continue;

			float distSq = GetDistanceSquared2D(Vec2(myActor->m_position.x, myActor->m_position.y),
				Vec2(otherPosition.x, otherPosition.y));

			if (distSq < meleeRangeSq)
			{
				meleeRangeSq = distSq;
				nearestMeleeTargetHandle = otherActor->m_actorHandle;
			}
		}
	}

	return nearestMeleeTargetHandle;
}

void Map::DebugPossessNext()
{
	m_curPlayerActorIndex++;
	m_curPlayerActorIndex %= m_actors.size();

	while (m_actors[m_curPlayerActorIndex] == nullptr || !m_actors[m_curPlayerActorIndex]->m_actorDef->m_canBePossessed)
	{
		m_curPlayerActorIndex++;
		m_curPlayerActorIndex %= m_actors.size();
	}

	if (m_actors[m_curPlayerActorIndex]->m_actorDef->m_canBePossessed)
	{
		m_game->m_playerController0->GetActor()->m_isVisible = true;
		m_game->m_playerController0->Possess(*m_actors[m_curPlayerActorIndex]->m_actorHandle,-1);
	}

}

void Map::SpawnPlantField(IntVec2 const& tileCoords)
{
	Vec3 position = Vec3((float)tileCoords.x + 0.5f, (float)tileCoords.y + 0.5f, 0.05f);
	FieldObject* newField = new FieldObject(position, this);
	m_field.push_back(newField);
}
