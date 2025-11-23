#include "Game/GameCamera.hpp"
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Math/Easing.hpp"
#include "Engine/Core/EngineCommon.hpp"

extern Window* g_theWindow;

GameCamera::GameCamera(Game* game)
	: Entity(game)
{
	// Camera doesn't need physics by default
	m_physicsMode = PhysicsMode::NOCLIP;

	// Setup the engine camera
	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
	m_engineCamera.SetViewport(viewport);

	float aspectRatio = viewport.GetDimensions().x / viewport.GetDimensions().y;
	m_engineCamera.SetPerspectiveView(aspectRatio, 60.f, 0.01f, 100000.f);

	// Set camera transform (matching your existing setup)
	m_engineCamera.SetCameraToRenderTransform(
		Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f))
	);

	Vec3 halfExtents(0.5f / 2.0f, 0.5f / 2.0f, 0.5f / 2.0f);
	m_physicsAABB.m_mins = -halfExtents;
	m_physicsAABB.m_maxs = halfExtents;
}

GameCamera::~GameCamera()
{
}

void GameCamera::Update(float deltaSeconds)
{
	if (!m_targetEntity)
	{
		// No target, just update engine camera with current position/orientation
		m_engineCamera.SetPositionAndOrientation(m_position, m_orientation);
		return;
	}

	// Update based on current mode
	switch (m_mode)
	{
	case CameraMode::FIRST_PERSON:
		UpdateFirstPerson(deltaSeconds);
		break;

	case CameraMode::OVER_THE_SHOULDER:
		UpdateOverTheShoulder(deltaSeconds);
		break;

	case CameraMode::INDEPENDENT:
		UpdateIndependent(deltaSeconds);
		break;

	case CameraMode::SPECTATOR_FULL:
		UpdateSpectatorFull(deltaSeconds);
		break;

	case CameraMode::SPECTATOR_XY:
		UpdateSpectatorXY(deltaSeconds);
		break;
	}
	// Update engine camera with final position and orientation
	if(m_mode!=CameraMode::INDEPENDENT)
		m_engineCamera.SetPositionAndOrientation(m_position, m_orientation);
}

void GameCamera::Render() const
{
	// GameCamera doesn't render itself
}

void GameCamera::SetMode(CameraMode mode)
{
	m_mode = mode;

	// Reset smooth velocity when switching modes
	m_positionVelocity = Vec3(0.f, 0.f, 0.f);
	m_distanceVelocity = 0.f;
}

void GameCamera::ToggleMode()
{
	// Cycle through modes
	int currentMode = static_cast<int>(m_mode);
	currentMode = (currentMode + 1) % static_cast<int>(CameraMode::SPECTATOR_XY) + 1;
	SetMode(static_cast<CameraMode>(currentMode));
}

void GameCamera::ProcessRotationInput(float yawDelta, float pitchDelta)
{
	switch (m_mode)
	{
	case CameraMode::FIRST_PERSON:
	case CameraMode::OVER_THE_SHOULDER:
		// In these modes, rotating the input rotates the TARGET entity
		// The camera follows and matches the target's orientation
		if (m_targetEntity)
		{
			EulerAngles targetOrientation = m_targetEntity->GetOrientation();
			targetOrientation.m_yawDegrees += yawDelta;
			targetOrientation.m_pitchDegrees += pitchDelta;
			targetOrientation.m_pitchDegrees =
				GetClamped(targetOrientation.m_pitchDegrees, m_minPitch, m_maxPitch);
			m_targetEntity->SetOrientation(targetOrientation);
		}
		break;

	case CameraMode::INDEPENDENT:
		// Rotate camera independently
		m_independentYaw += yawDelta;
		m_independentPitch += pitchDelta;
		m_independentPitch = GetClamped(m_independentPitch, m_minPitch, m_maxPitch);
		break;

	case CameraMode::SPECTATOR_FULL:
	case CameraMode::SPECTATOR_XY:
		// Rotate camera directly
	{
		EulerAngles cameraOrientation = GetOrientation();
		cameraOrientation.m_yawDegrees += yawDelta;
		cameraOrientation.m_pitchDegrees += pitchDelta;
		cameraOrientation.m_pitchDegrees =
			GetClamped(cameraOrientation.m_pitchDegrees, m_minPitch, m_maxPitch);
		SetOrientation(cameraOrientation);
	}
	break;
	}
}

void GameCamera::ProcessZoomInput(float zoomDelta)
{
	// Adjust distance for third-person modes
	switch (m_mode)
	{
	case CameraMode::INDEPENDENT:
		m_independentDistance += zoomDelta;
		m_independentDistance = GetClamped(m_independentDistance, m_minDistance, m_maxDistance);
		break;

	default:
		break;
	}
}

//----------------------------------------------------------------------
// Mode Update Functions
//----------------------------------------------------------------------

void GameCamera::UpdateFirstPerson(float deltaSeconds)
{
	// Camera at entity's eye position, looking where entity looks
	UNUSED(deltaSeconds);
	SetPosition(m_targetEntity->GetEyePosition());
}

void GameCamera::UpdateOverTheShoulder(float deltaSeconds)
{
	// Camera offset behind and to the side
	Vec3 forward, left, up;
	EulerAngles targetOrientation = m_targetEntity->GetOrientation();
	targetOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	Vec3 eyePos = m_targetEntity->GetEyePosition();
	Vec3 offset = -left * m_shoulderOffset.x + -forward * m_shoulderOffset.y + up * m_shoulderOffset.z;
	Vec3 idealCameraPos = eyePos + offset;

	// Raycast from eye to ideal camera position
	if (m_enableCollision && m_world)
	{
		Vec3 toCamera = idealCameraPos - eyePos;
		float desiredDistance = toCamera.GetLength();
		float actualDistance = CheckCameraCollision(eyePos, idealCameraPos);

		if (actualDistance < desiredDistance)
		{
			// Pull camera closer if blocked
			Vec3 direction = toCamera.GetNormalized();
			idealCameraPos = eyePos + direction * (actualDistance - m_cameraCollisionRadius);
		}
	}

	// Smooth follow
	Vec3 currentPos = GetPosition();
	SetPosition(SmoothDampPosition(currentPos, idealCameraPos, deltaSeconds));
}

//void GameCamera::UpdateFixedAngleTracking(float deltaSeconds)
//{
//	// Camera at fixed angle, always looking at entity
//	Vec3 eyePos = m_targetEntity->GetEyePosition();
//
//	// Calculate offset from spherical coordinates
//	float yawRad = m_fixedYaw * (3.14159f / 180.0f);
//	float pitchRad = m_fixedPitch * (3.14159f / 180.0f);
//
//	Vec3 offset;
//	offset.x = m_fixedDistance * CosDegrees(m_fixedPitch) * CosDegrees(m_fixedYaw);
//	offset.y = m_fixedDistance * CosDegrees(m_fixedPitch) * SinDegrees(m_fixedYaw);
//	offset.z = m_fixedDistance * SinDegrees(m_fixedPitch);
//
//	Vec3 targetPos = eyePos + offset;
//
//	// Smooth follow
//	Vec3 currentPos = GetPosition();
//	SetPosition(SmoothDampPosition(currentPos, targetPos, deltaSeconds));
//
//	// Always look at entity
//	Vec3 cameraPos = GetPosition();
//	Vec3 lookDir = (eyePos - cameraPos);
//	float distance = lookDir.GetLength();
//	if (distance > 0.001f)
//	{
//		lookDir /= distance;
//
//		// Calculate yaw and pitch from look direction
//		float yaw = Atan2Degrees(lookDir.y, lookDir.x);
//		float pitch = Atan2Degrees(lookDir.z, sqrtf(lookDir.x * lookDir.x + lookDir.y * lookDir.y));
//
//		SetOrientation(EulerAngles(yaw, pitch, 0.f));
//	}
//
//	// Check collision
//	if (m_enableCollision && m_world)
//	{
//		float actualDistance = CheckCameraCollision(eyePos, cameraPos);
//		if (actualDistance < distance)
//		{
//			SetPosition(eyePos + lookDir * actualDistance);
//		}
//	}
//}

void GameCamera::UpdateIndependent(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void GameCamera::UpdateSpectatorFull(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
}

void GameCamera::UpdateSpectatorXY(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
}

//----------------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------------

Vec3 GameCamera::SmoothDampPosition(const Vec3& current, const Vec3& target, float deltaTime)
{
	if (m_positionSmoothTime <= 0.f)
	{
		return target;
	}

	// 计算基础平滑因子
	float t = deltaTime / m_positionSmoothTime;
	t = GetClampedZeroToOne(t); // 确保 t 在 [0, 1] 范围内

	// 应用 SmoothStop3 曲线
	float smoothFactor = SmoothStop3(t);

	return Interpolate(current, target, smoothFactor);
}

float GameCamera::SmoothDampFloat(float current, float target, float deltaTime)
{
	if (m_distanceSmoothTime <= 0.f)
	{
		return target;
	}

	float t = deltaTime / m_distanceSmoothTime;
	t = GetClampedZeroToOne(t);
	float smoothFactor = SmoothStop3(t);

	return Interpolate(current, target, smoothFactor);
}

float GameCamera::CheckCameraCollision(const Vec3& from, const Vec3& to)
{
	if (!m_world)
	{
		Vec3 displacement = to - from;
		return displacement.GetLength();
	}

	Vec3 displacement = to - from;
	float maxDistance = displacement.GetLength();

	if (maxDistance < 0.001f)
	{
		return maxDistance;
	}

	Vec3 direction = displacement.GetNormalized();

	// Raycast from eye position toward camera position
	RaycastResult3D result = m_world->RaycastVsBlocks(from, direction, maxDistance);

	if (result.m_didImpact)
	{
		DebugAddWorldPoint(result.m_impactPos, 0.05f, 0.f, Rgba8::CYAN);
		// Hit a block, return distance to impact point
		return result.m_impactDist-m_cameraCollisionRadius;
	}

	// No collision, camera can be at full distance
	return maxDistance;
}