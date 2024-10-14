#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Beetle :public Entity
{
public:
    Beetle(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;
    void GetHurt(float hurtPoint);
    //------------------------------------------------------


    Vertex_PCU vertices[NUM_BEETLE_VERTS];

private:

};