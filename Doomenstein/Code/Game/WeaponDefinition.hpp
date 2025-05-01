#pragma once
#include <string>
#include "GameCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "SpriteAnimationGroupDefinition.hpp"
class Shader;
class Texture;


class WeaponDefinition
{
public:
	WeaponDefinition(XmlElement const* weaponDefElement);
	~WeaponDefinition();
	static void InitializeWeaponDefinitionFromFile();
	static std::vector<WeaponDefinition> s_weaponDefinitions;

	std::string m_name = "";
	float m_refireTime = 0.f;
	int m_rayCount = 0;
	float m_rayCone = 0.f;
	float m_rayRange = 0.f;
	FloatRange m_rayDamage = FloatRange(0.f, 0.f);
	float m_rayImpulse = 0.f;
	int m_projectileCount = 0;
	float m_projectileCone = 0.f;
	float m_projectileSpeed = 0.f;
	std::string m_projectileActor = "";
	int m_meleeCount = 0;
	float m_meleeRange = 0.f;
	float m_meleeArc = 0.f;
	FloatRange m_meleeDamage= FloatRange(0.f, 0.f);
	float m_meleeImpulse = 0.f;

	//HUD
	Shader* m_shader=nullptr;   
	Shader* m_weaponShader = nullptr;
	std::string m_baseTexturePath= "Data/Images/Hud_Base.png";           
	std::string m_reticleTexturePath="Data/Images/Reticle.png";       
	IntVec2 m_reticleSize=IntVec2(16,16);                 
	IntVec2 m_spriteSize = IntVec2(256, 256);                
	Vec2 m_spritePivot=Vec2(0.5f, 0.0f);                 
	std::map<std::string, SpriteSheet*> m_animSheet;
	std::map<std::string, SpriteAnimDefinition*> m_animDefs;

	//Sounds
	std::string m_fireSoundPath;

	//Anim

};