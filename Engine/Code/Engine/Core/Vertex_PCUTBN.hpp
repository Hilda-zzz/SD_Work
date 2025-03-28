#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"


struct Vertex_PCUTBN
{
	Vertex_PCUTBN()=default;
	explicit Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords,
		Vec3 const& tangent, Vec3 const& bitagent, Vec3 const& normal)
	: m_position(position), m_color(color), m_uvTexCoords(uvTexCoords),
		m_tangent(tangent),m_bitangent(bitagent), m_normal(normal)
	{}

	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	Vec3 m_tangent;
	Vec3 m_bitangent;
	Vec3 m_normal;
};