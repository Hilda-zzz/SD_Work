#include "Player.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/LegIdleState.hpp"
#include "Game/LegWalkState.hpp"
#include "Game/PlayerUpperIdleState.hpp"
#include "Game/PlayerUpperWalkState.hpp"


Player::~Player()
{
	delete m_curWeaponStrategy;

	for (auto& weaponPair : m_weaponStatesMap) {
		for (auto& statePair : weaponPair.second) {
			delete statePair.second;
		}
	}

	for (auto& pair : m_upperAnimDefs) {
		delete pair.second;
	}
	for (auto& pair : m_legAnimDefs) {
		delete pair.second;
	}
	for (auto& pair : m_weaponAnimDefs) {
		delete pair.second;
	}
	for (auto& pair : m_specialAnimDefs) {
		delete pair.second;
	}

	for (auto& pair : m_spriteSheets) {
		delete pair.second;
	}
}

Player::Player(Vec2 position)
	:m_position(position)
{
	Initialize();
	
	Vec2 upperDir = Vec2::MakeFromPolarDegrees(m_upperOrientation);
	Vec2 lowerDir = Vec2::MakeFromPolarDegrees(m_legOrientation);
	m_verts_upper.reserve(6);
	m_verts_leg.reserve(6);
	m_upperBodyBox = OBB2(Vec2(0.f, 0.f), upperDir, Vec2(1.f, 1.f));
	m_legBox = OBB2(Vec2(0.f, 0.f), lowerDir, Vec2(1.f, 1.f));

// 	AABB2 upperUV=m_walkUpperAnimSheet->GetSpriteUVs(0);
// 	AddVertsForOBB2D(m_verts_upper, m_upperBodyBox, Rgba8(255, 255, 255, 255),upperUV.m_mins,upperUV.m_maxs);
// 	AABB2 legUV = m_walkLegAnimSheet->GetSpriteUVs(4);
// 	AddVertsForOBB2D(m_verts_leg, m_legBox, Rgba8(255, 255, 255, 255),legUV.m_mins,legUV.m_maxs);
}

void Player::Initialize()
{
	m_upperBodyTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/sprPWalkUnarmed2_strip8.png");
	m_legTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/sprLegs_strip16.png");

	InitializeAnims();

	m_animConditions["isMoving"] = false;
	m_animConditions["isRunning"] = false;
	m_animConditions["isFiring"] = false;
	m_animConditions["isReloading"] = false;
	m_animConditions["canFire"] = true;
	m_animConditions["needsReload"] = false;
	m_animConditions["isMelee"] = false;
	m_animConditions["isRolling"] = false;
	m_animConditions["isDodging"] = false;
	m_animConditions["isCrouching"] = false;
	m_animConditions["isSpecialAction"] = false;

	ChangeWeapon(WeaponType::PISTOL);
}

void Player::InitializeAnims()
{
	m_spriteSheets["upperIdle"] = new SpriteSheet(*m_upperBodyTex, IntVec2(8, 1));
	m_upperAnimDefs["idle"] = new SpriteAnimDefinition(*m_spriteSheets["upperIdle"], 0, 0, 5.0f, SpriteAnimPlaybackType::LOOP);
	m_upperAnimDefs["walk"] = new SpriteAnimDefinition(*m_spriteSheets["upperIdle"], 0, 7, 12.0f, SpriteAnimPlaybackType::LOOP);
	m_upperStateMachine.RegisterState(
		UpperBodyState::IDLE,
		new PlayerUpperIdleState(m_upperAnimDefs["idle"]));
	m_upperStateMachine.RegisterState(
		UpperBodyState::WALK,
		new PlayerUpperWalkState(m_upperAnimDefs["walk"]));


	m_spriteSheets["legIdle"] = new SpriteSheet(*m_legTex, IntVec2(16, 1));
	m_legAnimDefs["idle"] = new SpriteAnimDefinition(*m_spriteSheets["legIdle"], 0, 0, 10.0f, SpriteAnimPlaybackType::LOOP);
	m_legAnimDefs["walk"] = new SpriteAnimDefinition(*m_spriteSheets["legIdle"], 0, 15, 20.0f, SpriteAnimPlaybackType::LOOP);
	m_legStateMachine.RegisterState(
		LegState::IDLE,
		new LegIdleState(m_legAnimDefs["idle"])
	);
	m_legStateMachine.RegisterState(
		LegState::WALK,
		new LegWalkState(m_legAnimDefs["walk"])
	);
// 	// 创建武器动画定义
// 	m_weaponAnimDefs["pistol_idle"] = new SpriteAnimDefinition(*pistolIdleSheet, 0, 3, 5.0f, SpriteAnimPlaybackType::LOOP);
// 	// ...更多武器动画定义
// 
// 	// 创建特殊动作动画
// 	m_specialAnimDefs["roll"] = new SpriteAnimDefinition(*rollSheet, 0, 5, 12.0f, SpriteAnimPlaybackType::ONCE);
// 	// ...更多特殊动画定义
// 
// 	// 注册下半身状态
// 	m_lowerStateMachine.RegisterState(static_cast<int>(LowerBodyState::IDLE),
// 		new LowerIdleState(m_lowerAnimDefs["idle"]));
// 	// ...更多下半身状态
// 
// 	// 为每种武器创建武器状态
// 	m_weaponStatesMap[WeaponType::PISTOL] = std::unordered_map<int, WeaponState*>();
// 	// ...创建武器状态
// 
// 	// 创建特殊动作状态
// 	m_upperStateMachine.RegisterState(static_cast<int>(UpperBodyState::ROLL),
// 		new RollState(m_specialAnimDefs["roll"]));
}

void Player::Update(float deltaTime)
{
	m_velocity = Vec2(0.f, 0.f);
	m_transUpper = Mat44();
	m_transLeg = Mat44();
	UpdateKeyboard(deltaTime);
	UpdatMouse(deltaTime);
	Mat44 translationMat = Mat44::MakeTranslation2D(m_position);
	m_transUpper.Append(translationMat);
	m_transUpper.Append(Mat44::MakeZRotationDegrees(m_upperOrientation));
	m_transLeg.Append(translationMat);
	m_transLeg.Append(Mat44::MakeZRotationDegrees(m_legOrientation));

	m_animConditions["isMoving"] = m_velocity.GetLengthSquared() > 0.01f;
	m_animConditions["isWalking"] = m_velocity.GetLengthSquared() > 0.01f;
	m_upperStateMachine.Update(deltaTime, m_animConditions);
	m_legStateMachine.Update(deltaTime, m_animConditions);

	if (m_legStateMachine.GetCurrentStateEnum() != LegState::HIDDEN)
	{
		SpriteDefinition const& legSpriteDef = m_legStateMachine.GetCurrentSprite();

		m_verts_leg.clear();
		AddVertsForOBB2D(m_verts_leg, m_legBox, Rgba8::WHITE, legSpriteDef.GetUVs().m_mins, legSpriteDef.GetUVs().m_maxs);
		m_legTex =&legSpriteDef.GetTexture();
	}

	SpriteDefinition const& upperSpriteDef = m_upperStateMachine.GetCurrentSprite();
	m_verts_upper.clear();
	AddVertsForOBB2D(m_verts_upper, m_upperBodyBox, Rgba8::WHITE, upperSpriteDef.GetUVs().m_mins,upperSpriteDef.GetUVs().m_maxs);
	m_upperBodyTex = &upperSpriteDef.GetTexture();
}

void Player::Render() const
{
	g_theRenderer->SetModelConstants(m_transLeg);
	g_theRenderer->BindTexture(m_legTex);
	g_theRenderer->DrawVertexArray(m_verts_leg);

	g_theRenderer->SetModelConstants(m_transUpper);
	g_theRenderer->BindTexture(m_upperBodyTex);
	g_theRenderer->DrawVertexArray(m_verts_upper);
}

void Player::UpdateKeyboard(float deltaTime)
{
	if (g_theInput->IsKeyDown('W'))
	{
		m_velocity += Vec2(0.f, 1.f);
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_velocity += Vec2(-1.f, 0.f);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_velocity += Vec2(0.f, -1.f);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_velocity += Vec2(1.f, 0.f);
	}
	m_velocity.Normalize();
	m_velocity *= m_speed;
	m_position += m_velocity*deltaTime;
	if (m_velocity != Vec2::ZERO)
	{
		m_legOrientation = m_velocity.GetOrientationDegrees();
	}
}

void Player::UpdatMouse(float deltaTime)
{
	Vec2 mouseUVPos=g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePos = m_position-Vec2(GAME_SIZE_X/2.f,GAME_SIZE_Y/2.f) + Vec2(mouseUVPos.x * GAME_SIZE_X, mouseUVPos.y * GAME_SIZE_Y);
	Vec2 upperDirection = mousePos - m_position;
	m_upperOrientation = upperDirection.GetOrientationDegrees();
}

void Player::ChangeWeapon(WeaponType weaponType)
{
// 	if (m_weaponStatesMap.find(weaponType) == m_weaponStatesMap.end()) 
// 	{
// 		return;
// 	}
// 
// 	int currentUpperStateEnum = m_upperStateMachine.GetCurrentStateEnum();
// 
// 	auto& weaponStates = m_weaponStatesMap[weaponType];
// 	for (auto& statePair : weaponStates) {
// 		m_upperStateMachine.RegisterState(statePair.first, statePair.second);
// 	}
// 
// 	if (m_curWeaponStrategy) {
// 		delete m_curWeaponStrategy;
// 	}

// 	switch (weaponType) {
// 	case WeaponType::PISTOL:
// 		m_curWeaponStrategy = new PistolStrategy(
// 			m_weaponAnimDefs["pistol_idle"],
// 			m_weaponAnimDefs["pistol_fire"],
// 			m_weaponAnimDefs["pistol_reload"]
// 		);
// 		break;
// 	case WeaponType::SHOTGUN:
// 		m_curWeaponStrategy = new ShotgunStrategy(
// 			m_weaponAnimDefs["shotgun_idle"],
// 			m_weaponAnimDefs["shotgun_fire"],
// 			m_weaponAnimDefs["shotgun_reload"]
// 		);
// 		break;
// 	case WeaponType::RIFLE:
// 		m_curWeaponStrategy = new RifleStrategy(
// 			m_weaponAnimDefs["rifle_idle"],
// 			m_weaponAnimDefs["rifle_fire"],
// 			m_weaponAnimDefs["rifle_reload"]
// 		);
// 		break;
// 	default:
// 
// 		m_curWeaponStrategy = new PistolStrategy(
// 			m_weaponAnimDefs["pistol_idle"],
// 			m_weaponAnimDefs["pistol_fire"],
// 			m_weaponAnimDefs["pistol_reload"]
// 		);
// 		break;
// 	}
// 
// 	if (currentUpperStateEnum >= static_cast<int>(UpperBodyState::WEAPON_IDLE) &&
// 		currentUpperStateEnum <= static_cast<int>(UpperBodyState::WEAPON_RELOAD)) 
// 	{
// 		m_upperStateMachine.SetCurrentState(static_cast<int>(UpperBodyState::WEAPON_IDLE));
// 	}
}

WeaponType Player::GetCurWeaponType() const
{
	if (!m_curWeaponStrategy) 
	{
		return WeaponType::NONE;
	}

	return m_curWeaponStrategy->GetWeaponType();
}
