#include "Vertex_PCU.hpp"

Vertex_PCU::Vertex_PCU()
    : m_position(), m_color(), m_uvTexCoords()
{   
}

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords)
    : m_position(position), m_color(color), m_uvTexCoords(uvTexCoords)
{
}