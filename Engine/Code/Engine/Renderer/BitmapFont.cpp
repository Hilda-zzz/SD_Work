#include "BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture,IntVec2 const& layout)
	:m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension),
	m_fontGlyphsSpriteSheet(fontTexture,layout)
{
	
}
	
Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspectScale) const
{
	float width = cellHeight * cellAspectScale;
	Vec2 curMins = textMins;
	for (int i = 0; i < (int)text.length(); i++)
	{
		AABB2 aabb_thisChar = AABB2(curMins, curMins + Vec2(width, cellHeight));
		Vec2 uvMins, uvMaxs;
		m_fontGlyphsSpriteSheet.GetSpriteUVs(uvMins, uvMaxs,(int)text[i]);
		AddVertsForAABB2D(vertexArray, aabb_thisChar,tint, uvMins, uvMaxs);
		curMins += Vec2(width, 0.f);
	}
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspectScale)
{
	return cellHeight*cellAspectScale*text.length();
}

float BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, std::string const& text, 
	AABB2 const& box, float cellHeight, Rgba8 const& tint, float cellAspectScale, 
	Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw)
{
	//----------------SplitLine-----------------------------------------
	std::vector<std::string> eachLine;
	int start = 0;
	int end = (int)text.find('\n');
	while (end != std::string::npos) 
	{
		eachLine.push_back(text.substr(start, end - start));
		start = end + 1; 
		end = (int)text.find('\n', start);
	}
	eachLine.push_back(text.substr(start, text.length()-start));
	int lineCount = (int)eachLine.size();
	//-------------------------------------------------------------------
	Vec2 boxDimensions = box.GetDimensions();
	float width = cellHeight * cellAspectScale;
	//-------------------------------------------------------------------
	int maxLength = 0;
	switch (mode)
	{
	case SHRINK_TO_FIT:
		if (boxDimensions.y < lineCount * cellHeight)
		{
			cellHeight = boxDimensions.y / (float)lineCount;
			width = cellHeight * cellAspectScale;
		}
		for (std::string line : eachLine) 
		{
			if ((int)line.length() > maxLength)
				maxLength = (int)line.length();
		}
		if (maxLength * width > boxDimensions.x)
		{
			width = boxDimensions.x / (float)maxLength;
			cellHeight = width / cellAspectScale;
		}
		break;
	case OVERRUN:
		break;
	}
	int textCount = 0;
	float firstLineMinY = box.m_maxs.y - (1.f - alignment.y) * (boxDimensions.y - lineCount * cellHeight) - cellHeight;
	for (int i = 0; i < lineCount; i++)
	{
		int textCountInLine = (int)eachLine[i].size();
		float lineLen = textCountInLine * width;
		float minX = box.m_mins.x + alignment.x * (boxDimensions.x - lineLen);
		float minY = firstLineMinY - i * cellHeight;
		Vec2 curMins = Vec2(minX, minY);
		for (int j = 0; j < (int)eachLine[i].length(); j++)
		{
			if (textCount >= maxGlyphsToDraw) break;
			AABB2 aabb_thisChar = AABB2(curMins, curMins + Vec2(width, cellHeight));
			Vec2 uvMins, uvMaxs;
			m_fontGlyphsSpriteSheet.GetSpriteUVs(uvMins, uvMaxs, (int)eachLine[i][j]);
			AddVertsForAABB2D(vertexArray, aabb_thisChar, tint, uvMins, uvMaxs);
			curMins += Vec2(width, 0.f);
			textCount++;
		}
	}
	return width;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	UNUSED(glyphUnicode);
	return m_fontDefaultAspect;
}
