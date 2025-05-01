#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "../Core/XmlUtils.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, 
	int endSpriteIndex, float framesPerSecond, SpriteAnimPlaybackType playbackType)
	:m_spriteSheet(sheet),m_startSpriteIndex(startSpriteIndex),m_endSpriteIndex(endSpriteIndex),
	m_framesPerSecond(framesPerSecond),m_playbackType(playbackType)
{
}

SpriteAnimDefinition::SpriteAnimDefinition(const XmlElement& element, SpriteSheet const& sheet):m_spriteSheet(sheet)
{
	m_startSpriteIndex = ParseXmlAttribute(&element, "startFrame", m_startSpriteIndex);
	m_endSpriteIndex = ParseXmlAttribute(&element, "endFrame", m_endSpriteIndex);
	m_framesPerSecond = ParseXmlAttribute(&element, "framesPerSecond", m_framesPerSecond);
	float secondsPerFrame= ParseXmlAttribute(&element, "secondsPerFrame",-1.f);
	if (secondsPerFrame > 0.f)
	{
		m_framesPerSecond = 1.f / secondsPerFrame;
	}
	std::string playbackType = ParseXmlAttribute(&element, "playbackType", "");
	if (playbackType == "ONCE")
	{
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
	else if (playbackType == "LOOP")
	{
		m_playbackType = SpriteAnimPlaybackType::LOOP;
	}
	else if (playbackType == "PINGPONG")
	{
		m_playbackType = SpriteAnimPlaybackType::PINGPONG;
	}
	else
	{
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	//# todo change (int) to round down int in math utils
	//# todo check pingpong
	float totalFrames = seconds * m_framesPerSecond;
	int animLen = m_endSpriteIndex - m_startSpriteIndex + 1;
	int loopCount = (int)totalFrames / animLen;
	float leftFrames = totalFrames - animLen * loopCount;

	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::ONCE:
		if (loopCount > 0)
		{
			return m_spriteSheet.GetSpriteDef(m_endSpriteIndex);
		}
		else
		{
			return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + (int)leftFrames);
		}
	case SpriteAnimPlaybackType::LOOP: // 0 1 2 3 4; 0 1 2 3 4
		return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + (int)leftFrames);
	case SpriteAnimPlaybackType::PINGPONG: // 0 1 2 3; 4 3 2 1; 0 1 2 3 ...
		static int lastFrameIdx = 0;
		static int lastTotalFrames = 0;
		if (loopCount % 2 == 0)
		{
			if (lastFrameIdx == m_startSpriteIndex + (int)leftFrames) {
				if (RoundDownToInt(totalFrames) != lastTotalFrames)
				{
					//ERROR_AND_DIE("");
				}
			}
			lastFrameIdx = m_startSpriteIndex + (int)leftFrames;
			lastTotalFrames = RoundDownToInt(totalFrames);
			return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + (int)leftFrames);
		}
		else
		{
			if (lastFrameIdx == (int)((float)m_endSpriteIndex - leftFrames)) {
				if (RoundDownToInt(totalFrames) != lastTotalFrames)
				{
					//ERROR_AND_DIE("");
				}
			}
			lastFrameIdx = (int)((float)m_endSpriteIndex - leftFrames);
			lastTotalFrames = RoundDownToInt(totalFrames);
			return m_spriteSheet.GetSpriteDef((int)((float)m_endSpriteIndex -leftFrames));
		}
	}
	
	ERROR_AND_DIE("there is not such mode of sprite animation");
}

bool SpriteAnimDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_startSpriteIndex = ParseXmlAttribute(&element, "startSpriteIndex", m_startSpriteIndex);
	m_endSpriteIndex = ParseXmlAttribute(&element, "endSpriteIndex", m_endSpriteIndex);
	m_framesPerSecond = ParseXmlAttribute(&element, "framesPerSecond", m_framesPerSecond);
	std::string playbackType = ParseXmlAttribute(&element, "playbackType", "");
	if (playbackType == "ONCE")
	{
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
	else if (playbackType == "LOOP")
	{
		m_playbackType = SpriteAnimPlaybackType::LOOP;
	}
	else if (playbackType == "PINGPONG")
	{
		m_playbackType = SpriteAnimPlaybackType::PINGPONG;
	}
	return false;
}


bool SpriteAnimDefinition::IsPlayOnceFinished(float seconds)
{
	int animLen = m_endSpriteIndex - m_startSpriteIndex + 1;
	float playeOnceTime = animLen / m_framesPerSecond;
	if (seconds > playeOnceTime)
	{
		return true;
	}
	else
		return false;
}

float SpriteAnimDefinition::GetAnimTime()
{
	int animLen = m_endSpriteIndex - m_startSpriteIndex + 1;
	float playeOnceTime = animLen / m_framesPerSecond;
	return playeOnceTime;
}

void SpriteAnimDefinition::SetFramesPerSecond(float time)
{
	m_framesPerSecond = time;
}

SpriteAnimPlaybackType SpriteAnimDefinition::GetPlaybackType()
{
	return m_playbackType;
}


