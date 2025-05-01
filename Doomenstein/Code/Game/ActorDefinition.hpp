#pragma once
#include <string>
#include "GameCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "SpriteAnimationGroupDefinition.hpp"
class Shader;
class Texture;
class SpriteSheet;
enum class Faction
{
	NEUTRAL,
	GOOD,
	EVIL
};

class ActorDefinition
{
public:
	ActorDefinition(XmlElement const* actorDefElement);
	~ActorDefinition();
	static void InitializeActorDefinitionFromFile();
	static void InitializeProjectileActorDefinitionFromFile();
	static std::vector<ActorDefinition*> s_actorDefinitions;
	static std::vector<ActorDefinition*> s_projectileActorDefinitions;

	//----------base----------------------------------------
	std::string m_name = "";
	Faction m_faction = Faction::NEUTRAL;
	bool m_isVisible = false;
	int  m_health = 1;
	float m_corpseLifetime = 0.f;
	bool m_canBePossessed = false;
	bool m_dieOnSpawn = false;
	//----------Collision----------------------------------------
	float m_physicsRadius=0.f;
	float m_physicsHeight=0.f;
	bool  m_collideWithWorld = false;
	bool  m_collideWithActors = false;
	bool  m_dieOnCollide = false;
	FloatRange m_damageOnCollide = FloatRange(0.f, 0.f);
	float m_impulseOnCollide = 0.f;
	//----------Physics----------------------------------------
	bool m_simulated = false;
	bool m_flying = false;
	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_drag = 0.f;
	float m_turnSpeed = 0.f;
	//----------Camera----------------------------------------
	float m_eyeHeight = 0.f;
	float m_cameraFOVDegrees = 60.f;
	//----------AI----------------------------------------
	bool m_aiEnabled = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;
	//----------Weapon----------------------------------------
	std::vector<std::string> m_weaponName;
	//------------Visual-------------------------------------
	Vec2 m_spriteSize = Vec2(1.f, 1.f);
	Vec2 m_spritePivot = Vec2::ZERO;
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_isRenderLit = false;
	bool m_isRenderRounded = false;
	Shader* m_shader = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_cellCount = IntVec2(1, 1);
	std::vector<SpriteAnimationGroupDefinition> m_animGroupDef;
	//-------------Sounds-------------------------------------
	std::string m_hurtSoundPath;
	std::string m_deathSoundPath;
};