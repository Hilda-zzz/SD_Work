#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

struct PointLight
{
	PointLight(){}
	PointLight(Vec3 const& position, float range, Rgba8 const& color, float intensity, Vec3 const& attenuation) :
		c_position(position), c_range(range), c_intensity(intensity), c_attenuation(attenuation)
	{
		color.GetAsFloats(&c_color[0]);
	}
// 	Vec3 Position;
// 	float Range;
// 	float Color[4];
// 	float Intensity;
// 	Vec3 Attenuation;

	Vec3 c_position;
	float c_range;
	float c_color[4];
	float c_intensity;
	Vec3 c_attenuation;
};