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

	void Update(float deltaSeconds);
	void Render() const;

	ObstacleType GetType() const { return m_type; }
	IntVec2 GetGridPos() const { return m_gridPos; }
	int GetHealth() const { return m_durability; }

	void SetTransparent(Rgba8 const& aimColor);

	bool TakeDamage(int damage) {
		m_durability -= damage;
		return m_durability <= 0;  
	}

	bool IsDestroyed() const { return m_durability <= 0; }
	void SetDurability(int durability) { m_durability = durability; }

private:
	ObstacleDefinition* m_obstacleDef = nullptr;
	std::vector<Vertex_PCU> m_verts;
	ObstacleType m_type;
	IntVec2 m_gridPos;
	int m_durability=10;

	SpriteSheet* m_spriteSheet=nullptr;
	int m_spriteIndex = 0;

	// Transparent trees
	Rgba8 m_curColor=Rgba8::WHITE;
	Rgba8 m_aimColor = Rgba8(255,255,255,255);
};