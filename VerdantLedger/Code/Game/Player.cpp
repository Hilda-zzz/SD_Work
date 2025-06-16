#include "Player.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "PlayerBodyState.hpp"
#include "Game.hpp"
#include "Engine/Core/VertexUtils.hpp"

extern Window* g_theWindow;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;

Player::Player()
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_gameplayCam.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_gameplayCam.SetOrthographicView(m_position-Vec2(10.f,5.f), m_position + Vec2(10.f, 5.f));
	m_physicsRadius = 0.25f;

	Initialize();
}

Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	m_velocity = Vec2(0.f, 0.f);
	UpdateKeyboard(deltaSeconds);
	m_gameplayCam.SetOrthographicView(m_position - Vec2(10.f, 5.f), m_position + Vec2(10.f, 5.f));

	UpdateAnimations(deltaSeconds);
}

void Player::Render() const
{
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	DebugDrawCircle(m_physicsRadius, m_position, Rgba8::WHITE);

	g_theRenderer->BindTexture(m_curBodyTex);
	g_theRenderer->SetModelConstants(Mat44::MakeTranslation2D(m_position));
	g_theRenderer->DrawVertexArray(m_verts);
}

void Player::Initialize()
{
	// Initialize textures
	m_idleTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Idle.png");
	m_walkTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/Character and Portrait - Tiny Asset Pack/Character/Pre-made/Alex/Walk.png");
	
	// Initialize anims
	InitializeAnims();

	m_animConditions["isMoving"] = false;
}

void Player::InitializeAnims()
{
	//m_spriteSheets["idle"] = new SpriteSheet(*m_idleTex, IntVec2(4, 3));
	//m_bodySpriteAnimDefs["idle"] = new SpriteAnimDefinition(*m_spriteSheets["idle"], 0, 3, 6.f, SpriteAnimPlaybackType::LOOP);	
	m_spriteSheets["idle"] = new SpriteSheet(*m_idleTex, IntVec2(4, 3));
	m_bodySpriteAnimDefs["idle"] = new SpriteAnimDefinition(*m_spriteSheets["idle"], 0, 3, 6.f, SpriteAnimPlaybackType::LOOP);
	m_bodyStateMachine.RegisterState(
		PlayerBodyStates::IDLE,
		new PlayerBodyIdleState(m_bodySpriteAnimDefs["idle"]));

	m_spriteSheets["walk"] = new SpriteSheet(*m_walkTex, IntVec2(6, 3));
	m_bodySpriteAnimDefs["walk"] = new SpriteAnimDefinition(*m_spriteSheets["walk"], 0, 5, 10.f, SpriteAnimPlaybackType::LOOP);
	m_bodyStateMachine.RegisterState(
		PlayerBodyStates::WALK,
		new PlayerBodyWalkState(m_bodySpriteAnimDefs["walk"]));
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
	m_position += m_velocity * deltaTime;
}

void Player::UpdateAnimations(float deltaTime)
{
	// Anim Update
	m_animConditions["isMoving"] = m_velocity.GetLengthSquared() > 0.01f;

	m_bodyStateMachine.Update(deltaTime, m_animConditions);

	SpriteDefinition const& curBodySpriteDef = m_bodyStateMachine.GetCurrentSprite();
	m_curBodyTex = &curBodySpriteDef.GetTexture();
	Vec2 spriteDimension = curBodySpriteDef.GetSpritesSheet().GetEachSpriteWidthHeight();
	Vec2 resizeDimension = spriteDimension / (float)RESOLUTION;
	m_bodyBox = AABB2(Vec2(-resizeDimension.x/2.f, -resizeDimension.y / 2.f+0.3f), Vec2(resizeDimension.x / 2.f, resizeDimension.y / 2.f+0.3f));
	m_verts.clear();
	AddVertsForAABB2D(m_verts, m_bodyBox, Rgba8::WHITE,
		curBodySpriteDef.GetUVs().m_mins, curBodySpriteDef.GetUVs().m_maxs);
}
