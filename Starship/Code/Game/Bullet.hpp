#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Bullet :public Entity
{
public:
    Bullet(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;

    //------------------------------------------------------


    Vertex_PCU vertices[NUM_BULLET_VERTS];

private:
    //void DetectAsteroid();
};