#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/XmlUtils.hpp"
//------------------------------------------------------------------------------------------------
enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};
//------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex,
		float framesPerSecond, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);

	SpriteAnimDefinition(const XmlElement& element, SpriteSheet const& sheet);

	SpriteDefinition const& GetSpriteDefAtTime(float seconds) const; 

	bool LoadFromXmlElement(const XmlElement& element);

	bool IsPlayOnceFinished(float seconds);

	float GetAnimTime();

	void SetFramesPerSecond(float time);

	SpriteAnimPlaybackType GetPlaybackType();

private:
	SpriteSheet const&	m_spriteSheet;
	int					m_startSpriteIndex = -1;
	int					m_endSpriteIndex = -1;
	float				m_framesPerSecond = 1.f;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
};
