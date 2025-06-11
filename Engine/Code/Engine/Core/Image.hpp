#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/TextureAtlas.hpp"

//struct AtlasMember;

class Image
{
	friend class Renderer;
public:
	Image(char const* imageFilePath);
	Image(std::vector<AtlasMember>& atlasMembers);
	Image(IntVec2 size, Rgba8 color);
public:
	std::string const&	GetImageFilePath() const;
	IntVec2				GetDimensions() const;
	Rgba8				GetTexelColor(IntVec2 const& texelCoords) const;
	void				SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor);
	const void*			GetRawData() const;
private:
	std::string			m_imageFilePath;
	IntVec2				m_dimensions = IntVec2(0, 0);
	std::vector	<Rgba8>	m_rgbaTexels;  
};
