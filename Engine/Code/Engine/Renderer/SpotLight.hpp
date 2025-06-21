#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

struct SpotLight
{
	SpotLight() {}
	SpotLight(Vec3 const& position, float range, Rgba8 const& color, float intensity,
		Vec3 const& attenuation, float cone, Vec3 const& direction, float halfAngle) :
		c_position(position), c_range(range), c_intensity(intensity), c_attenuation(attenuation),
		c_cone(cone), c_direction(direction), c_spotCutoffAngle(halfAngle), c_padding(Vec3(0.f,0.f,0.f))
	{
		color.GetAsFloats(&c_color[0]);
	}

	Vec3 c_position;			
 	float c_range;				
 	float c_color[4];			
 	float c_intensity;			
 	Vec3 c_attenuation;			
 	float c_cone;				
 	Vec3 c_direction;			
 	float c_spotCutoffAngle;	
 	Vec3 c_padding;				
};