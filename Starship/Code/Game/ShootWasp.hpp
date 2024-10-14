#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class ShootWasp :public Entity
{
public:
    ShootWasp(Game* game, float x, float y);
    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;
    void GetHurt(float hurtPoint);
    static void InitializedVerts(Vertex_PCU* vertsToFillIn, Rgba8 const& color);

    bool m_isAutoCoolDown =true;
    double m_startCoolDownTime = 0.f;
    float m_coolDownTime = 1.f;
private:
    Vertex_PCU vertices[NUM_SHOOTWASP_VERTS];

    Rgba8 m_shootWaspOriColor = Rgba8(100, 100, 100, 255);
    Rgba8 m_shootWaspCurColor;
};