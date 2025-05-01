#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

struct PointLight
{
	PointLight(){}
	PointLight(Vec3 const& position, float range, Rgba8 const& color, float intensity, Vec3 const& attenuation) :
		Position(position), Range(range), Intensity(intensity), Attenuation(attenuation)
	{
		color.GetAsFloats(&Color[0]);
	}
	Vec3 Position;
	float Range;
	float Color[4];
	float Intensity;
	Vec3 Attenuation;
};