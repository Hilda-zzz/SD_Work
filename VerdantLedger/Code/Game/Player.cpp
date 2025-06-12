#include "Player.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Window* g_theWindow;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;

Player::Player()
{
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_gameplayCam.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_gameplayCam.SetOrthographicView(m_position-Vec2(30.f,15.f), m_position + Vec2(30.f, 15.f));
	m_physicsRadius = 0.4;
}

Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	m_velocity = Vec2(0.f, 0.f);
	UpdateKeyboard(deltaSeconds);
	m_gameplayCam.SetOrthographicView(m_position - Vec2(30.f, 15.f), m_position + Vec2(30.f, 15.f));
}

void Player::Render() const
{
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	DebugDrawCircle(m_physicsRadius, m_position, Rgba8::WHITE);
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
