#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Nova2DParticleInstance 
{
	Vec2 position;   // 8 bytes
	Vec2 size;       // 8 bytes  
	Rgba8 color;     // 4 bytes
	float rotation;  // 4 bytes
	// 24 bytes
};