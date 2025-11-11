#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "CellMatDef.hpp"

enum class PixelLayerType 
{
	EMPTY,
	GRAYSCALE,   
	COLORED
};

struct TemplatePixel 
{
	PixelLayerType m_layerType;
	float m_density;           // 0-1，用于黑白灰
	Rgba8 m_color;             // 用于彩色层
	CellMatType m_matType;     // 彩色层对应的材质类型
	bool m_isEdge = false;

	TemplatePixel()
		: m_layerType(PixelLayerType::GRAYSCALE)
		, m_density(0.0f)
		, m_color(Rgba8::BLACK)
		, m_matType(CellMatType::MAT_EMPTY)
		,m_isEdge(false)
	{}
};

struct ColorMaterialMapping
{
	Rgba8 m_color;
	CellMatType m_matType;
	float m_colorTolerance;  // 颜色容差（0-255）

	ColorMaterialMapping(Rgba8 c, CellMatType type, float tolerance = 30.0f)
		: m_color(c), m_matType(type), m_colorTolerance(tolerance) {}
};