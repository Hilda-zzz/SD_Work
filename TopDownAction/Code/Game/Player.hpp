#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
class Texture;
class SpriteSheet;

class Player
{
public:
	Player() {};
	~Player();
	Player(Vec2 position);
	void Update(float deltaTime);
	void Render() const;
public:
	Vec2 m_position = Vec2(0.f, 0.f);
	Vec2 m_velocity = Vec2(0.f, 0.f);
	float m_upperOrientation = 0.f;
	float m_legOrientation = 0.f;
private:
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	Texture* m_upperBodyTex = nullptr;
	Texture* m_legTex = nullptr;
	SpriteSheet* m_walkAnimSheet = nullptr;
	OBB2 m_upperBodyBox;
	OBB2 m_legBox;
	std::vector<Vertex_PCU> m_verts_upper;
	std::vector<Vertex_PCU> m_verts_leg;
};