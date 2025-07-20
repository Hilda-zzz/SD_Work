#include "Player.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "PlayerBodyState.hpp"
#include "Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include "Inventory.hpp"
#include "InventoryItemDef.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/InventorySlotButton.hpp"
#include "Game/InventoryItem.hpp"

extern Window* g_theWindow;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;

static const std::map<char, PlayerTools> s_playerKeyCodeToolMap = {
	{'0', PlayerTools::NONE},
	{'1', PlayerTools::AXE},
	{'2', PlayerTools::HOE},
	{'3', PlayerTools::PICKAXE},
	{'4', PlayerTools::SHOVEL},
	{'5', PlayerTools::SICKLE},
	{'6', PlayerTools::WATER}
};

static const std::map<PlayerTools, std::string> s_playerToolConditionMap = {
	{PlayerTools::AXE, "usingAxe"},
	{PlayerTools::HOE, "usingHoe"},
	{PlayerTools::PICKAXE, "usingPickaxe"},
	{PlayerTools::SHOVEL, "usingShovel"},
	{PlayerTools::SICKLE, "usingSickle"},
	{PlayerTools::WATER, "usingWater"},
	{PlayerTools::SEEDS, "usingSeed"}
};


Player::Player(Game* game):m_game(game)
{
	m_physicsRadius = 0.25f;

	AddVertsForAABB2D(m_hoverGridCursorSquareVerts, AABB2(Vec2(-1.5f, -1.5f), Vec2(1.5f, 1.5f)), Rgba8::WHITE,Vec2(0.f,0.f),Vec2(0.5f,1.f));

	Initialize();
}

Player::~Player()
{
	for (auto& pair : m_idleDirectionalAnimDefs)
	{
		delete pair.second;
	}
	m_idleDirectionalAnimDefs.clear();

	for (auto& pair : m_walkDirectionalAnimDefs)
	{
		delete pair.second;
	}
	m_walkDirectionalAnimDefs.clear();

	for (auto& pair : m_spriteSheets)
	{
		delete pair.second;
	}
	m_spriteSheets.clear();

	delete m_inventory;
	m_inventory = nullptr;
}

void Player::Initialize()
{
	// Initialize textures
	m_idleTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Idle.png");
	m_walkTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Walk.png");
	m_runTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Run.png");
	m_axeTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Axe.png");
	m_hoeTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Hoe.png");
	m_pickaxeTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Pickaxe.png");
	m_shovelTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Shovel.png");
	m_sickleTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Sickle.png");
	m_waterTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Watering.png");
	m_seedTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Throwingitems.png");
	// Initialize anims
	InitializeAnims();

	//-------------------------------------------------------------------------------
	m_animConditions["isMoving"] = false;
	m_animConditions["isWalking"] = false;
	m_animConditions["isRunning"] = false;

	m_animConditions["holdTool"] = false;
	m_animConditions["usingAxe"] = false;
	m_animConditions["usingHoe"] = false;
	m_animConditions["usingPickaxe"] = false;
	m_animConditions["usingShovel"] = false;
	m_animConditions["usingSickle"] = false;
	m_animConditions["usingWater"] = false;
	m_animConditions["usingSeed"] = false;

	//-----------------------------------------------------
	m_inventory = new Inventory(this);
	AddStartingToolsToInventory();
	AddStartingSeedsToInventory();
}

void Player::InitializeAnims()
{
	InitializeStateAnimations("idle", m_idleTex, IntVec2(4, 3), PlayerBodyStates::IDLE,
		m_idleDirectionalAnimDefs,
		new PlayerBodyIdleState(&m_idleDirectionalAnimDefs), SpriteAnimPlaybackType::LOOP, 6);

	InitializeStateAnimations("walk", m_walkTex, IntVec2(6, 3), PlayerBodyStates::WALK,
		m_walkDirectionalAnimDefs,
		new PlayerBodyWalkState(&m_walkDirectionalAnimDefs), SpriteAnimPlaybackType::LOOP, 6);

	InitializeStateAnimations("run", m_runTex, IntVec2(6, 3), PlayerBodyStates::RUN,
		m_runDirectionalAnimDefs,
		new PlayerBodyRunState(&m_runDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 6);

	InitializeStateAnimations("axe", m_axeTex, IntVec2(6, 3), PlayerBodyStates::AXE,
		m_axeDirectionalAnimDefs,
		new PlayerBodyAxeState(&m_axeDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("hoe", m_hoeTex, IntVec2(6, 3), PlayerBodyStates::HOE,
		m_hoeDirectionalAnimDefs,
		new PlayerBodyHoeState(&m_hoeDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("pickaxe", m_pickaxeTex, IntVec2(6, 3), PlayerBodyStates::PICKAXE,
		m_pickaxeDirectionalAnimDefs,
		new PlayerBodyPickaxeState(&m_pickaxeDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("shovel", m_shovelTex, IntVec2(5, 3), PlayerBodyStates::SHOVEL,
		m_shovelDirectionalAnimDefs,
		new PlayerBodyShovelState(&m_shovelDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("sickle", m_sickleTex, IntVec2(6, 3), PlayerBodyStates::SICKLE,
		m_sickleDirectionalAnimDefs,
		new PlayerBodySickleState(&m_sickleDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("water", m_waterTex, IntVec2(8, 3), PlayerBodyStates::WATER,
		m_waterDirectionalAnimDefs,
		new PlayerBodyWaterState(&m_waterDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);

	InitializeStateAnimations("seed", m_seedTex, IntVec2(5, 3), PlayerBodyStates::SEED,
		m_seedDirectionalAnimDefs,
		new PlayerBodySeedState(&m_seedDirectionalAnimDefs), SpriteAnimPlaybackType::ONCE, 8);
}

void Player::InitializeStateAnimations(const std::string& stateName, Texture* texture, const IntVec2& gridSize,
	PlayerBodyStates stateEnum, std::map<Direction, SpriteAnimDefinition*>& directionAnimsMap, AnimState* state,
	SpriteAnimPlaybackType playbackType, int framePerSecond)
{
	m_spriteSheets[stateName] = new SpriteSheet(*texture, gridSize);

	directionAnimsMap[Direction::DOWN] = new SpriteAnimDefinition(*m_spriteSheets[stateName], 0, gridSize.x - 1, (float)framePerSecond, playbackType);
	directionAnimsMap[Direction::UP] = new SpriteAnimDefinition(*m_spriteSheets[stateName], gridSize.x, 2 * gridSize.x - 1, (float)framePerSecond, playbackType);
	directionAnimsMap[Direction::RIGHT] = new SpriteAnimDefinition(*m_spriteSheets[stateName], 2 * gridSize.x, 3 * gridSize.x - 1, (float)framePerSecond, playbackType);
	directionAnimsMap[Direction::LEFT] = new SpriteAnimDefinition(*m_spriteSheets[stateName], 2 * gridSize.x, 3 * gridSize.x - 1, (float)framePerSecond, playbackType);

	m_bodyStateMachine.RegisterState(
		stateEnum,
		state);
}

void Player::AddStartingToolsToInventory()
{
	InventoryItemDef* axeDef = InventoryItemDef::GetItemDefFromName("Axe");
	InventoryItemDef* hoeDef = InventoryItemDef::GetItemDefFromName("Hoe");
	InventoryItemDef* pickaxeDef = InventoryItemDef::GetItemDefFromName("Pickaxe");
	InventoryItemDef* shovelDef = InventoryItemDef::GetItemDefFromName("Shovel");
	InventoryItemDef* sickleDef = InventoryItemDef::GetItemDefFromName("Sickle");
	InventoryItemDef* wateringCanDef = InventoryItemDef::GetItemDefFromName("Watering Can");

	if (axeDef) {
		bool success = m_inventory->AddItem(axeDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Axe to inventory");
		}
	}

	if (hoeDef) {
		bool success = m_inventory->AddItem(hoeDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Hoe to inventory");
		}
	}

	if (pickaxeDef) {
		bool success = m_inventory->AddItem(pickaxeDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Pickaxe to inventory");
		}
	}

	if (shovelDef) {
		bool success = m_inventory->AddItem(shovelDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Shovel to inventory");
		}
	}

	if (sickleDef) {
		bool success = m_inventory->AddItem(sickleDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Sickle to inventory");
		}
	}

	if (wateringCanDef) {
		bool success = m_inventory->AddItem(wateringCanDef, 1);
		if (!success) {
			ERROR_AND_DIE("Failed to add Watering Can to inventory");
		}
	}
}

void Player::AddStartingSeedsToInventory()
{
	InventoryItemDef* strawberrySeedDef = InventoryItemDef::GetItemDefFromName("Strawberry_Seed");
	InventoryItemDef* greenOnionSeedDef = InventoryItemDef::GetItemDefFromName("GreenOnion_Seed");
	InventoryItemDef* potatoSeedDef = InventoryItemDef::GetItemDefFromName("Potato_Seed");

	if (strawberrySeedDef) {
		bool success = m_inventory->AddItem(strawberrySeedDef, 2);
		if (!success) {
			ERROR_AND_DIE("Failed to add strawberrySeedDef to inventory");
		}
	}

	if (greenOnionSeedDef) {
		bool success = m_inventory->AddItem(greenOnionSeedDef, 20);
		if (!success) {
			ERROR_AND_DIE("Failed to add  greenOnionSeedDef to inventory");
		}
	}

// 	if (potatoSeedDef) {
// 		bool success = m_inventory->AddItem(potatoSeedDef, 30);
// 		if (!success) {
// 			ERROR_AND_DIE("Failed to add potatoSeedDef to inventory");
// 		}
// 	}
}

void Player::Update(float deltaSeconds)
{
	HandleInput();

	UpdateAnimations(deltaSeconds);

	if (m_bodyStateMachine.GetCurrentStateEnum() == PlayerBodyStates::WALK)
	{
		UpdateMovement(deltaSeconds);
	}
	//m_gameplayCam.SetOrthographicView(m_position - Vec2(10.f, 5.f), m_position + Vec2(10.f, 5.f));
}

void Player::HandleInput()
{
	m_inputDirection = Vec2::ZERO;
	//-------------------Movement--------------
	if (g_theInput->IsKeyDown('W'))
	{
		m_inputDirection += Vec2(0.f, 1.f);
		//m_curDirection = Direction::UP;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_inputDirection += Vec2(0.f, -1.f);
		//m_curDirection = Direction::DOWN;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_inputDirection += Vec2(-1.f, 0.f);
		//m_curDirection = Direction::LEFT;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_inputDirection += Vec2(1.f, 0.f);
		//m_curDirection = Direction::RIGHT;
	}
	//-------------------CursorPos--------------
	UpdateToolAimGridPos();

	//---------------Tool-----------------------
// 	for (const auto& pair : s_playerKeyCodeToolMap)
// 	{
// 		if (g_theInput->WasKeyJustPressed(pair.first))
// 		{
// 			m_curTool = pair.second;
// 			break;
// 		}
// 	}
}

void Player::UpdateAnimations(float deltaTime)
{
	//-------------------------------CONDITIONS--------------------------------------
	m_animConditions["isMoving"] = (m_inputDirection.GetLengthSquared() > 0.01f);

	m_curTool != PlayerTools::NONE ? m_animConditions["holdTool"] = true : m_animConditions["holdTool"] = false;

	//reset of using tools
	for (const auto& pair : s_playerToolConditionMap)
	{
		m_animConditions[pair.second] = false;
	}
	// update using tool
	if (m_curTool != PlayerTools::NONE && g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		auto curToolCondition = s_playerToolConditionMap.find(m_curTool);
		if (curToolCondition != s_playerToolConditionMap.end())
		{
			m_animConditions[curToolCondition->second] = true;
			m_curDirection = GetDirectionFromIntVec2(m_curToolToPlayerDirection);
		}
		// change direction by using tool
	}
	//--------------------------------------------------------------------------------------------
	m_bodyStateMachine.Update(deltaTime, m_animConditions, m_curDirection);
	UpdateToolUsingResult();
	//--------------------------------------------------------------------------------------------
	// Render body
	SpriteDefinition const* curBodySpriteDef = nullptr;
	curBodySpriteDef = &m_bodyStateMachine.GetCurrentSprite();
	m_curBodyTex = &curBodySpriteDef->GetTexture();
	Vec2 spriteDimension = curBodySpriteDef->GetSpritesSheet().GetEachSpriteWidthHeight();
	Vec2 resizeDimension = spriteDimension / (float)RESOLUTION;
	m_bodyBox = AABB2(Vec2(-resizeDimension.x / 2.f, -resizeDimension.y / 2.f + 0.45f), Vec2(resizeDimension.x / 2.f, resizeDimension.y / 2.f + 0.45f));
	m_verts.clear();
	AABB2 curUV = curBodySpriteDef->GetUVs();
	m_curDirection == Direction::LEFT ? curUV = curBodySpriteDef->GetUVsReverse() : curUV = curBodySpriteDef->GetUVs();
	AddVertsForAABB2D(m_verts, m_bodyBox, Rgba8::WHITE,
		curUV.m_mins, curUV.m_maxs,0.f);
}

void Player::UpdateToolAimGridPos()
{
	m_curCursorHoverGridPos = GetCurrentCursorGridPos();
	bool cursorInRange = false;
	IntVec2 playerGridPos = m_game->m_curMap->GetTileCoordsFromPoint(m_position);

	for (int dy = -1; dy <= 1; dy++)
	{
		for (int dx = -1; dx <= 1; dx++) 
		{
			if (dx == 0 && dy == 0) continue; 

			IntVec2 adjacentPos = playerGridPos + IntVec2(dx, dy);
			if (adjacentPos == m_curCursorHoverGridPos) 
			{
				cursorInRange = true;
				break;
			}
		}
	}

	if (cursorInRange) 
	{
		m_curToolAimGridPos = m_curCursorHoverGridPos;
	}
	else
	{
		m_curToolAimGridPos = playerGridPos + GetCurDirectionIntVec2();
	}
	m_curToolToPlayerDirection = m_curToolAimGridPos - playerGridPos;
}

void Player::UpdateToolUsingResult()
{
	AnimState* curState=m_bodyStateMachine.GetCurrentState();
	std::string curStateName = curState->GetName();
	if (curStateName!=m_previousAnimStateName && m_toolStates.find(curStateName) != m_toolStates.end())
	{
		m_game->m_curMap->UsingToolTowardsGridPos(m_curToolAimGridPos, m_curTool,*m_curSelectedBtn);
	}
	m_previousAnimStateName = curStateName;
}

IntVec2 Player::GetCurrentCursorGridPos()
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePositionInGameCam= AABB2(Vec2(0.f, 0.f), 2.f*Vec2(g_halfGameCamDimensions.x, g_halfGameCamDimensions.y)).GetPointAtUV(mouseUV);
	Vec2 mousePosInWorldPlace = mousePositionInGameCam - Vec2(g_halfGameCamDimensions.x, g_halfGameCamDimensions.y) + m_position;

	return m_game->m_curMap->GetTileCoordsFromPoint(mousePosInWorldPlace);
}

IntVec2 Player::GetCurDirectionIntVec2()
{
	switch (m_curDirection) 
	{
	case Direction::DOWN:  return IntVec2(0, -1);
	case Direction::UP:    return IntVec2(0, 1);
	case Direction::LEFT:  return IntVec2(-1, 0);
	case Direction::RIGHT: return IntVec2(1, 0);
	default:               return IntVec2(0, -1); 
	}
}

Direction Player::GetDirectionFromIntVec2(IntVec2 const& directionVec)
{
	if (directionVec.x == 0 && directionVec.y == 0) {
		return Direction::DOWN;
	}

	if (abs(directionVec.x) > abs(directionVec.y)) {
		return (directionVec.x > 0) ? Direction::RIGHT : Direction::LEFT;
	}
	else {
		return (directionVec.y > 0) ? Direction::UP : Direction::DOWN;
	}
}

void Player::UpdateMovement(float deltaTime)
{
	m_inputDirection.Normalize();
	m_velocity = m_inputDirection;
	if (m_inputDirection.y > 0)
	{
		m_curDirection = Direction::UP;
	}
	else if(m_inputDirection.y < 0)
	{
		m_curDirection = Direction::DOWN;
	}
	else if (m_inputDirection.x < 0)
	{
		m_curDirection = Direction::LEFT;
	}
	else if (m_inputDirection.x > 0)
	{
		m_curDirection = Direction::RIGHT;
	}
	
	m_velocity *= m_speed;
	m_position += m_velocity * deltaTime;
}



void Player::Render() const
{
	float zValue = m_position.y + Z_OFFSET-0.2f;

//  //collide
// 	g_theRenderer->BindTexture(nullptr);
// 	g_theRenderer->SetModelConstants();
// 	DebugDrawCircle(m_physicsRadius, m_position, Rgba8::WHITE);

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(m_curBodyTex);
	Mat44 modelMatrix = Mat44::MakeTranslation3D(Vec3(m_position.x,m_position.y,zValue)); 
	g_theRenderer->SetModelConstants(modelMatrix);
	g_theRenderer->DrawVertexArray(m_verts);

	if (m_bodyStateMachine.GetCurrentState()->GetName() != "playerWalk" &&
		m_bodyStateMachine.GetCurrentState()->GetName() != "playerRun") {
		Mat44 hoverModelMatrix = Mat44::MakeTranslation3D(
			Vec3(
				(float)m_curToolAimGridPos.x + 0.5f, (float)m_curToolAimGridPos.y + 0.5f, zValue + 5.f)
		);
		g_theRenderer->SetModelConstants(hoverModelMatrix);
		Texture* hoverTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/CursorHoverSqaure.png");
		g_theRenderer->BindTexture(hoverTex);
		g_theRenderer->DrawVertexArray(m_hoverGridCursorSquareVerts);
	}
	
}
