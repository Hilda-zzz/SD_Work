#include "Game/Bullet.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
Bullet::Bullet(EntityType type, EntityFaction faction):Entity(type,faction)
{
	m_cosmeticRadius = BULLET_PLAYER_COS_RADIUS;
	m_physicsRadius = BULLET_PLAYER_PHY_RADIUS;
	m_orientationDegrees = 0.f;
	m_spinOri = m_orientationDegrees;
	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;
	
	switch (m_type)
	{
	case ENTITY_TYPE_GOOD_BULLET:
		m_speed = g_gameConfigBlackboard.GetValue("defaultBulletSpeed",10.f); 
		m_bounceTime = 2;
		m_causeHurtPoint = 1.f;
		m_texBullet= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBullet.png");
		m_bulletBox = OBB2(Vec2(0.f, 0.f), Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.1f, 0.05f));
		break;
	case ENTITY_TYPE_EVIL_BULLET:
		m_speed = g_gameConfigBlackboard.GetValue("defaultBulletSpeed", 10.f);
		m_causeHurtPoint = 1.f;
		m_texBullet = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");
		m_bulletBox = OBB2(Vec2(0.f, 0.f), Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.1f, 0.05f));
		break;
	case ENTITY_TYPE_GOOD_BOLT:
	case ENTITY_TYPE_EVIL_BOLT:
		m_speed = g_gameConfigBlackboard.GetValue("defaultBoltSpeed", 6.f);
		m_causeHurtPoint = 1.f;
		m_texBullet = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
		m_bulletBox = OBB2(Vec2(0.f, 0.f), Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.14f, 0.07f));
		break;
	case ENTITY_TYPE_EVIL_GDMISSILE:
		m_speed = g_gameConfigBlackboard.GetValue("guideMissileSpeed", 5.f);
		m_angularVelocity = g_gameConfigBlackboard.GetValue("guideMissileTurnRate", 100.f);
		m_causeHurtPoint = 2.f;
		m_texBullet = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyShell.png");
		m_bulletBox = OBB2(Vec2(0.f, 0.f), Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.18f, 0.09f));
		break;
	case ENTITY_TYPE_GOOD_FLAMEBULLET:
		m_speed=g_gameConfigBlackboard.GetValue("flameBulletSpeed", 10.f);
		m_angularVelocity = g_theGame->m_rng.RollRandomFloatInRange(-500.f, 500.f);
		m_causeHurtPoint = 1.f;
		m_texBullet = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyShell.png");
		m_bulletBox = OBB2(Vec2(0.f, 0.f), Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.45f, 0.45f));
		m_startTime =(float) GetCurrentTimeSeconds();
		break;
	}
	AddVertsForOBB2D(m_verts_body, m_bulletBox, Rgba8(255, 255, 255, 255));
}

void Bullet::Update(float deltaTime)
{
	if (m_type == ENTITY_TYPE_EVIL_GDMISSILE)
	{
		Vec2 aimDirection = m_map->m_player->m_position - m_position;
		float curDeg = GetTurnedTowardDegrees(m_orientationDegrees, aimDirection.GetOrientationDegrees(), m_angularVelocity * deltaTime);
		m_orientationDegrees = curDeg;
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_speed);
		m_position.x += m_velocity.x * deltaTime;
		m_position.y += m_velocity.y * deltaTime;
		return;
	}
	else if (m_type == ENTITY_TYPE_GOOD_FLAMEBULLET)
	{
		m_spinOri += m_angularVelocity * deltaTime;
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_speed);
		m_position.x += m_velocity.x * deltaTime;
		m_position.y += m_velocity.y * deltaTime;

		float duration =(float) GetCurrentTimeSeconds() - m_startTime;
		SpriteDefinition const& curExplosionAniSpriteDef = g_explosionAnimDef_Fast->GetSpriteDefAtTime(duration);
		m_texBullet = &curExplosionAniSpriteDef.GetTexture();
		m_verts_body.clear();
		AddVertsForOBB2D(m_verts_body, m_bulletBox, Rgba8(255, 255, 255, 255), curExplosionAniSpriteDef.GetUVs().m_mins, curExplosionAniSpriteDef.GetUVs().m_maxs);

		if ((float)GetCurrentTimeSeconds() - m_startTime > 0.35f)
		{
			Die();
		}
		return;
	}
	else
	{
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_speed);
		m_position.x += m_velocity.x * deltaTime;
		m_position.y += m_velocity.y * deltaTime;
		return;
	}

}

void Bullet::Render() const
{
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
	{
		temp_verts.push_back(m_verts_body[i]);
	}
	if (m_type == ENTITY_TYPE_GOOD_FLAMEBULLET)
	{
		TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_spinOri, m_position);
		g_theRenderer->BindTexture(m_texBullet);
		g_theRenderer->DrawVertexArray(temp_verts);
		return;
	}
	TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_texBullet);
	g_theRenderer->DrawVertexArray(temp_verts);
}

void Bullet::Die()
{
	m_isDead = true;
	m_isGarbage = true;
	if (m_type != ENTITY_TYPE_GOOD_FLAMEBULLET)
	{
		m_map->SpawnExplosion(m_position, 0.2f, g_explosionAnimDef_Fast);
	}
}

void Bullet::HitWall(IntVec2 const& normal)
{
	m_bounceTime--;
	if (m_bounceTime >= 0)
	{
		Vec2 curDirection = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
		Vec2 newDirection= curDirection.GetReflected(Vec2((float)normal.x,(float)normal.y));
		m_orientationDegrees = newDirection.GetOrientationDegrees();
		return;
	}
	Die();
}

void Bullet::HitWall(Vec2 const& normal)
{
	m_bounceTime--;
	if (m_bounceTime >= 0)
	{
		Vec2 curDirection = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
		Vec2 newDirection = curDirection.GetReflected(normal);
		m_orientationDegrees = newDirection.GetOrientationDegrees();
		return;
	}
	Die();
}

void Bullet::HitEntity()
{
	Die();
}


