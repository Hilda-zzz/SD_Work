#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Canon.hpp"
#include "Game/LaserExplosionCanon.hpp"
#include "Game/LightSaber.hpp"

constexpr float  MIN_SABER_MP = 1;
class PlayerShip:public Entity
{
public:
	PlayerShip(Game* game, float x, float y);
    ~PlayerShip();
    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;
    void GetHurt(float hurtHp);
    static void InitializedVerts(Vertex_PCU* vertsToFillIn, Rgba8 const& color);

public:
    bool m_isUseUpLives = false;
    int m_lives = 4;
    float m_maxHealth =100;

    bool m_isInvincibleState = false;
    double m_invincibleStartTime = 0.0;

    //related to permanent weapon
    int m_FirstWeaponState = 2;
    //int m_CanonLevel = 1;
    //int m_LaserCanonLevel = 1;
    Canon* m_Canon;
    LaserExplosionCanon* m_laserExplosionCanon;
    LightSaber* m_lightSaber;

    float m_maxMagicPoint = 300.f;
    float m_magicPoint = 300.f;

    bool m_isSaber = false;
    bool m_isControllerSaber = false;

    bool m_isCanonAutoCoolDown = true;
    float m_canonCoolDownTime = 0.2f;
    double m_startCanonCoolDownTime = 0.0;

private:
    Vertex_PCU vertices[NUM_SHIP_VERTS];
    Vertex_PCU tail_vertices[3];
    bool m_isShipRotateLeft = false;
    bool m_isShipRotateRight = false;
    bool m_isAccelerate = false;
    bool m_isControllerAccelerate = false;
    bool m_isKeepShoot = false;
    bool m_isJustShoot = false;
    bool m_isChangeState = false;
    float m_deadTime = 0.f;
    float m_thrustFraction;
    float m_quickDashRadius =0.f;
    bool m_isPlaySaberSound = false;
    float m_previousRightTrigger = 0.f;
private:
    void BounceOffWall();
    void RespawnShip();
    void UpdateFromController(float deltaTime);
    void TriggerBulletTime(float rightTrigger);
    void TriggerQuickDash();
    bool m_isTriggerQuickDash = false;
    bool m_isStartQuickDash = false;
    double m_startQuickDashTime = 0.0;
    void RenderQuickDashUI() const;
    void UpdateQuickDashUI(float leftMagnitute);
    Vec2 m_previousVelocity;
};