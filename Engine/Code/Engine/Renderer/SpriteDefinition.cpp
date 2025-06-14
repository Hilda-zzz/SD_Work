#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
SpriteDefinition::SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, 
	Vec2 const& uvAtMins, Vec2 const& uvAtMaxs):m_spriteSheet(spriteSheet),m_spriteIndex(spriteIndex),
	m_uvAtMins(uvAtMins),m_uvAtMaxs(uvAtMaxs)
{
}

void SpriteDefinition::GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}

AABB2 SpriteDefinition::GetUVs() const
{
	return AABB2(m_uvAtMins,m_uvAtMaxs);
}

AABB2 SpriteDefinition::GetUVsReverse() const
{
	Vec2 reverseMin = Vec2(m_uvAtMaxs.x, m_uvAtMins.y);
	Vec2 reverseMax = Vec2(m_uvAtMins.x, m_uvAtMaxs.y);
	return AABB2(reverseMin,reverseMax);
}

SpriteSheet const& SpriteDefinition::GetSpritesSheet() const
{
	return m_spriteSheet;
}

Texture& SpriteDefinition::GetTexture() const
{
	// TODO: insert return statement here
	//return nullptr;
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	return m_spriteSheet.GetEachSpriteAspect();
}
