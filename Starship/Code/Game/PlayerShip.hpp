#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "GameCommon.hpp"
class PlayerShip:public Entity
{
public:
	PlayerShip(Game* game, float x, float y);

    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;

    //------------------------------------------------------
    bool m_isShipRotateLeft = false;
    bool m_isShipRotateRight = false;
    bool m_isAccelerate = false;    
    bool m_isShoot = false;
    Vertex_PCU vertices[NUM_SHIP_VERTS];

private:
    void BounceOffWall();
    //void DetectAsteroids();
    void RespawnShip();
    
};