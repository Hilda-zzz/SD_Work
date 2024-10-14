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
    void GetHurt(float hurtPoint);

private:
    Vertex_PCU vertices[NUM_WASP_VERTS];
};