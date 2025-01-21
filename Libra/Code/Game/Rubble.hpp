#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include <vector>
#include "Game/GameCommon.hpp"

class Rubble :public Entity
{
public:
	Rubble(EntityType type, EntityFaction faction);
	~Rubble();
	void Update(float deltaTime) override;
	void Render() const override;
	void Die() override;
	void DrawDebugMode() const override;
public:

private:

private:
	OBB2	m_bodyBox;
	std::vector<Vertex_PCU> m_verts_body;
	Texture* m_texRubble;
};