#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <unordered_map>
class SpriteSheet;

constexpr float SPRITE_SIZE_PIXELS = 64.0f;
constexpr float TEXTURE_SIZE_PIXELS = 512.0f;
constexpr float UV_SIZE_PER_SPRITE = SPRITE_SIZE_PIXELS / TEXTURE_SIZE_PIXELS;
constexpr int SPRITES_PER_ROW = static_cast<int>(TEXTURE_SIZE_PIXELS / SPRITE_SIZE_PIXELS); // 8

class BlockDefinition
{
public:
	BlockDefinition(XmlElement const* blockDefElement);
	~BlockDefinition();

	static void InitializeBlockDefinitionsFromFile();
	static void ShutdownBlockDefinitions();
	static std::vector<BlockDefinition> s_blockDefs;
	static std::unordered_map<std::string, uint8_t> s_nameToIndexMap;
	static SpriteSheet* s_blockSheet;

	std::string m_name="";
	bool m_isVisible = false;
	bool m_isSolid = false;
	bool m_isOpaque = false;
	IntVec2 m_topSpriteCoords = IntVec2::ZERO;
	IntVec2 m_bottomSpriteCoords = IntVec2::ZERO;
	IntVec2 m_sideSpriteCoords = IntVec2::ZERO;
	AABB2 m_topUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_sideUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_bottomUVs = AABB2::ZERO_TO_ONE;
};