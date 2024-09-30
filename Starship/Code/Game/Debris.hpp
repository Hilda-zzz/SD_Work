#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Debris :public Entity
{
public:
    Debris(Game* game, float x, float y,float minSpeed,float maxSpeed,float minRadius,float maxRadius, Rgba8 color,Vec2& const oriVelocity);

    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;

    //------------------------------------------------------


    Vertex_PCU vertices[NUM_DEBRIS_VERTS];
    float m_point_len[(NUM_DEBRIS_VERTS / 3) + 1];

private:
    double startTime = 0;
    double currentTime = 0;
    float current_a = 0;
    
};