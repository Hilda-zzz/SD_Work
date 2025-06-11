#define STB_IMAGE_IMPLEMENTATION 
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


Image::Image(char const* imageFilePath):m_imageFilePath(imageFilePath)
{
	int originalChannels;
	int desiredChannels = 4;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* pixelData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &originalChannels, desiredChannels);
	if (pixelData == nullptr)
	{
		ERROR_AND_DIE("can not find the image");
		return;
	}
	m_rgbaTexels.resize(m_dimensions.x*m_dimensions.y);

	for (int i = 0; i < m_dimensions.x; i++)
	{
		for (int j = 0; j < m_dimensions.y; j++)
		{
			int index = (m_dimensions.x * j + i) * desiredChannels;
			Rgba8 curColor;
			curColor.r = pixelData[index];
			curColor.g = pixelData[index + 1];
			curColor.b = pixelData[index + 2];
			curColor.a = pixelData[index + 3];
			SetTexelColor(IntVec2(i, j), curColor);
		}
	}

	stbi_image_free(pixelData);
}

Image::Image(std::vector<AtlasMember>& atlasMembers)
{
}

Image::Image(IntVec2 size, Rgba8 color)
{
	m_dimensions.x = size.x;
	m_dimensions.y = size.y;
	m_rgbaTexels.reserve(m_dimensions.x * m_dimensions.y);
	for (int i = 0; i < m_dimensions.x * m_dimensions.y;i++)
	{
		m_rgbaTexels.push_back(color);
	}
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	int index = m_dimensions.x * texelCoords.y + texelCoords.x;
	return m_rgbaTexels[index];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	int index = m_dimensions.x * texelCoords.y + texelCoords.x;
	m_rgbaTexels[index] = newColor;
}

const void* Image::GetRawData() const
{
	return m_rgbaTexels.data();
}
