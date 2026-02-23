#include "LungeState.hpp"
#include "Tentacle.hpp"
#include "Food.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Easing.hpp"

LungeState::LungeState(Tentacle* owner)
	: TentacleState(owner)
	, m_currentPhase(LungePhase::EXTEND)
	, m_targetPos(Vec2::ZERO)
	, m_targetFood(nullptr)
	, m_retractTarget(Vec2::ZERO)
	, m_originalLength(0.0f)
	, m_currentLength(0.0f)
	, m_contactTimer(0.0f)
	, m_retractTimer(0.0f)
	, m_insideRetractZone(false)
	, m_extendSpeed(200.0f)           // 默认：快速伸长
	, m_retractSpeed(150.0f)          // 默认：稍慢收回
	, m_contactDuration(0.25f)        // 默认：接触 0.25 秒
	, m_reachThreshold(10.0f)         // 默认：10 单位内算到达
	, m_retractRadius(100.0f)         // 默认：100 单位内算收回区域
	, m_retractCompleteDuration(2.f) // 默认：在区域内停留 0.3 秒完成
	, m_retractStartPos(Vec2::ZERO)
	, m_retractProgress(0.0f)
	, m_extendProgress(0.0f)
	, m_targetLength(0.0f)
	, m_retractStartLength(0.0f)
{
}

void LungeState::OnEnter()
{
	// 读取目标数据（由 Tentacle::SetLungeTarget 设置）
	m_targetPos = m_owner->GetLungeTargetPos();
	m_targetFood = m_owner->GetLungeTargetFood();

	// 记录原始长度
	m_originalLength = m_owner->GetOriginalLength();
	m_currentLength = m_originalLength;

	// 初始化为 EXTEND 阶段
	m_currentPhase = LungePhase::EXTEND;
	m_contactTimer = 0.0f;
	m_retractTimer = 0.0f;
	m_insideRetractZone = false;

	m_extendProgress = 0.0f;
	m_targetLength = m_originalLength;
}

void LungeState::Update(float deltaSeconds)
{
	// 安全检查：食物被删除或已被其他触手抓走
	if (!m_targetFood)
	{
		if (m_currentPhase != LungePhase::RETRACT)
		{
			m_currentPhase = LungePhase::RETRACT;

			// 记录起点
			m_retractStartPos = m_owner->GetRealTipPos();

			m_retractStartLength = m_currentLength;

			// 随机终点
			Vec2 rootPos = m_owner->GetRootPos();
			float randomAngle = m_rng.RollRandomFloatInRange(0.0f, 360.0f);
			float randomRadius = m_rng.RollRandomFloatInRange(30.0f, 80.0f);
			Vec2 randomOffset = Vec2::MakeFromPolarDegrees(randomAngle, randomRadius);
			m_retractTarget = rootPos + randomOffset;

			m_retractProgress = 0.0f;
			m_retractTimer = 0.0f;
			m_insideRetractZone = false;
		}
	}

	// 根据当前阶段更新
	switch (m_currentPhase)
	{
	case LungePhase::EXTEND:
		UpdateExtend(deltaSeconds);
		break;

	case LungePhase::CONTACT:
		UpdateContact(deltaSeconds);
		break;

	case LungePhase::RETRACT:
		UpdateRetract(deltaSeconds);
		break;
	}
}

void LungeState::OnExit()
{
	// 恢复原始长度
	m_owner->SetTotalLength(m_originalLength);
}

// ============================================================================
// 各阶段更新实现
// ============================================================================

void LungeState::UpdateExtend(float deltaSeconds)
{
	// 更新目标位置（食物可能在移动/掉落）
	if (m_targetFood)
	{
		m_targetPos = m_targetFood->GetPosition();
	}

	Vec2 rootPos = m_owner->GetRootPos();
	Vec2 tipPos = m_owner->GetRealTipPos();

	// 计算需要的目标长度
	float distanceRootToFood = (m_targetPos - rootPos).GetLength();
	m_targetLength = distanceRootToFood * 1.f;  // 留 20% 余量

	// 增加进度（控制伸长速度）
	float extendProgressSpeed = 1.5f;  // 进度增长速度（可调参数）
	m_extendProgress += extendProgressSpeed * deltaSeconds;
	m_extendProgress = GetClamped(m_extendProgress, 0.0f, 1.0f);

	// 使用 SmoothStep 插值长度（缓入缓出）
	float t = EaseOutCubic(m_extendProgress);

	m_currentLength = Interpolate(m_originalLength, m_targetLength, t);
	m_owner->SetTotalLength(m_currentLength);

	// 设置 FABRIK 目标为食物位置
	m_owner->SetFABRIKTarget(m_targetPos);

	// 检查是否到达
	float distanceToFood = (m_targetPos - tipPos).GetLength();

	if (distanceToFood < m_reachThreshold)
	{
		// 到达食物，进入接触阶段
		m_currentPhase = LungePhase::CONTACT;
		m_contactTimer = 0.0f;
	}
}

void LungeState::UpdateContact(float deltaSeconds)
{
	// 尖端跟随食物位置（食物可能还在掉落）
	if (m_targetFood)
	{
		m_targetPos = m_targetFood->GetPosition();
	}

	m_owner->SetFABRIKTarget(m_targetPos);

	// 接触计时
	m_contactTimer += deltaSeconds;

	if (m_contactTimer >= m_contactDuration)
	{
		// 接触时间足够，标记食物被抓
		if (m_targetFood)
		{
			m_targetFood->SetState(ObjectState::CAPTURED);
		}

		// 进入收回阶段
		m_currentPhase = LungePhase::RETRACT;

		// 记录当前尖端位置作为起点
		m_retractStartPos = m_owner->GetRealTipPos();

		m_retractStartLength = m_currentLength;

		// 随机选择一个终点（在根部附近的小半径内）
		Vec2 rootPos = m_owner->GetRootPos();
		float randomAngle = m_rng.RollRandomFloatInRange(0.0f, 360.0f);
		float randomRadius = m_rng.RollRandomFloatInRange(30.0f, 80.0f);  // 30-80 单位范围
		Vec2 randomOffset = Vec2::MakeFromPolarDegrees(randomAngle, randomRadius);
		m_retractTarget = rootPos + randomOffset;

		// 初始化收回状态
		m_retractProgress = 0.0f;
		m_retractTimer = 0.0f;
		m_insideRetractZone = false;
	}
}

void LungeState::UpdateRetract(float deltaSeconds)
{
	// 增加收回进度
	float retractSpeed = 1.0f;  // 进度增长速度（可调参数）
	m_retractProgress += retractSpeed * deltaSeconds;
	m_retractProgress = GetClamped(m_retractProgress, 0.0f, 1.0f);

	// 使用 SmoothStep 计算插值参数
	float t = SmoothStep3(m_retractProgress);  // 缓入缓出

	// 1. 插值长度（与位置同步）
	m_currentLength = Interpolate(m_retractStartLength, m_originalLength, t);
	m_owner->SetTotalLength(m_currentLength);

	// 2. 插值尖端目标位置
	Vec2 currentTarget = Interpolate(m_retractStartPos, m_retractTarget, t);
	m_owner->SetFABRIKTarget(currentTarget);

	// 3. 食物跟随尖端
	if (m_targetFood)
	{
		m_targetFood->SetPosition(m_owner->GetRealTipPos());
	}

	// 4. 检查是否进入收回半径范围
	Vec2 tipPos = m_owner->GetRealTipPos();
	Vec2 rootPos = m_owner->GetRootPos();
	float distanceToRoot = (tipPos - rootPos).GetLength();

	if (distanceToRoot <= m_retractRadius)
	{
		// 在收回范围内
		if (!m_insideRetractZone)
		{
			// 刚进入范围，开始计时
			m_insideRetractZone = true;
			m_retractTimer = 0.0f;
		}

		m_retractTimer += deltaSeconds;

		if (m_retractTimer >= m_retractCompleteDuration)
		{
			// 在范围内停留足够时间，完成！
			m_owner->ChangeState(TentacleStateType::IDLE);
		}
	}
	else
	{
		// 离开了范围，重置计时
		m_insideRetractZone = false;
		m_retractTimer = 0.0f;
	}
}