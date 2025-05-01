#include "SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

SpriteAnimationGroupDefinition::SpriteAnimationGroupDefinition(XmlElement const* animGroupElement,SpriteSheet const& spriteSheet)
{
	//XmlElement const* animGroupElement = actorDefElement->FirstChildElement("AnimationGroup");
	if (animGroupElement)
	{
		m_name= ParseXmlAttribute(animGroupElement, "name", "");
		m_scaleBySpeed = ParseXmlAttribute(animGroupElement, "scaleBySpeed", m_scaleBySpeed);
		m_secondPerFrame = ParseXmlAttribute(animGroupElement, "secondsPerFrame", m_secondPerFrame);

		std::string playbackType = ParseXmlAttribute(animGroupElement, "playbackType", "");
		if (playbackType == "ONCE")
		{
			m_playbackMode = SpriteAnimPlaybackType::ONCE;
		}
		else if (playbackType == "LOOP")
		{
			m_playbackMode = SpriteAnimPlaybackType::LOOP;
		}
		else if (playbackType == "PINGPONG")
		{
			m_playbackMode = SpriteAnimPlaybackType::PINGPONG;
		}

		for (XmlElement const* directAnimElement = animGroupElement->FirstChildElement("Direction");
			directAnimElement != nullptr;
			directAnimElement = directAnimElement->NextSiblingElement("Direction"))
		{
			Vec3 direction= ParseXmlAttribute(directAnimElement, "vector", Vec3(0.f,0.f,0.f));
			direction.Normalized();

			XmlElement const* animElement = directAnimElement->FirstChildElement("Animation");
			//int startFrame= ParseXmlAttribute(animElement, "startFrame", 0);
			//int endFrame = ParseXmlAttribute(animElement, "endFrame", 0);
			SpriteAnimDefinition curAnimDef = SpriteAnimDefinition(*animElement,spriteSheet);
			curAnimDef.SetFramesPerSecond(1.f/m_secondPerFrame);
			DirectAnim curDirectAnim=DirectAnim(direction,curAnimDef);
			m_animSet.push_back(curDirectAnim);
		}
	}
}

SpriteAnimationGroupDefinition::~SpriteAnimationGroupDefinition()
{
}
