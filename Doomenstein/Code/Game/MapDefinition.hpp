#pragma once
#include <string>
#include "GameCommon.hpp"
#include "Engine/Core/Image.hpp"
class Shader;
class Texture;

class MapDefinition
{
public:
	MapDefinition(std::string name, std::string imageName,Shader* shader, Texture* tex, IntVec2 cellCount)
		:m_name(name), m_mapImageName(imageName), m_shader(shader), m_spriteSheetTexture(tex),m_spriteSheetCellCount(cellCount)
	{
	}
	MapDefinition(XmlElement const* mapDefElement);
	~MapDefinition();
	static void InitializeMapDefinitionFromFile();

	static std::vector<MapDefinition> s_mapDefinitions;
	std::string m_name = "";
	std::string m_mapImageName = "";
	Shader* m_shader=nullptr;
	Texture* m_spriteSheetTexture= nullptr;
	IntVec2 m_spriteSheetCellCount=IntVec2::ZERO;
};