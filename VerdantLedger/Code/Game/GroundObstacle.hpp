#pragma once
#include "TileTypesInGame.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
class Texture;
class SpriteSheet;

class GroundObstacle 
{
public:
	GroundObstacle(ObstacleType type, IntVec2 gridPos);
	~GroundObstacle();
	void Render() const;

	ObstacleType GetType() const { return m_type; }
	IntVec2 GetGridPos() const { return m_gridPos; }
	int GetHealth() const { return m_health; }

	bool TakeDamage(int damage) {
		m_health -= damage;
		return m_health <= 0;  
	}

	bool IsDestroyed() const { return m_health <= 0; }

private:
	std::vector<Vertex_PCU> m_verts;
	ObstacleType m_type;
	IntVec2 m_gridPos;
	int m_health;
	Texture* m_texture=nullptr;
	SpriteSheet* m_spriteSheet=nullptr;
	int m_spriteIndex = 0;
};