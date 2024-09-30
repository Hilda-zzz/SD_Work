#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Asteroid :public Entity
{
public:
    Asteroid(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;

    //------------------------------------------------------


    Vertex_PCU vertices[NUM_ASTEROID_VERTS];
    float m_point_len[(NUM_ASTEROID_VERTS/3)+1];

private:
    void WrapAround();
};