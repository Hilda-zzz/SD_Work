#pragma once

#include "Engine/Math/Vec2.hpp"

//#include "Game.hpp"

class Game;
class Entity {
public:

    Entity(Game* game, float x, float y);

    virtual void Update(float deltaTime)=0;
    virtual void Render() const=0;
    virtual void Die()=0;

    virtual ~Entity() = default;

//protected:
    Vec2 m_position;
    Vec2 m_velocity;
    float m_orientationDegrees;
    float m_angularVelocity; 
    float m_physicsRadius;
    float m_cosmeticRadius; 
    int m_health; 
    bool m_isDead; 
    bool m_isGarbage; 
    Game* m_game;

public:
    bool IsOffscreen() const;
    Vec2 GetForwardNormal() const;
    bool IsAlive() const;

    bool IsGarbage() const;
};
