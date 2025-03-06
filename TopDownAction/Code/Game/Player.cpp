#include "Player.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

Player::~Player()
{
	delete m_walkAnimSheet;
	m_walkAnimSheet = nullptr;
}

Player::Player(Vec2 position)
	:m_position(position)
{
	Vec2 upperDir = Vec2::MakeFromPolarDegrees(m_upperOrientation);
	Vec2 lowerDir = Vec2::MakeFromPolarDegrees(m_legOrientation);
	m_verts_upper.reserve(6);
	m_verts_leg.reserve(6);
	m_upperBodyBox = OBB2(Vec2(0.f, 0.f), upperDir, Vec2(10.f, 10.f));
	m_legBox = OBB2(Vec2(0.f, 0.f), lowerDir, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_upper, m_upperBodyBox, Rgba8(255, 255, 255, 255));
	AddVertsForOBB2D(m_verts_leg, m_legBox, Rgba8(255, 255, 255, 255));
	m_upperBodyTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/sprPWalkUnarmed2_strip8.png");
	m_walkAnimSheet= new SpriteSheet(*m_upperBodyTex, IntVec2(8, 1));
}

void Player::Update(float deltaTime)
{
}

void Player::Render() const
{
	g_theRenderer->SetModelConstants(Mat44::MakeTranslation2D(m_position));
	g_theRenderer->BindTexture(m_upperBodyTex);
	g_theRenderer->DrawVertexArray(m_verts_upper);
}
