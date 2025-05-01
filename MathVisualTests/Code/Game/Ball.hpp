#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"

struct Ball
{
	Ball(Vec2 const& position, float radius, float ela,Rgba8 const& curColor)
		:m_pos(position),m_radius(radius),m_elasticity(ela),m_color(curColor)
	{
		//m_verts.reserve(100);
		//AddVertsForDisc2D(m_verts, Vec2::ZERO, m_radius, curColor);
	}

	void Update(float deltaTime);
	//void Render() const;

	Vec2 m_pos;
	Vec2 m_acceleration;
	Vec2 m_velocity;
	float m_radius;
	float m_elasticity;
	//std::vector<Vertex_PCU> m_verts;
	Rgba8 m_color;
};