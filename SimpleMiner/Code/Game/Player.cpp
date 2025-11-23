#include "Game/Player.hpp"
#include "Game/GameCamera.hpp"
#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Game.hpp"

extern Window* g_theWindow;

Player::Player(Game* owner)
	: Entity(owner)
{
	m_eyeHeight = 1.65f;  // Eyes at 1.65m above feet

	// Start in walking mode
	SetPhysicsMode(PhysicsMode::WALKING);

	// Initial position (high up so we fall to ground)
	SetPosition(Vec3(0.f, 0.f, 110.f));
	SetOrientation(EulerAngles(0.f, 0.f, 0.f));
}

Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	// 检查是否应该应用重力
	bool shouldApplyGravity = false;
	if (m_physicsMode == PhysicsMode::WALKING)
	{
		if (!m_isOnGround)
		{
			Chunk* chunkBelow = m_world->GetChunkByWorldPos(m_position);
			if (chunkBelow != nullptr)
			{
				shouldApplyGravity = true;
			}
		}
	}

	if (shouldApplyGravity)
	{
		m_acceleration.z -= g_playerGravityAcceleration;
	}
	//------------------------------------------------------------
	// Drag
	float dragCoeff = m_isOnGround ? g_playerHorizontalGroundDragCoefficient : 
		g_playerHorizontalAirDragCoefficient;
	Vec3 horizontalVelocity(m_velocity.x, m_velocity.y, 0.f);
	Vec3 dragForce = -dragCoeff * horizontalVelocity;

	m_acceleration += dragForce;

	//------------------------------------------------------------
	Entity::UpdatePhysics(deltaSeconds);
}

void Player::SetEyesightOrientation(EulerAngles eye)
{
	m_eyeSightOrientation = eye;
}



bool Player::CanJump() const
{
	// Can only jump if on the ground and in walking mode
	return IsOnGround() && GetPhysicsMode() == PhysicsMode::WALKING;
}