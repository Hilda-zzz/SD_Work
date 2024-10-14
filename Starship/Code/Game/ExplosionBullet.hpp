#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class ExplosionBullet :public Entity
{
public:
    ExplosionBullet(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;

private:
    Vertex_PCU vertices[NUM_EXPLOSION_BULLET_VERTS];
};