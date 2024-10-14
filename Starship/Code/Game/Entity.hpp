#pragma once
#include "Engine/Math/Vec2.hpp"
class Game;

class Entity {
public:

    Entity(Game* game, float x, float y);

    virtual void Update(float deltaTime)=0;
    virtual void Render() const=0;
    virtual void Die()=0;

    virtual ~Entity() = default;

    //change to
    //virtual ~Entity();
    //when new specific entity
    //Asteroid*asteroid=dynamic_cast<Asteroid>(m_asteroid[index]);
    //Entity* pointer only know parent class thing

    //private CheckBulletVSEnemyList(Bullet& bullet,int listMaxSize, Entity** entityList);
    //  for(i<maxSize)
    //  Entity* pointerToThisEntity=enemyList[i];
    //  if(IsAlive())
    //  CheckBUlletVsEnemy(bullet,*pointToThisEntity)


//protected:
    Vec2 m_position;
    Vec2 m_velocity;
    float m_orientationDegrees;
    float m_angularVelocity; 
    float m_physicsRadius;
    float m_cosmeticRadius; 
    float m_health; 
    float m_oriHealth;
    bool m_isDead; 
    bool m_isGarbage; 
    Game* m_game;

public:
    bool IsOffscreen() const;
    Vec2 GetForwardNormal() const;
    bool IsAlive() const;
    bool IsGarbage() const;
    void RenderUI();
};
