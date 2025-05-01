#include "WeaponDefinition.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

std::vector<WeaponDefinition> WeaponDefinition::s_weaponDefinitions;

WeaponDefinition::WeaponDefinition(XmlElement const* weaponDefElement)
{
	m_name = ParseXmlAttribute(weaponDefElement, "name", "");
	m_refireTime = ParseXmlAttribute(weaponDefElement, "refireTime", m_refireTime);

	m_rayCount = ParseXmlAttribute(weaponDefElement, "rayCount", m_rayCount);
	m_rayCone = ParseXmlAttribute(weaponDefElement, "rayCone", m_rayCone);
	m_rayRange = ParseXmlAttribute(weaponDefElement, "rayRange", m_rayRange);

	std::string rayDamageStr = ParseXmlAttribute(weaponDefElement, "rayDamage", "");
	if (!rayDamageStr.empty())
	{
		if (rayDamageStr.find('~') != std::string::npos)
		{
			std::vector<std::string> rangeParts = SplitStringOnDelimiter(rayDamageStr, '~');
			if (rangeParts.size() == 2)
			{
				float min = static_cast<float>(atof(rangeParts[0].c_str()));
				float max = static_cast<float>(atof(rangeParts[1].c_str()));
				m_rayDamage = FloatRange(min, max);
			}
		}
		else
		{
			float damage = static_cast<float>(atof(rayDamageStr.c_str()));
			m_rayDamage = FloatRange(damage, damage);
		}
	}

	m_rayImpulse = ParseXmlAttribute(weaponDefElement, "rayImpulse", m_rayImpulse);

	m_projectileCount = ParseXmlAttribute(weaponDefElement, "projectileCount", m_projectileCount);
	m_projectileCone = ParseXmlAttribute(weaponDefElement, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXmlAttribute(weaponDefElement, "projectileSpeed", m_projectileSpeed);
	m_projectileActor = ParseXmlAttribute(weaponDefElement, "projectileActor", m_projectileActor);

	m_meleeCount = ParseXmlAttribute(weaponDefElement, "meleeCount", m_meleeCount);
	m_meleeRange = ParseXmlAttribute(weaponDefElement, "meleeRange", m_meleeRange);
	m_meleeArc = ParseXmlAttribute(weaponDefElement, "meleeArc", m_meleeArc);

	std::string m_meleeDamageStr = ParseXmlAttribute(weaponDefElement, "meleeDamage", "");
	if (!m_meleeDamageStr.empty())
	{
		if (m_meleeDamageStr.find('~') != std::string::npos)
		{
			std::vector<std::string> rangeParts = SplitStringOnDelimiter(m_meleeDamageStr, '~');
			if (rangeParts.size() == 2)
			{
				float min = static_cast<float>(atof(rangeParts[0].c_str()));
				float max = static_cast<float>(atof(rangeParts[1].c_str()));
				m_meleeDamage = FloatRange(min, max);
			}
		}
		else
		{
			float damage = static_cast<float>(atof(m_meleeDamageStr.c_str()));
			m_meleeDamage = FloatRange(damage, damage);
		}
	}

	m_meleeImpulse = ParseXmlAttribute(weaponDefElement, "meleeImpulse", m_meleeImpulse);

	//hud
	XmlElement const* hudElement = weaponDefElement->FirstChildElement("HUD");
	if (hudElement)
	{
		m_spriteSize = ParseXmlAttribute(hudElement, "spriteSize", m_spriteSize);
		m_spritePivot = ParseXmlAttribute(hudElement, "spritePivot", m_spritePivot);
		m_baseTexturePath = ParseXmlAttribute(hudElement, "baseTexture", m_baseTexturePath);
		m_reticleTexturePath = ParseXmlAttribute(hudElement, "reticleTexture", m_reticleTexturePath);
		m_reticleSize = ParseXmlAttribute(hudElement, "reticleSize", m_reticleSize);

		std::string shaderName = ParseXmlAttribute(hudElement, "shader", "Default");
		if (shaderName != "Default") 
		{
			m_shader = g_theRenderer->CreateShaderFromFile(shaderName.c_str(), VertexType::VERTEX_PCUTBN);
		}

		//Animation
		for (XmlElement const* animationElement = hudElement->FirstChildElement("Animation");
			animationElement != nullptr;
			animationElement = animationElement->NextSiblingElement("Animation"))
		{
			SpriteSheet* spriteSheet = nullptr;
			std::string name= ParseXmlAttribute(animationElement, "name", "");
			std::string spriteSheetPath = ParseXmlAttribute(animationElement, "spriteSheet", "");
			IntVec2 cellCount = ParseXmlAttribute(animationElement, "cellCount", IntVec2(1, 1));
			
			std::string weaponShaderName = ParseXmlAttribute(animationElement, "shader", "Default");
			if (weaponShaderName != "Default")
			{
				m_weaponShader = g_theRenderer->CreateShaderFromFile(shaderName.c_str(), VertexType::VERTEX_PCUTBN);
			}

			if (!spriteSheetPath.empty()) {
				Texture* curTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
				if (curTexture)
				{
					spriteSheet = new SpriteSheet(*curTexture, cellCount);
					m_animSheet[name] = spriteSheet;
				}
			}

			if (spriteSheet)
			{
				SpriteAnimDefinition* curAnimationGroupDef = new SpriteAnimDefinition(*animationElement, *spriteSheet);
				m_animDefs[name] = curAnimationGroupDef;
			}
		}
	}

	//Sounds
	XmlElement const* soundsElement = weaponDefElement->FirstChildElement("Sounds");
	if (soundsElement)
	{
		for (XmlElement const* sound = soundsElement->FirstChildElement("Sound");
			sound != nullptr;
			sound = sound->NextSiblingElement("Sound"))
		{
			std::string soundName = ParseXmlAttribute(sound, "sound", "");
			if (soundName == "Fire")
			{
				m_fireSoundPath = ParseXmlAttribute(sound, "name", "");
			}
		}
	}
}

WeaponDefinition::~WeaponDefinition()
{
	for (auto& pair : m_animSheet) 
	{
		delete pair.second;
		pair.second = nullptr;  
	}
	m_animSheet.clear();

	for (auto& pair : m_animDefs)
	{
		delete pair.second;
		pair.second = nullptr;
	}
	m_animDefs.clear();
}

void WeaponDefinition::InitializeWeaponDefinitionFromFile()
{
	s_weaponDefinitions.clear();
	s_weaponDefinitions.reserve(64);
	XmlDocument weaponDefsDoc;
	char const* filePath = "Data/Definitions/WeaponDefinitions.xml";
	XmlResult result = weaponDefsDoc.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required actors defs file \"%s\"", filePath));

	XmlElement* rootElement = weaponDefsDoc.RootElement();
	if (!rootElement)
	{
		GUARANTEE_OR_DIE(rootElement, "Actor definitions file has no root element");
		return;
	}

	XmlElement* weaponDefElement = rootElement->FirstChildElement();
	while (weaponDefElement)
	{
		std::string elementName = weaponDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "WeaponDefinition", Stringf("Root child element in %s was <%s>, must be <WeaponDefinition>!", filePath, elementName.c_str()));
		s_weaponDefinitions.emplace_back(weaponDefElement);
		weaponDefElement = weaponDefElement->NextSiblingElement();
	}
}
