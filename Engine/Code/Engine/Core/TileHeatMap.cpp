#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

TileHeatMap::TileHeatMap(IntVec2 const& dimensions):m_dimensions(dimensions)
{
	m_values.resize(m_dimensions.x * m_dimensions.y);
	SetAllValues(999999.f);
}

float TileHeatMap::GetTileHeatValue(IntVec2 const& coords)
{
	return GetTileHeatValue(coords.x, coords.y);
}

float TileHeatMap::GetTileHeatValue(int coordsX, int coordsY)
{
	int index = coordsX + coordsY * m_dimensions.x;
	return m_values[index];
}

void TileHeatMap::SetTileHeatValue(IntVec2 const& coords,float value)
{
	SetTileHeatValue(coords.x, coords.y, value);
}

void TileHeatMap::SetTileHeatValue(int coordsX, int coordsY, float value)
{
	int index = coordsX + coordsY * m_dimensions.x;
	m_values[index] = value;
}

void TileHeatMap::AddTileHeatValue(IntVec2 const& coords, float value)
{
	int index = coords.x + coords.y * m_dimensions.x;
	m_values[index] += value;
}

void TileHeatMap::AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 totalBounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor)
{
	float tileLenX = (totalBounds.m_maxs.x - totalBounds.m_mins.x) / m_dimensions.x;
	float tileLenY = (totalBounds.m_maxs.y - totalBounds.m_mins.y) / m_dimensions.y;
	for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++)
	{
		int coordsX = i % m_dimensions.x;
		int coordsY = i / m_dimensions.x;
		Rgba8 curTileColor;
		AABB2 curTileAABB2 = AABB2(coordsX * tileLenX, coordsY * tileLenY,
			coordsX * tileLenX + tileLenX, coordsY * tileLenY + tileLenY);
		if (m_values[i] == specialValue)
		{
			curTileColor = specialColor;
		}
		else
		{
			float rangeResult = RangeMapClamped(m_values[i], valueRange, FloatRange::ZERO_TO_ONE);
			curTileColor = Interpolate(lowColor, highColor, rangeResult);
		}
		AddVertsForAABB2D(verts, curTileAABB2, curTileColor);
	}
}

void TileHeatMap::SetAllValues(float value)
{
	for (int i = 0; i < m_dimensions.x * m_dimensions.y; i++)
	{
		m_values[i] = value;
	}
}
