#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Nova2DParticleInstance 
{
	Vec2 m_worldPosition;   // 8 bytes
	Vec2 m_size;       // 8 bytes  
	Rgba8 m_color;     // 32 bytes
	float m_rotation;  // 4 bytes

	float m_uvMinX;
	float m_uvMinY;
	float m_uvMaxX;
	float m_uvMaxY;

	float m_padding[2];
	// 48 bytes/ instance
};