#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"

constexpr  float LIGHTSABER_WIDTH = 0.8f;
constexpr float   LIGHTSABER_LEN = 20.f;
constexpr float   LIGHTSABER_L1_SHIFT = 60.f;
constexpr float   LIGHTSABER_L2_SHIFT = 45.f;
class LightSaber :public Entity
{
public:
    LightSaber(Game* game, float x, float y,int level);
    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;
    //static void InitializedVerts(Vertex_PCU* vertsToFillIn);
public:
    int m_level = 1;
    Vertex_PCU vertices_0_deg[NUM_LIGHTSABER_EACHVERTS];
    Vertex_PCU vertices_120_deg[NUM_LIGHTSABER_EACHVERTS];
    Vertex_PCU vertices_240_deg[NUM_LIGHTSABER_EACHVERTS];

    Vertex_PCU vertices_90_deg[NUM_LIGHTSABER_EACHVERTS];
    Vertex_PCU vertices_180_deg[NUM_LIGHTSABER_EACHVERTS];
    Vertex_PCU vertices_270_deg[NUM_LIGHTSABER_EACHVERTS];
private:


    //AABB2 m_aabb_0;
    //AABB2 m_aabb_120;
    //AABB2 m_aabb_240;
    //AABB2 m_aabb_90;
    //AABB2 m_aabb_180;
    //AABB2 m_aabb_270;
    //Vertex_PCU vertices_L2[NUM_CANONL2_VERTS];
    //Vertex_PCU vertices_L3[NUM_CANONL3_VERTS];
    //Vertex_PCU vertices_L4[NUM_BULLET_VERTS];
};