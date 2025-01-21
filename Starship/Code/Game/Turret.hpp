#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
class Turret :public Entity
{
public:
    Turret(Game* game, float x, float y);
    void Update(float deltaTime) override;
    void Render() const  override;
    //void RenderUI();
    void Die() override;
    void GetHurt(float hurtPoint);

    //bool m_isAutoCoolDown = true;
    double m_startRotateTime = 0.f;
    float m_staticTime = 3.f;
private:
    Vertex_PCU vertices_base[NUM_TURRET_VERTS];

    Rgba8 m_shootWaspOriColor = Rgba8(100, 100, 100, 255);
    Rgba8 m_shootWaspCurColor;

    Vec2 m_oriPosition;

    bool m_isPlayerInArea = false;
    //bool m_isCanTure = false;
    bool m_isStaticState = true;
    bool m_isRotate = false;
};