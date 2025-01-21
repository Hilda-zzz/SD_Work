#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include <vector>
#include "Game/GameCommon.hpp"

class Tile;
class Player :public Entity
{
	friend Map;
public:
	Player(EntityType type,EntityFaction faction);
	~Player();
	void Update(float deltaTime) override;
	void Render() const override;
	void Die() override;
	void DrawDebugMode() const override;

	float	m_turretOrientationDegrees;
public:
	IntVec2 m_curPlayerTileCoorPos;
	float m_curSpinSpeed = 0.f;
	float m_scale = 1.f;
	void ResetHealth();
private:
	void UpdateKBMoveInput(float deltaTime);
	void UpdateKBTurretInput(float deltaTime);
	void UpdateFromController(float deltaSeconds);
	void UpdateMovement(float deltaSeconds);
	
private:
	OBB2	m_bodyBox;
	float	m_speed;
	bool	m_isMove = false;
	bool	m_isRotateTurret = false;
	Vec2	m_aimDirection;
	float	m_previous_orientation;

	OBB2	m_turretBox;
	
	Vec2	m_turret_aimDirection;
	float	m_turretAngularVelocity;

	std::vector<Vertex_PCU> m_verts_body;
	std::vector<Vertex_PCU> m_verts_turret;

	Texture* m_texPlayerTankBase;
	Texture* m_texPlayerTankTurret;
	Texture* m_texPlayerTankBullet;

	bool m_isShooting = false;
	bool m_isShootCoolDown = false;
	double m_lastShootTime=0.f;

	float m_speedRatio = 1.f;
	
	int m_weaponState = 1;

	//double m_gapShootTime = 0.1f;
};