#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

struct SpotLight
{
	SpotLight() {}

	SpotLight(Vec3 const& position, Vec3 const& direction, float innerCutoffAngle, float outerCutoffAngle,
		Rgba8 const& color, float intensity,float innerRadius,float outerRadius) :
		c_position(position),
		c_direction(direction),
		c_intensity(intensity),
		c_innerCutoffAngle(innerCutoffAngle),
		c_outerCutoffAngle(outerCutoffAngle),
		c_innerRadius(innerRadius),
		c_outerRadius(outerRadius)
	{
		color.GetAsFloats(&c_color[0]);
	}
	Vec3 c_position;
	float c_intensity;
	Vec3 c_direction;		
	float c_padding = 0.f;	
	float c_color[4];
	float c_innerCutoffAngle;
 	float c_outerCutoffAngle;	
	float c_innerRadius;
	float c_outerRadius;	
	
};