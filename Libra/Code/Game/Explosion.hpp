#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
class Explosion:public Entity
{
public:
	Explosion(EntityType type, EntityFaction faction);
	~Explosion() {}
	void Update(float deltaTime) override;
	void Render() const override;
	void Die() override;
public:
	float m_size = 0.5f;
	SpriteAnimDefinition* m_spriteAnimDef = nullptr;
private:
	std::vector<Vertex_PCU> m_verts;
	OBB2	m_explosionBox;
	int m_bounceTime = 0;
	double m_startTime = 0.0;
	
	Texture* m_curTex=nullptr;
	SpriteDefinition* m_curSpriteDef = nullptr;
};