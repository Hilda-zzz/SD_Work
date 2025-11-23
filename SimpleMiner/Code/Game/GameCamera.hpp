#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

class World;

enum class CameraMode
{
	FIRST_PERSON,           // Camera at entity's eye position, looking where entity looks
	OVER_THE_SHOULDER,      // Camera behind and slightly to side of entity
	//FIXED_ANGLE_TRACKING,   // Camera at fixed angle relative to entity, always facing entity
	INDEPENDENT,            // Camera independent of entity, but still following at distance
	SPECTATOR_FULL,         // Free camera movement in all directions (includes roll)
	SPECTATOR_XY,           // Free camera movement, but constrained to XY plane (no pitch/roll)
	COUNT
};

// GameCamera is an Entity that can follow and observe other entities
class GameCamera : public Entity
{
public:
	GameCamera(Game* game);
	virtual ~GameCamera();

	void Update(float deltaSeconds) override;
	void Render() const override;

	// Camera mode control
	void SetMode(CameraMode mode);
	CameraMode GetMode() const { return m_mode; }
	void ToggleMode(); // Cycle through modes

	// Target control - which entity is the camera following?
	void SetTarget(Entity* target) { m_targetEntity = target; }
	Entity* GetTarget() { return m_targetEntity; }

	// World reference for collision detection
	void SetWorld(World* world) { m_world = world; }

	// Get the underlying engine camera
	Camera& GetEngineCamera() { return m_engineCamera; }
	const Camera& GetEngineCamera() const { return m_engineCamera; }

	// Input for camera control (called by Player or other controllers)
	void ProcessRotationInput(float yawDelta, float pitchDelta);
	void ProcessZoomInput(float zoomDelta);

private:
	// Update functions for each mode
	void UpdateFirstPerson(float deltaSeconds);
	void UpdateOverTheShoulder(float deltaSeconds);
	//void UpdateFixedAngleTracking(float deltaSeconds);
	void UpdateIndependent(float deltaSeconds);
	void UpdateSpectatorFull(float deltaSeconds);
	void UpdateSpectatorXY(float deltaSeconds);

	// Third person camera helpers
	float CheckCameraCollision(const Vec3& from, const Vec3& to);

	// Smoothing utilities
	Vec3 SmoothDampPosition(const Vec3& current, const Vec3& target, float deltaTime);
	float SmoothDampFloat(float current, float target, float deltaTime);

private:
	// The actual engine camera object
	Camera m_engineCamera;

	// Camera mode and target
	CameraMode m_mode = CameraMode::FIRST_PERSON;
	Entity* m_targetEntity = nullptr;     // The entity we're following
	World* m_world = nullptr;             // For collision detection

	// Third person camera parameters
	float m_thirdPersonDistance = 5.0f;   // Desired distance from target
	float m_thirdPersonYaw = 0.0f;        // Horizontal angle around target
	float m_thirdPersonPitch = 20.0f;     // Vertical angle (degrees above horizon)

	// Camera constraints
	float m_minDistance = 1.0f;
	float m_maxDistance = 15.0f;
	float m_minPitch = -80.0f;
	float m_maxPitch = 80.0f;

	// Collision settings
	float m_cameraCollisionRadius = 0.2f; // Sphere radius for collision
	bool m_enableCollision = true;

	// Smoothing
	float m_positionSmoothTime = 0.1f;    // Smoothing for position transitions
	float m_distanceSmoothTime = 0.15f;   // Smoothing for distance changes
	Vec3 m_positionVelocity;              // Used by smooth damp
	float m_distanceVelocity = 0.0f;      // Used by smooth damp
	float m_currentDistance = 5.0f;       // Current smoothed distance

	// OverTheShoulder mode parameters
	Vec3 m_shoulderOffset = Vec3(0.5f, 4.0f, 0.0f);  // (right, back, up)

	// FixedAngleTracking mode parameters
	float m_fixedYaw = 40.0f;
	float m_fixedPitch = 30.0f;
	float m_fixedDistance = 10.0f;

	// Independent mode parameters
	float m_independentYaw = 0.0f;
	float m_independentPitch = 30.0f;
	float m_independentDistance = 8.0f;
};