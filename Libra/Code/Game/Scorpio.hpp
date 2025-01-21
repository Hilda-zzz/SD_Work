#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include <vector>
#include "Game/GameCommon.hpp"
#include "Engine/Math/Ray.hpp"
#include "Engine/Math/RaycastUtils.hpp"
class Scorpio :public Entity
{
public:
	Scorpio(EntityType type,EntityFaction faction);
	~Scorpio();
	void Update(float deltaTime) override;
	void Render() const override;
	void Die() override;
	void DrawDebugMode() const override;
public:
	
private:

private:
	OBB2	m_bodyBox;
	float	m_speed;

	OBB2	m_turretBox;
	//float	m_turret_orientation;
	Vec2	m_turret_aimDirection;
	float	m_turretAngularVelocity;

	std::vector<Vertex_PCU> m_verts_body;
	std::vector<Vertex_PCU> m_verts_turret;
	std::vector<Vertex_PCU> m_verts_ray;

	Texture* m_texScorpioBase;
	Texture* m_texScorpioTurret;
	Texture* m_texScorpioBullet;

	Entity* m_target = nullptr;
	bool m_isTargetVisible = false;

	Ray m_ray;
	RaycastResult2D m_raycastResult;

	bool m_isShooting = false;
	bool m_isShootCoolDown = false;
	double m_lastShootTime = 0.f;
	double m_gapShootTime = 1.2f;
};