#include "RuledTileset.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

extern Renderer* g_theRenderer;

RuledTileset::RuledTileset(XmlElement* rootElement)
{
	m_ruledTilesetName = ParseXmlAttribute(rootElement, "name", "");
	IntVec2 spriteSheetLayout= ParseXmlAttribute(rootElement, "spriteLayout", IntVec2(0,0));
	std::string texturePath = ParseXmlAttribute(rootElement, "texturePath", "");
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
	m_spriteSheet = new SpriteSheet(*m_texture, spriteSheetLayout);

	XmlElement* rulesElement = rootElement->FirstChildElement("TileRules");
	if (rulesElement) 
	{
		for (XmlElement* ruleElement = rulesElement->FirstChildElement("TileRule");
			ruleElement != nullptr;
			ruleElement = ruleElement->NextSiblingElement("TileRule"))
		{
			std::string maskStr = ParseXmlAttribute(ruleElement, "neighborMask", "");
			if (maskStr.empty()) continue;

			uint8_t neighborMask = ParseBinaryMask(maskStr);

			XmlElement* gridPosElement = ruleElement->FirstChildElement("SpriteGridPos");
			if (gridPosElement)
			{
				IntVec2 spriteGridPos = ParseXmlAttribute(gridPosElement, "gridPos", IntVec2(0, 0));
				m_rules.emplace_back(neighborMask, spriteGridPos);
			}
		}
	}
}

RuledTileset::~RuledTileset()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

uint8_t RuledTileset::ParseBinaryMask(std::string const& binaryStr)
{
	if (binaryStr.empty())
	{
		return 0;
	}

	if (binaryStr.size() > 2 && binaryStr.substr(0, 2) == "0b")
	{
		std::string binaryPart = binaryStr.substr(2);
		return static_cast<uint8_t>(std::stoi(binaryPart, nullptr, 2));
	}
	else
	{
		return static_cast<uint8_t>(std::stoi(binaryStr));
	}
}

AABB2 RuledTileset::GetUvFromMask(uint8_t neighborMask)
{
	for (int i = 0; i < (int)m_rules.size(); i++)
	{
		if (neighborMask == m_rules[i].m_neighborMask)
		{
			int spriteIndex= m_spriteSheet->GetSpriteIndexFromGridPos(m_rules[i].m_spriteGridPos);
			AABB2 uv = m_spriteSheet->GetSpriteUVs(spriteIndex);
			return uv;
		}
	}
	return AABB2();
}
