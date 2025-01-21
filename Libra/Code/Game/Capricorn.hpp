#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include <vector>
#include "Game/GameCommon.hpp"

class Capricorn :public Entity
{
public:
	Capricorn(EntityType type, EntityFaction faction);
	~Capricorn();
	void Update(float deltaTime) override;
	void Render() const override;
	void Die() override;
	void DrawDebugMode() const override;
public:

private:

private:
	OBB2	m_bodyBox;
	float	m_speed;

	std::vector<Vertex_PCU> m_verts_body;
	Texture* m_texCapricornBase;
	Texture* m_texCapricornBullet;

	bool m_isShooting = false;
	bool m_isShootCoolDown = false;
	double m_lastShootTime = 0.f;
	double m_gapShootTime = 1.2f;

	bool m_isTargetVisible = false;
	Vec2 m_lastTargetPos;
	Vec2 m_nextWayPointPosition;
	int  m_pathState = 0;//0 wander,1 go to player,2 go to last seen
	float m_speedRatio = 1.f;
};