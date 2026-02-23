#include "Game/TentacleStates.hpp"
#include "Game/Tentacle.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Core/EngineCommon.hpp>
#include <ThirdParty/Noise/SmoothNoise.hpp>
#include <Engine/Math/Easing.hpp>

//-----------------------------------------------------------------------------------------------
// IdleSwayState
//-----------------------------------------------------------------------------------------------
IdleSwayState::IdleSwayState(Tentacle* owner, float swayRadius, float swaySpeed, unsigned int seedX, unsigned int seedY)
	: TentacleState(owner)
	, m_swayTime(0.0f)
	, m_swayRadius(swayRadius)
	, m_swaySpeed(swaySpeed)
	, m_noiseSeedX(seedX)
	, m_noiseSeedY(seedY)
	, m_currentTarget(Vec2::ZERO)
	, m_nextTarget(Vec2::ZERO)
	, m_targetChangeTimer(0.0f)
	, m_targetChangeDuration(2.0f)  // 每 2 秒换一次目标
{

}

void IdleSwayState::OnEnter()
{
	m_swayTime = 0.0f;
	m_targetChangeTimer = 0.0f;

	// Start from current FABRIK target (avoid jump when entering from other states)
	m_currentTarget = m_owner->GetRealTipPos();

	// Generate the first random target
	GenerateNewTarget();
}

void IdleSwayState::Update(float deltaSeconds)
{
	m_swayTime += deltaSeconds;
	m_targetChangeTimer += deltaSeconds;

	// 到达切换时间，生成新目标
	if (m_targetChangeTimer >= m_targetChangeDuration)
	{
		m_currentTarget = m_nextTarget;
		GenerateNewTarget();
		m_targetChangeTimer = 0.0f;

		// 随机化下次切换时间（1-3 秒）
		m_targetChangeDuration = m_rng.RollRandomFloatInRange(3.0f, 5.0f);
	}

	// 平滑插值到下一个目标
	float t = m_targetChangeTimer / m_targetChangeDuration;
	t = SmoothStep3(t);  // 缓入缓出

	Vec2 smoothTarget = Interpolate(m_currentTarget, m_nextTarget, t);
	m_owner->SetFABRIKTarget(smoothTarget);
}

void IdleSwayState::OnExit()
{
}

void IdleSwayState::GenerateNewTarget()
{
	Vec2 rootPos = m_owner->GetRootPos();
	float totalLength = m_owner->GetTotalLength();

	// 在圆环区域内随机选点
	float minRadius = totalLength * 0.2f;
	float maxRadius = totalLength * 0.9f;

	float randomAngle = m_rng.RollRandomFloatInRange(0.0f, 360.0f);
	float randomRadius = m_rng.RollRandomFloatInRange(minRadius, maxRadius);

	Vec2 offset = Vec2::MakeFromPolarDegrees(randomAngle, randomRadius);
	m_nextTarget = rootPos + offset;
}

//-----------------------------------------------------------------------------------------------
// TrackTargetState
//-----------------------------------------------------------------------------------------------
TrackTargetState::TrackTargetState(Tentacle* owner)
	: TentacleState(owner)
	, m_smoothTarget(Vec2::ZERO)
	, m_lerpSpeed(5.0f)  // Default: medium response
{
}

void TrackTargetState::OnEnter()
{
	// Initialize smooth target to current tip position (avoid jump)
	m_smoothTarget = m_owner->GetRealTipPos();
}

void TrackTargetState::Update(float deltaSeconds)
{
	// Read mouse position from owner
	Vec2 mousePos = m_owner->GetMouseWorldPosition();

	// Smoothly interpolate smooth target towards mouse position
	float alpha = 1.0f - expf(-m_lerpSpeed * deltaSeconds);
	m_smoothTarget = Interpolate(m_smoothTarget, mousePos, alpha);

	// Set tentacle's FABRIK target to smooth target
	m_owner->SetFABRIKTarget(m_smoothTarget);
}

void TrackTargetState::OnExit()
{
	// Clean exit
}