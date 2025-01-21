#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, 
	int endSpriteIndex, float framesPerSecond, SpriteAnimPlaybackType playbackType)
	:m_spriteSheet(sheet),m_startSpriteIndex(startSpriteIndex),m_endSpriteIndex(endSpriteIndex),
	m_framesPerSecond(framesPerSecond),m_playbackType(playbackType)
{
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	//# todo change (int) to round down int in math utils
	
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
