#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "AnimStateEnum.hpp"
#include "StateMachine.hpp"

class Player
{
public:
	Player();
	~Player();

	void Update(float deltaSeconds);
	void Render() const;

private:
	void Initialize();
	void InitializeAnims();
	void UpdateKeyboard(float deltaTime);
	void UpdateAnimations(float deltaTime);

public:
	Vec2	m_position = Vec2(0.f, 0.f);
	float	m_orientation = 0.f;
	Vec2	m_velocity = Vec2(0.f, 0.f);
	float   m_speed = 5.f;
	Camera  m_gameplayCam;
	float	m_physicsRadius = 0.f;
private:
	
	float	m_cosmeticRadius = 0.f;
	std::vector<Vertex_PCU> m_verts;
	
	// Animation State Machine
	AABB2 m_bodyBox;
	Texture* m_curBodyTex = nullptr;

	Texture* m_idleTex = nullptr;
	Texture* m_walkTex = nullptr;

	StateMachine<PlayerBodyStates>	      m_bodyStateMachine;
	std::unordered_map<std::string, bool> m_animConditions;

	std::unordered_map<std::string, SpriteSheet*> m_spriteSheets;
	std::unordered_map<std::string, SpriteAnimDefinition*> m_bodySpriteAnimDefs;

};