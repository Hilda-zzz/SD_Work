#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include "PlayerWeaponEnum.hpp"
#include <unordered_map>
#include <Engine/Renderer/SpriteAnimDefinition.hpp>
#include "Game/StateMachine.hpp"
#include "WeaponStrategy.hpp"
#include "WeaponState.hpp"
#include "PlayerAnimStateEnum.hpp"
class Texture;
class SpriteSheet;
class WeaponState;

class Player
{
public:
	Player() {};
	~Player();
	Player(Vec2 position);

	void Initialize();
	void InitializeAnims();

	void Update(float deltaTime);
	void Render() const;

	void UpdateKeyboard(float deltaTime);
	void UpdatMouse(float deltaTime);

	void ChangeWeapon(WeaponType weaponType);
	WeaponType GetCurWeaponType() const;

public:
	Vec2	m_position = Vec2(0.f, 0.f);
	Vec2	m_velocity = Vec2(0.f, 0.f);
	float   m_speed = 3.f;
	float	m_upperOrientation = 0.f;
	float	m_legOrientation = 0.f;

private:
	float	m_physicsRadius = 0.f;
	float	m_cosmeticRadius = 0.f;
	Mat44	m_transUpper = Mat44();
	Mat44	m_transLeg = Mat44();
	OBB2	m_upperBodyBox;
	OBB2	m_legBox;
	std::vector<Vertex_PCU> m_verts_upper;
	std::vector<Vertex_PCU> m_verts_leg;

	Texture* m_upperBodyTex = nullptr;
	Texture* m_legTex = nullptr;
	std::unordered_map<std::string, SpriteSheet*> m_spriteSheets;

	std::unordered_map<std::string, SpriteAnimDefinition*> m_upperAnimDefs;
	std::unordered_map<std::string, SpriteAnimDefinition*> m_legAnimDefs;
	std::unordered_map<std::string, SpriteAnimDefinition*> m_weaponAnimDefs;
	std::unordered_map<std::string, SpriteAnimDefinition*> m_specialAnimDefs;

	StateMachine<UpperBodyState>	m_upperStateMachine;
	StateMachine<LegState>			m_legStateMachine;

	WeaponStrategy* m_curWeaponStrategy = nullptr;
	std::unordered_map<WeaponType, std::unordered_map<int, WeaponState*>> m_weaponStatesMap;

	std::unordered_map<std::string, bool> m_animConditions;
};