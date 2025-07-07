#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"

class SpriteSheet;
class Texture;

struct TileRule 
{
	TileRule(uint8_t mask, IntVec2 spriteGridPos) : m_neighborMask(mask), m_spriteGridPos(spriteGridPos) {}

	// up(0), right up(1), right(2), right down(3), down(4), left down(5), left(6), left up(7)
	uint8_t m_neighborMask;
	IntVec2 m_spriteGridPos;
};

class RuledTileset
{
public:
	RuledTileset(XmlElement* rootElement);
	~RuledTileset();

	std::string GetName() { return m_ruledTilesetName; }
	Texture* GetTexture() { return m_texture; }
	uint8_t ParseBinaryMask(std::string const& binaryStr);
	AABB2 GetUvFromMask(uint8_t neighborMask);
public:

private:
	std::string m_ruledTilesetName="";
	Texture* m_texture=nullptr;
	SpriteSheet* m_spriteSheet=nullptr;

	std::vector<TileRule> m_rules;
};