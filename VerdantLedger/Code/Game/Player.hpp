#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Player
{
public:
	Player();
	~Player();

private:

public:
	Vec2	m_position = Vec2(0.f, 0.f);
	float	m_orientation = 0.f;
	Vec2	m_velocity = Vec2(0.f, 0.f);
	float   m_speed = 3.f;
private:
	float	m_physicsRadius = 0.f;
	float	m_cosmeticRadius = 0.f;
	std::vector<Vertex_PCU> m_verts;
};