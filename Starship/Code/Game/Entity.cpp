#include "Entity.hpp"
#include "GameCommon.hpp"

Entity::Entity(Game* game, float x, float y)
    : m_game(game), m_position{ x, y }, m_velocity{ 0, 0 },
    m_orientationDegrees(0), m_angularVelocity(0),
    m_physicsRadius(1.0f), m_cosmeticRadius(1.0f),
    m_health(100), m_isDead(false), m_isGarbage(false)
{

}

bool Entity::IsOffscreen() const
{
    //------
	if (m_position.x < -m_cosmeticRadius|| m_position.x > WORLD_SIZE_X + m_cosmeticRadius
		|| m_position.y < -m_cosmeticRadius|| m_position.y > WORLD_SIZE_Y + m_cosmeticRadius)
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
    //revert?
    return !m_isDead;
}

bool Entity::IsGarbage() const
{
    return m_isGarbage;
}
