#include "Engine/Renderer/SpriteSheet.hpp"

SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout):m_texture(texture),m_gridLayout(simpleGridLayout)
{
	m_spriteDefs.reserve(m_gridLayout.x * m_gridLayout.y);
	for (int i = 0; i < m_gridLayout.x * m_gridLayout.y; i++)
	{
		Vec2 thisMin, thisMax;
		GetSpriteUVs(thisMin, thisMax, i);
		SpriteDefinition thisSpriteDef = SpriteDefinition(*this, i,thisMin,thisMax);
		m_spriteDefs.push_back(thisSpriteDef);
	}
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return m_gridLayout.x*m_gridLayout.y;
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	int index_y = spriteIndex / m_gridLayout.x;
	int index_x = spriteIndex % m_gridLayout.x;
	float nudgeX = 1.f / (m_texture.GetDimensions().x * 128.f);
	float nudgeY = 1.f / (m_texture.GetDimensions().y * 128.f);
	out_uvAtMins.x = index_x * (1.0f / m_gridLayout.x)+ nudgeX;
	out_uvAtMaxs.x = out_uvAtMins.x + (1.0f / m_gridLayout.x)- nudgeX;
	out_uvAtMaxs.y = (m_gridLayout.y - index_y)* (1.0f / m_gridLayout.y)- nudgeY;
	out_uvAtMins.y= out_uvAtMaxs.y- (1.0f / m_gridLayout.y)+ nudgeY;
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	AABB2 spriteBox;
	GetSpriteUVs(spriteBox.m_mins, spriteBox.m_maxs, spriteIndex);
	return spriteBox;
}

float SpriteSheet::GetEachSpriteAspect() const
{
	IntVec2 texDimension = m_texture.GetDimensions();
	float width = (float)texDimension.x / (float)m_gridLayout.x;
	float height = (float)texDimension.y / (float)m_gridLayout.y;
	return width/height;
}

Vec2 SpriteSheet::GetEachSpriteWidthHeight() const
{
	IntVec2 texDimension = m_texture.GetDimensions();
	float width = (float)texDimension.x / (float)m_gridLayout.x;
	float height = (float)texDimension.y / (float)m_gridLayout.y;
	return Vec2(width, height);
}
