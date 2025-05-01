#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);
	~SpriteSheet(){}
	Texture& GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteIndex) const;
	float GetEachSpriteAspect() const;
public:
	IntVec2 const m_gridLayout;
protected:
	Texture& m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};