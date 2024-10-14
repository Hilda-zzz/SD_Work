#include "Entity.hpp"
#include "GameCommon.hpp"
#include <Engine/Math/MathUtils.hpp>

Entity::Entity(Game* game, float x, float y)
    : m_game(game), m_position{ x, y }, m_velocity{ 0, 0 },
    m_orientationDegrees(0), m_angularVelocity(0),
    m_physicsRadius(1.0f), m_cosmeticRadius(1.0f),
    m_health(100), m_isDead(false), m_isGarbage(false)
{
}

bool Entity::IsOffscreen() const
{
	if (m_position.x < WORLDSPACE_SIZE_BL_X-m_cosmeticRadius|| m_position.x > WORLDSPACE_SIZE_TR_X + m_cosmeticRadius
		|| m_position.y < WORLDSPACE_SIZE_BL_Y-m_cosmeticRadius|| m_position.y > WORLDSPACE_SIZE_TR_Y+ m_cosmeticRadius)
	{
		return true;
	}
    else
        return false;
}

Vec2 Entity::GetForwardNormal() const
{
    return Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1);
}

bool Entity::IsAlive() const 
{
    return !m_isDead;
}

bool Entity::IsGarbage() const
{
    return m_isGarbage;
}

void Entity::RenderUI()
{
    float hpBarLen = RangeMapClamped(m_health, 0.f, m_oriHealth, 0.f, 2 * m_cosmeticRadius);
    DebugDrawBox(Vec2(m_position.x - m_cosmeticRadius, m_position.y + m_cosmeticRadius + 0.5f),
        Vec2(m_position.x + m_cosmeticRadius, m_position.y + m_cosmeticRadius + 0.5f + 0.5f), Rgba8(255, 255, 255));
    DebugDrawBox(Vec2(m_position.x - m_cosmeticRadius, m_position.y + m_cosmeticRadius + 0.5f),
        Vec2(m_position.x - m_cosmeticRadius + hpBarLen, m_position.y + m_cosmeticRadius + 0.5f + 0.5f), Rgba8(255, 50, 10));
}


