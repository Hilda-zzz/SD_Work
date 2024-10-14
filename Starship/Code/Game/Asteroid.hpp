#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class Asteroid :public Entity
{
public:
    Asteroid(Game* game, float x, float y);
    ~Asteroid();
    void Update(float deltaTime) override;
    void Render() const  override;
    void Die() override;
    void GetHurt(float hurtPoint);
public:
    bool m_isInvincibleState = false;
    double m_invincibleStartTime = 0.0;

private:
    Vertex_PCU vertices[NUM_ASTEROID_VERTS];
    float m_point_len[(NUM_ASTEROID_VERTS / 3) + 1];

    Rgba8 m_asteroidOriColor = Rgba8(100, 100, 100, 255);
    Rgba8 m_asteroidCurColor;

private:
    void WrapAround();
};
