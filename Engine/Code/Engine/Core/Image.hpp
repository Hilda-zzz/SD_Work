#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
class Image
{
public:
	Image(char const* imageFilePath);
	std::string const& GetImageFilePath() const;
	IntVec2		GetDimensions() const;
	Rgba8			GetTexelColor(IntVec2 const& texelCoords) const;
	void			SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor);

private:
	std::string			m_imageFilePath;
	IntVec2			m_dimensions = IntVec2(0, 0);
	std::vector	<Rgba8>	m_rgbaTexels;  // or Rgba8* m_rgbaTexels = nullptr; if you prefer new[] and delete[]
};
