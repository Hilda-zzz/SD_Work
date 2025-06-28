#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "TileTypesInGame.hpp"
#include <map>

class SpriteSheet;
class Texture;

class ObstacleDefinition
{
public:
	ObstacleDefinition(XmlElement const* obstacleDefElement);
	~ObstacleDefinition();
	static void InitializeObstacleDefinitionFromFile();
	static void ShutdownObstacleDefinition();
	static std::map<ObstacleType,ObstacleDefinition*> s_obstacleDefinitions;
	static Texture* s_obstacleTexture;
	static SpriteSheet* s_obstacleSpriteSheet;
	//static ObstacleDefinition* GetObstacleDefFromType(ObstacleType type);

	ObstacleType m_obstacleType = ObstacleType::NONE;
	std::vector<IntVec2> m_spriteGridPos;
	int m_maxDurability = 0;
};