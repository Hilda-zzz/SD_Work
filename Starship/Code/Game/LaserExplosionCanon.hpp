#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"

class LaserExplosionCanon :public Entity
{
public:
    LaserExplosionCanon(Game* game, float x, float y, int level);
    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;
    static void InitializedVertsL1(Vertex_PCU* vertsToFillIn);
    //static void InitializedVertsL2(Vertex_PCU* vertsToFillIn);
   // static void InitializedVertsL3(Vertex_PCU* vertsToFillIn);
public:
    int m_level = 1;

private:
    Vertex_PCU vertices_L1[NUM_LASER_CANON_VERTS];
    //Vertex_PCU vertices_L2[NUM_CANONL2_VERTS];
    //Vertex_PCU vertices_L3[NUM_CANONL3_VERTS];
    //Vertex_PCU vertices_L4[NUM_BULLET_VERTS];
};