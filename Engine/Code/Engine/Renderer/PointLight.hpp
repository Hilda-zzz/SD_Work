#pragma once
#include "Engine/Math/Vec3.hpp"
#include "../Core/Rgba8.hpp"

//struct PointLight
//{
//	PointLight(){}
//	PointLight(Vec3 const& position, float innerRadius, float outerRadius, Rgba8 const& color, float intensity, Vec3 const& attenuation) :
//		c_position(position), c_innerRadius(innerRadius), c_outerRadius(outerRadius), c_intensity(intensity)
//	{
//		color.GetAsFloats(&c_color[0]);
//	}
//// 	Vec3 Position;
//// 	float Range;
//// 	float Color[4];
//// 	float Intensity;
//// 	Vec3 Attenuation;
//
//	Vec3 c_position;
//	float c_innerRadius;
//	float c_outerRadius;
//	float c_color[4];
//	float c_intensity;
//	Vec2  c_padding;
//	//Vec c_attenuation;
//};

struct PointLight
{
	PointLight() {}
	PointLight(Vec3 const& position, float innerRadius, float outerRadius, Rgba8 const& color, float intensity) :
		c_position(position), c_innerRadius(innerRadius), c_outerRadius(outerRadius), c_intensity(intensity)
	{
		color.GetAsFloats(&c_color[0]);
	}

	Vec3 c_position;      
	float c_innerRadius;  

	float c_color[4];     

	float c_outerRadius;
	float c_intensity;
	Vec2 c_padding;

};