#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Wasp :public Entity
{
public:
    Wasp(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;

    //------------------------------------------------------


    Vertex_PCU vertices[NUM_BEETLE_VERTS];

private:

};