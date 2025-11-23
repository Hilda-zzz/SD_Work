#pragma once
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Game;
class World;

enum class PhysicsMode
{
	WALKING,
	FLYING,
	NOCLIP
};

struct AxisImpact
{
	bool m_hasImpact = false;
	float m_impactFraction = 1.0f;
	Vec3 m_normal;
};

class Entity
{
public:
	Entity(Game* owner);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const;
	virtual Mat44 GetModelToWorldTransform() const;
	virtual Mat44 GetModelToWorldTransform_WithoutOrientation() const;

	//---------------------------------------------------------------------------------
	// Physics functions - implemented in Entity for all derived classes
	virtual void UpdatePhysics(float deltaSeconds);

	Vec3 ResolveCollisionsWithRaycasts(const Vec3& deltaPosition);
	float SweepAABBAlongAxis(const Vec3& startPosition, const Vec3& direction, float distance);
	// Collision detection (uses AABB3 physics body)
	void CheckGroundCollision();


	// Physics queries
	bool IsOnGround() const { return m_isOnGround; }
	AABB3 GetPhysicsAABB() const { return m_physicsAABB; }
	Vec3 GetEyePosition() const;

	//---------------------------------------------------------------------------------
	// Public accessors for transform
	Vec3 GetPosition() const { return m_position; }
	void SetPosition(const Vec3& position) { m_position = position; }

	EulerAngles GetOrientation() const { return m_orientation; }
	void SetOrientation(const EulerAngles& orientation) { m_orientation = orientation; }

	Vec3 GetVelocity() const { return m_velocity; }
	void SetVelocity(const Vec3& velocity) { m_velocity = velocity; }

	EulerAngles GetAngularVelocity() const { return m_angularVelocity; }
	void SetAngularVelocity(const EulerAngles& angularVelocity) { m_angularVelocity = angularVelocity; }

	Vec3 GetAcceleration() const { return m_acceleration; }
	void SetAcceleration(const Vec3& acceleration) { m_acceleration = acceleration; }

	// Physics mode accessors
	PhysicsMode GetPhysicsMode() const { return m_physicsMode; }
	void SetPhysicsMode(PhysicsMode mode) { m_physicsMode = mode; }

	// AABB3 setup
	void SetPhysicsAABB(const AABB3& aabb) { m_physicsAABB = aabb; }

	// World interaction
	void SetWorld(World* world) { m_world = world; }
	World* GetWorld() { return m_world; }

private:
	std::vector<Vec3> GetCornerPositions();
	Vec3 GetShrunkCorner(const Vec3& corner);
	void DepenetrateSolidBlocks();

protected:
	Game* m_game = nullptr;

	// World reference
	World* m_world = nullptr;

	// Transform
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles	m_orientation;
	EulerAngles m_angularVelocity;

	// Physics properties
	PhysicsMode m_physicsMode = PhysicsMode::WALKING;
	Vec3 m_acceleration;
	float m_verticalVelocity;      // Vertical component for gravity
	bool m_isOnGround=false;

	// AABB3 physics body (as required by assignment)
	AABB3 m_physicsAABB;           // Physics collision box
	float m_eyeHeight;             // Eye height above bottom of AABB

	// Visual
	Rgba8 m_color = Rgba8::CYAN;
	std::vector<Vertex_PCU> m_wireFrameVerts;
};