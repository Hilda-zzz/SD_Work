#pragma once
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

struct DirectAnim
{
	DirectAnim(Vec3 const& direct, SpriteAnimDefinition const& spriteAnimDef)
		:m_direction(direct),m_anim(spriteAnimDef){}

	Vec3 m_direction;
	SpriteAnimDefinition m_anim;
};

class SpriteAnimationGroupDefinition
{
public:
	SpriteAnimationGroupDefinition(XmlElement const* animGroupElement,SpriteSheet const& spriteSheet);
	~SpriteAnimationGroupDefinition();
	//static void InitializeAnimationGroupDefFromFile();
	//static std::vector<ActorDefinition> s_actorDefinitions;
	
	std::string m_name;
	bool m_scaleBySpeed = false;
	float m_secondPerFrame = 0.f;
	SpriteAnimPlaybackType m_playbackMode= SpriteAnimPlaybackType::ONCE;
	std::vector<DirectAnim> m_animSet;
};