#include "Game/Rubble.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
Rubble::Rubble(EntityType type, EntityFaction faction) :Entity(type, faction)
{
	m_cosmeticRadius = LEO_COS_RADIUS;
	m_physicsRadius = LEO_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;

	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_verts_body.reserve(6);
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.4f, 0.4f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255),g_decoSpriteSheet->GetSpriteUVs(1).m_mins, g_decoSpriteSheet->GetSpriteUVs(1).m_maxs);
	m_texRubble = &g_decoSpriteSheet->GetTexture();

	m_health = 5.f;
	m_totalHealth = m_health;
	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
}

Rubble::~Rubble()
{
}

void Rubble::Update(float deltaTime)
{
	UNUSED(deltaTime);
}

void Rubble::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texRubble);
	g_theRenderer->DrawVertexArray(temp_verts);
}

void Rubble::Die()
{
	g_theAudio->StartSound(g_theGame->m_enemyDead);
	m_isDead = true;
	m_isGarbage = true;
	m_map->SpawnExplosion(m_position, 0.7f, g_explosionAnimDef_Slow);
}

void Rubble::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
}
