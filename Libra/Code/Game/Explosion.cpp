#include "Game/Explosion.hpp"
#include "Engine/Core/Time.hpp"

Explosion::Explosion(EntityType type, EntityFaction faction):Entity(type,faction)
{
	m_startTime = GetCurrentTimeSeconds();
	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_explosionBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(m_size, m_size));
	m_verts.reserve(6);
	//AddVertsForOBB2D(m_verts, m_explosionBox, Rgba8(255, 255, 255, 255));
}

void Explosion::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_explosionBox.m_center = m_position;
	m_explosionBox.m_iBasisNormal= Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_explosionBox.m_halfDimensions = Vec2(m_size, m_size);
	float duration = (float)GetCurrentTimeSeconds() - (float)m_startTime;
	SpriteDefinition const& curExplosionAniSpriteDef = m_spriteAnimDef->GetSpriteDefAtTime(duration);
	m_curTex = &curExplosionAniSpriteDef.GetTexture();
	m_verts.clear();
	AddVertsForOBB2D(m_verts, m_explosionBox, Rgba8(255, 255, 255, 255), curExplosionAniSpriteDef.GetUVs().m_mins, curExplosionAniSpriteDef.GetUVs().m_maxs);

	if (m_spriteAnimDef->IsPlayOnceFinished(duration))
	{
		Die();
	}
}

void Explosion::Render() const
{
	g_theRenderer->BindTexture(m_curTex);
	g_theRenderer->DrawVertexArray(m_verts);
}
	

void Explosion::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
