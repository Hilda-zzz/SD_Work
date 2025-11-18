#include "HerringboneTile.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <algorithm>

// ============================================
// HerringboneTile 实现
// ============================================

HerringboneTile::HerringboneTile()
	: m_tileID(-1)
	, m_orientation(HbTileOrientation::HORIZONTAL)
	, m_contentSize(0, 0)
{
}

HerringboneTile::~HerringboneTile() 
{
}

void HerringboneTile::Initialize(HbTileOrientation orientation, const IntVec2& contentSize) 
{
	m_orientation = orientation;
	m_contentSize = contentSize;
	m_contentPixels.resize(contentSize.x * contentSize.y, Rgba8::BLACK);
}

void HerringboneTile::SetEdgeConstraint(int edgeIndex, const HbEdgeConstraint& constraint) 
{
	if (edgeIndex >= 0 && edgeIndex < 6) 
	{
		m_edges[edgeIndex] = constraint;
	}
}

HbEdgeConstraint HerringboneTile::GetEdgeConstraint(int edgeIndex) const 
{
	if (edgeIndex >= 0 && edgeIndex < 6) 
	{
		return m_edges[edgeIndex];
	}
	return HbEdgeConstraint();
}

void HerringboneTile::SetContentPixel(int x, int y, const Rgba8& color) 
{
	if (x >= 0 && x < m_contentSize.x && y >= 0 && y < m_contentSize.y) 
	{
		m_contentPixels[y * m_contentSize.x + x] = color;
	}
}

Rgba8 HerringboneTile::GetContentPixel(int x, int y) const 
{
	if (x >= 0 && x < m_contentSize.x && y >= 0 && y < m_contentSize.y) 
	{
		return m_contentPixels[y * m_contentSize.x + x];
	}
	return Rgba8::BLACK;
}

void HerringboneTile::SetContentData(const std::vector<Rgba8>& pixels) 
{
	m_contentPixels = pixels;
}

bool HerringboneTile::MatchesConstraints(const HbEdgeConstraint* constraints, int constraintCount) const 
{
	if (constraintCount > 6) return false;

	for (int i = 0; i < constraintCount; ++i) 
	{
		if (m_edges[i] != constraints[i]) 
		{
			return false;
		}
	}
	return true;
}

