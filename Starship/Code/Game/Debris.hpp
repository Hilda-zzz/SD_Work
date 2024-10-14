#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Debris :public Entity
{
public:
    Debris(Game* game, float x, float y,float minSpeed,float maxSpeed,float minRadius,
        float maxRadius, Rgba8 color,Vec2 const& oriVelocity,int type);
    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;

private:
    double  m_startTime = 0;
    double  m_currentTime = 0;
    float   m_current_a = 0; 
    int m_type = 1;
    Vertex_PCU  vertices[NUM_DEBRIS_VERTS];
    float       m_point_len[(NUM_DEBRIS_VERTS / 3) + 1];
};