#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"
class TileHeatMap
{
public:
	TileHeatMap(IntVec2 const& dimensions);
	~TileHeatMap() = default;
	float GetTileHeatValue(IntVec2 const& coords);
	float GetTileHeatValue(int coordsX, int coordsY);
	void SetTileHeatValue(IntVec2 const& coords,float value);
	void SetTileHeatValue(int coordsX,int coordsY, float value);
	void AddTileHeatValue(IntVec2 const& coords, float value);
	void AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts,
		AABB2 totalBounds, FloatRange valueRange = FloatRange(0.f, 1.f),
		Rgba8 lowColor = Rgba8(0, 0, 0, 255), Rgba8 highColor = Rgba8(255, 255, 255, 255),
		float specialValue = 999999.f, Rgba8 specialColor = Rgba8(0, 0, 200));
	void SetAllValues(float value);
public:
	IntVec2 m_dimensions;
	std::vector<float> m_values;
};