#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

struct SpotLight
{
	SpotLight() {}
	SpotLight(Vec3 const& position, float range, Rgba8 const& color, float intensity, Vec3 const& attenuation,float cone,Vec3 const& direction,float halfAngle) :
		Position(position), Range(range), Intensity(intensity), Attenuation(attenuation),Cone(cone),Direction(direction),SpotCutoffAngle(halfAngle)
	{
		color.GetAsFloats(&Color[0]);
	}
	Vec3 Position;
	float Range;
	float Color[4];
	float Intensity;
	Vec3 Attenuation;
	float Cone;
	Vec3 Direction;
	float SpotCutoffAngle;
	Vec3 padding;
};