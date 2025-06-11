#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"

class Player
{
public:
	Player();
	~Player();

	void Update(float deltaSeconds);
	void Render() const;

private:
	void UpdateKeyboard(float deltaTime);

public:
	Vec2	m_position = Vec2(0.f, 0.f);
	float	m_orientation = 0.f;
	Vec2	m_velocity = Vec2(0.f, 0.f);
	float   m_speed = 20.f;
	Camera  m_gameplayCam;

private:
	float	m_physicsRadius = 0.f;
	float	m_cosmeticRadius = 0.f;
	std::vector<Vertex_PCU> m_verts;
	
};