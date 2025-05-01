#include "ActorDefinition.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "GameCommon.hpp"
#include "SpriteAnimationGroupDefinition.hpp"

std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitions;
std::vector<ActorDefinition*> ActorDefinition::s_projectileActorDefinitions;


ActorDefinition::ActorDefinition(XmlElement const* actorDefElement)
{
	// Base attributes
	m_name = ParseXmlAttribute(actorDefElement, "name", m_name);

	std::string factionStr = ParseXmlAttribute(actorDefElement, "faction", "NEUTRAL");
	if (factionStr == "GOOD" || factionStr == "Marine")
		m_faction = Faction::GOOD;
	else if (factionStr == "EVIL")
		m_faction = Faction::EVIL;
	else
		m_faction = Faction::NEUTRAL;

	m_isVisible = ParseXmlAttribute(actorDefElement, "visible", m_isVisible);
	m_health = ParseXmlAttribute(actorDefElement, "health", m_health);
	m_corpseLifetime = ParseXmlAttribute(actorDefElement, "corpseLifetime", m_corpseLifetime);
	m_canBePossessed = ParseXmlAttribute(actorDefElement, "canBePossessed", m_canBePossessed);
	m_dieOnSpawn = ParseXmlAttribute(actorDefElement, "dieOnSpawn", m_canBePossessed);

	// Collision attributes
	XmlElement const* collisionElement = actorDefElement->FirstChildElement("Collision");
	if (collisionElement)
	{
		m_physicsRadius = ParseXmlAttribute(collisionElement, "radius", m_physicsRadius);
		m_physicsHeight = ParseXmlAttribute(collisionElement, "height", m_physicsHeight);
		m_collideWithWorld = ParseXmlAttribute(collisionElement, "collidesWithWorld", m_collideWithWorld);
		m_collideWithActors = ParseXmlAttribute(collisionElement, "collidesWithActors", m_collideWithActors);
		m_dieOnCollide = ParseXmlAttribute(collisionElement, "dieOnCollide", m_dieOnCollide);

		// Parse damage range if present
		std::string damageRangeStr = ParseXmlAttribute(collisionElement, "damageOnCollide", "");
		if (!damageRangeStr.empty())
		{
			std::vector<std::string> rangeParts = SplitStringOnDelimiter(damageRangeStr, '~');
			if (rangeParts.size() == 2)
			{
				float min = static_cast<float>(atof(rangeParts[0].c_str()));
				float max = static_cast<float>(atof(rangeParts[1].c_str()));
				m_damageOnCollide = FloatRange(min, max);
			}
		}

		m_impulseOnCollide = ParseXmlAttribute(collisionElement, "impulseOnCollide", m_impulseOnCollide);
	}

	// Physics attributes
	XmlElement const* physicsElement = actorDefElement->FirstChildElement("Physics");
	if (physicsElement)
	{
		m_simulated = ParseXmlAttribute(physicsElement, "simulated", m_simulated);
		m_flying = ParseXmlAttribute(physicsElement, "flying", m_flying);
		m_walkSpeed = ParseXmlAttribute(physicsElement, "walkSpeed", m_walkSpeed);
		m_runSpeed = ParseXmlAttribute(physicsElement, "runSpeed", m_runSpeed);
		m_drag = ParseXmlAttribute(physicsElement, "drag", m_drag);
		m_turnSpeed = ParseXmlAttribute(physicsElement, "turnSpeed", m_turnSpeed);
	}

	// Camera attributes
	XmlElement const* cameraElement = actorDefElement->FirstChildElement("Camera");
	if (cameraElement)
	{
		m_eyeHeight = ParseXmlAttribute(cameraElement, "eyeHeight", m_eyeHeight);
		m_cameraFOVDegrees = ParseXmlAttribute(cameraElement, "cameraFOV", m_cameraFOVDegrees);
	}

	// AI attributes
	XmlElement const* aiElement = actorDefElement->FirstChildElement("AI");
	if (aiElement)
	{
		m_aiEnabled = ParseXmlAttribute(aiElement, "aiEnabled", m_aiEnabled);
		m_sightRadius = ParseXmlAttribute(aiElement, "sightRadius", m_sightRadius);
		m_sightAngle = ParseXmlAttribute(aiElement, "sightAngle", m_sightAngle);
	}

	//weapons
	XmlElement const* inventoryElement = actorDefElement->FirstChildElement("Inventory");
	if (inventoryElement)
	{
		for (XmlElement const* weaponInfoElement = inventoryElement->FirstChildElement("Weapon");
			weaponInfoElement != nullptr;
			weaponInfoElement = weaponInfoElement->NextSiblingElement("Weapon"))
		{
			std::string weaponName = ParseXmlAttribute(weaponInfoElement, "name","");
			m_weaponName.push_back(weaponName);
		}
	}

	//visual
	XmlElement const* visualElement = actorDefElement->FirstChildElement("Visuals");
	if (visualElement)
	{
		m_spriteSize = ParseXmlAttribute(visualElement, "size", m_spriteSize);
		m_spritePivot = ParseXmlAttribute(visualElement, "pivot", m_spritePivot);

		std::string billboardTypeStr = ParseXmlAttribute(visualElement, "billboardType", "NONE");
		if (billboardTypeStr == "WorldUpFacing") {
			m_billboardType = BillboardType::WORLD_UP_FACING;
		}
		else if (billboardTypeStr == "WorldUpOpposing") {
			m_billboardType = BillboardType::WORLD_UP_OPPOSING;
		}
		else if (billboardTypeStr == "FullFacing") {
			m_billboardType = BillboardType::FULL_FACING;
		}
		else if (billboardTypeStr == "FullOpposing") {
			m_billboardType = BillboardType::FULL_OPPOSING;
		}
		else {
			m_billboardType = BillboardType::NONE;
		}

		std::string shaderPath = ParseXmlAttribute(visualElement, "shader", "");
		if (shaderPath!="Default") {
			m_shader = g_theRenderer->CreateShaderFromFile(shaderPath.c_str(),VertexType::VERTEX_PCUTBN);
		}

		m_isRenderLit = ParseXmlAttribute(visualElement, "renderLit", m_isRenderLit);
		m_isRenderRounded = ParseXmlAttribute(visualElement, "renderRounded", m_isRenderRounded);

		m_cellCount = ParseXmlAttribute(visualElement, "cellCount", m_cellCount);
		std::string spriteSheetPath = ParseXmlAttribute(visualElement, "spriteSheet", "");
		if (!spriteSheetPath.empty()) {
			Texture* curTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
			if (curTexture)
			{
				m_spriteSheet = new SpriteSheet(*curTexture, m_cellCount);
			}
		}

		//Animation Group
		for (XmlElement const* animationGroupInfoElement = visualElement->FirstChildElement("AnimationGroup");
			animationGroupInfoElement != nullptr;
			animationGroupInfoElement = animationGroupInfoElement->NextSiblingElement("AnimationGroup"))
		{
			if (m_spriteSheet)
			{
				SpriteAnimationGroupDefinition curAnimationGroupDef = SpriteAnimationGroupDefinition(animationGroupInfoElement, *m_spriteSheet);
				m_animGroupDef.push_back(curAnimationGroupDef);
			}	
		}
	}
	// Sounds
	XmlElement const* soundsElement = actorDefElement->FirstChildElement("Sounds");
	if (soundsElement)
	{
		for (XmlElement const* sound = soundsElement->FirstChildElement("Sound");
			sound != nullptr;
			sound = sound->NextSiblingElement("Sound"))
		{
			std::string soundName = ParseXmlAttribute(sound, "sound", "");
			if (soundName == "Hurt")
			{
				m_hurtSoundPath = ParseXmlAttribute(sound, "name", "");
			}
			if (soundName == "Death")
			{
				m_deathSoundPath = ParseXmlAttribute(sound, "name", "");
			}
		}
	}
}

ActorDefinition::~ActorDefinition()
{
	if (m_spriteSheet)
	{
		delete m_spriteSheet;
		m_spriteSheet = nullptr;
	}
}

void ActorDefinition::InitializeActorDefinitionFromFile()
{
	s_actorDefinitions.clear();
	XmlDocument actorDefsDoc;
	char const* filePath = "Data/Definitions/ActorDefinitions.xml";
	XmlResult result = actorDefsDoc.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required actors defs file \"%s\"", filePath));

	XmlElement* rootElement = actorDefsDoc.RootElement();
	if (!rootElement)
	{
		GUARANTEE_OR_DIE(rootElement,"Actor definitions file has no root element");
		return;
	}

	XmlElement* actorDefElement = rootElement->FirstChildElement();
	while (actorDefElement)
	{
		std::string elementName = actorDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ActorDefinition", Stringf("Root child element in %s was <%s>, must be <MapDefinition>!", filePath, elementName.c_str()));
		ActorDefinition* newMapDef =new ActorDefinition(actorDefElement);
		s_actorDefinitions.push_back(newMapDef);
		actorDefElement = actorDefElement->NextSiblingElement();
	}
}

void ActorDefinition::InitializeProjectileActorDefinitionFromFile()
{
	s_projectileActorDefinitions.clear();
	XmlDocument actorDefsDoc;
	char const* filePath = "Data/Definitions/ProjectileActorDefinitions.xml";
	XmlResult result = actorDefsDoc.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required actors defs file \"%s\"", filePath));

	XmlElement* rootElement = actorDefsDoc.RootElement();
	if (!rootElement)
	{
		GUARANTEE_OR_DIE(rootElement, "Actor definitions file has no root element");
		return;
	}

	XmlElement* actorDefElement = rootElement->FirstChildElement();
	while (actorDefElement)
	{

		std::string elementName = actorDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ActorDefinition", Stringf("Root child element in %s was <%s>, must be <MapDefinition>!", filePath, elementName.c_str()));
		ActorDefinition* newMapDef = new ActorDefinition(actorDefElement);
		s_projectileActorDefinitions.push_back(newMapDef);
		actorDefElement = actorDefElement->NextSiblingElement();
	}
}
