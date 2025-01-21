#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
class EnemyBullet :public Entity
{
public:
    EnemyBullet(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;
    static void InitializedVerts(Vertex_PCU* vertsToFillIn);

private:
    Vertex_PCU vertices[NUM_ENEMYBULLET_VERTS];
};