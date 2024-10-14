#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
#include "Game.hpp"

class BkgStar
{

public:
    BkgStar(Game* game,Vec2 pos,int layer);
    ~BkgStar();
    void Update(float deltaTime);
    void Render() const;
private:
    Vertex_PCU m_vertices[48];
    Vec2 m_position;
    Game* m_game;
    int m_layer;
    float m_movingRatio;
};