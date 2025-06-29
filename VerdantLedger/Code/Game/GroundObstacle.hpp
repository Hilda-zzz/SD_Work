#pragma once
#include "TileTypesInGame.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
class Texture;
class SpriteSheet;
class ObstacleDefinition;

class GroundObstacle 
{
public:
	GroundObstacle(ObstacleType type, ObstacleDefinition* curDef,IntVec2 gridPos, int spriteIndex);
	~GroundObstacle();

	void Update();
	void Render() const;

	ObstacleType GetType() const { return m_type; }
	IntVec2 GetGridPos() const { return m_gridPos; }
	int GetHealth() const { return m_durability; }

	void SetTransparent(bool turnToTransparent);

	bool TakeDamage(int damage) {
		m_durability -= damage;
		return m_durability <= 0;  
	}

	bool IsDestroyed() const { return m_durability <= 0; }

private:
	ObstacleDefinition* m_obstacleDef = nullptr;
	std::vector<Vertex_PCU> m_verts;
	ObstacleType m_type;
	IntVec2 m_gridPos;
	int m_durability;

	SpriteSheet* m_spriteSheet=nullptr;
	int m_spriteIndex = 0;

	Timer m_transparentTimer;
	bool m_playerIsAbove = false;
	bool m_isTransparent = false;
	bool m_isMask = true;
	bool m_startTurnToTransparent = false;
	bool m_startTurnToMask = false;
	bool m_isTurningToTransparent = false;
	bool m_isTurningToOpaque = false;
};