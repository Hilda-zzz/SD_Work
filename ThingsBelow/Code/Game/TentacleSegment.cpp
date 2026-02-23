#include "Game/TentacleSegment.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

TentacleSegment::TentacleSegment()
    : m_startPos(Vec2::ZERO)
    , m_endPos(Vec2::ZERO)
    , m_startRadius(1.0f)
    , m_endRadius(1.0f)
{
}

TentacleSegment::TentacleSegment(Vec2 const& start, Vec2 const& end, float startRadius, float endRadius)
    : m_startPos(start)
    , m_endPos(end)
    , m_startRadius(startRadius)
    , m_endRadius(endRadius)
{
}

TentacleSegment::~TentacleSegment()
{
}

float TentacleSegment::GetLength() const
{
    return (m_endPos - m_startPos).GetLength();
}

Vec2 TentacleSegment::GetDirection() const
{
    Vec2 dir = m_endPos - m_startPos;
    dir.Normalize();
    return dir;
}

void TentacleSegment::Render() const
{
    // Draw start circle
    DebugDrawCircle(m_startRadius, m_startPos, Rgba8::WHITE);
    
    // Draw end circle
    DebugDrawCircle(m_endRadius, m_endPos, Rgba8::WHITE);
    
    // Draw trapezoid between circles
    Vec2 direction = GetDirection();
    Vec2 perpendicular = direction.GetRotated90Degrees();
    
    Vec2 startLeft = m_startPos + perpendicular * m_startRadius;
    Vec2 startRight = m_startPos - perpendicular * m_startRadius;
    Vec2 endLeft = m_endPos + perpendicular * m_endRadius;
    Vec2 endRight = m_endPos - perpendicular * m_endRadius;
    
    Vertex_PCU vertices[6];
    vertices[0] = Vertex_PCU(Vec3(startLeft.x, startLeft.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    vertices[1] = Vertex_PCU(Vec3(startRight.x, startRight.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    vertices[2] = Vertex_PCU(Vec3(endRight.x, endRight.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    
    vertices[3] = Vertex_PCU(Vec3(startLeft.x, startLeft.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    vertices[4] = Vertex_PCU(Vec3(endRight.x, endRight.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    vertices[5] = Vertex_PCU(Vec3(endLeft.x, endLeft.y, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f));
    
    g_theRenderer->DrawVertexArray(6, vertices);
}
