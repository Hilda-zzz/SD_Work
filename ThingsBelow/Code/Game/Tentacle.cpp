#include "Game/Tentacle.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicHermiteSpline2D.hpp"
#include <cmath>
#include "GameCommon.hpp"
#include "TentacleStateMachine.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "ITentacleContext.hpp"
#include "Food.hpp"

extern Window* g_theWindow;
extern InputSystem* g_theInput;

Tentacle::Tentacle()
	: m_targetPos(Vec2::ZERO)
	, m_rootPos(Vec2::ZERO)
	, m_isRootFixed(true)
	, m_maxAngleConstraintDegrees(360.0f)
	, m_waveEnabled(true)
	, m_waveFrequency(2.5f)
	, m_waveAmplitude(2.0f)
	, m_waveSpeed(3.0f)
	, m_waveTime(0.0f)
	, m_stateMachine(nullptr)
	, m_followRadius(300.0f)
	, m_currentState(nullptr)
	, m_pendingState(nullptr)
	, m_isTransitioning(false)
	, m_transitionTimer(0.0f)
	, m_transitionDuration(0.5f)
	, m_camera(nullptr)
	, m_context(nullptr)
	, m_lungeTargetPos(Vec2::ZERO)
	, m_lungeTargetFood(nullptr)
	, m_originalLength(0.0f)
{
	m_stateMachine = new TentacleStateMachine(this);
}

Tentacle::~Tentacle()
{
	delete m_stateMachine;
	m_stateMachine = nullptr;
}

void Tentacle::Initialize(int segmentCount, float totalLength, Vec2 const& rootPos)
{
	m_positions.clear();
	m_segmentLengths.clear();
	m_segments.clear();
	m_radiusCurve.clear();
	m_radiusControlPoints.clear();

	m_rootPos = rootPos;
	float segmentLength = totalLength / static_cast<float>(segmentCount);

	// Initialize positions (segmentCount + 1 positions for segmentCount segments)
	Vec2 currentPos = rootPos;
	for (int i = 0; i <= segmentCount; ++i)
	{
		m_positions.push_back(currentPos);
		m_radiusCurve.push_back(5.0f); // Default radius, will be updated
		currentPos = currentPos + Vec2(0.0f, -segmentLength);
	}

	// Initialize segment lengths
	for (int i = 0; i < segmentCount; ++i)
	{
		m_segmentLengths.push_back(segmentLength);
	}

	// Initialize 5 control points for radius curve (x=normalized position 0-1, y=radius)
	m_radiusControlPoints.push_back(Vec2(0.0f, 10.0f));   // Root: thick
	m_radiusControlPoints.push_back(Vec2(0.25f, 8.0f));
	m_radiusControlPoints.push_back(Vec2(0.5f, 5.0f));
	m_radiusControlPoints.push_back(Vec2(0.75f, 3.0f));
	m_radiusControlPoints.push_back(Vec2(1.0f, 1.5f));    // Tip: thin

	// Update radii from control points
	UpdateRadiusCurveFromControlPoints();

	// Build initial segments from positions
	UpdateSegmentsFromPositions();

	m_targetPos = m_positions.back();

	m_originalLength = totalLength;

	m_stateMachine->Initialize();
}

void Tentacle::Update(float deltaSeconds)
{
	//UpdateMouseInteraction(deltaSeconds);
	// Check mouse interaction and switch states
	m_stateMachine->Update(deltaSeconds);

	// state machine in charge of setting of m_targetPos
	// Step 1: Solve FABRIK on positions
	SolveFABRIK(2);
	ApplyAngleConstraints();
	// Step 2: Apply wave animation to positions
	if (m_waveEnabled)
	{
		ApplyWaveAnimation(deltaSeconds);
		ApplyAngleConstraints();
	}

	// Step 3: Rebuild segments from updated positions
	UpdateSegmentsFromPositions();
}

void Tentacle::Render() const
{
	for (const auto& segment : m_segments)
	{
		segment.Render();
	}

	for (int i = 0; i < (int)m_positions.size(); ++i)
	{
		//g_theRenderer->SetModelConstants();
		DebugDrawCircle(3.f, m_positions[i], Rgba8::BLUE);
	}

	// Draw target position
	DebugDrawCircle(6.0f, GetRealTipPos(), Rgba8::CYAN);
	DebugDrawCircle(4.0f, m_targetPos, Rgba8::RED);
	
}

void Tentacle::ChangeState(TentacleStateType newStateType)
{
	m_stateMachine->ChangeState(newStateType);
}

char const* Tentacle::GetCurrentStateName() const
{
	return m_stateMachine->GetCurrentStateName();
}

TentacleStateType Tentacle::GetCurrentStateType() const
{
	return m_stateMachine->GetCurrentStateType();
}

void Tentacle::SolveFABRIK(int iterations)
{
	if (m_positions.size() < 2)
		return;

	Vec2 originalRootPos = m_rootPos;

	for (int iter = 0; iter < iterations; ++iter)
	{
		// Forward reaching: start from target (tip)
		m_positions.back() = m_targetPos;

		for (int i = (int)m_positions.size() - 2; i >= 0; --i)
		{
			Vec2 direction = m_positions[i] - m_positions[i + 1];
			float currentLength = direction.GetLength();
			if (currentLength > 0.0f)
			{
				direction.Normalize();
				m_positions[i] = m_positions[i + 1] + direction * m_segmentLengths[i];
			}
		}

		// Backward reaching: restore root if fixed
		if (m_isRootFixed)
		{
			m_positions[0] = originalRootPos;
		}

		for (int i = 0; i < (int)m_positions.size() - 1; ++i)
		{
			Vec2 direction = m_positions[i + 1] - m_positions[i];
			float currentLength = direction.GetLength();
			if (currentLength > 0.0f)
			{
				direction.Normalize();
				m_positions[i + 1] = m_positions[i] + direction * m_segmentLengths[i];
			}
		}
	}
}
// =============================================================================
// Dynamic Length Control (for advanced states)
// =============================================================================
float Tentacle::GetTotalLength() const
{
	float total = 0.0f;
	for (float length : m_segmentLengths)
	{
		total += length;
	}
	return total;
}

void Tentacle::SetTotalLength(float newLength)
{
	// Proportionally scale all segment lengths
	float currentLength = GetTotalLength();
	if (currentLength > 0.0f)
	{
		float scale = newLength / currentLength;
		for (float& length : m_segmentLengths)
		{
			length *= scale;
		}
	}
}

void Tentacle::SetSegmentCount(int newCount)
{
	// This is a complex operation that requires:
	// 1. Recalculating segment lengths
	// 2. Redistributing positions
	// 3. Updating radius curve
	// Implementation depends on whether you want to preserve total length

	// Simple implementation: preserve total length, redistribute positions
	float totalLength = GetTotalLength();
	int oldCount = (int)m_segmentLengths.size();

	if (newCount == oldCount)
		return;

	// Store old positions
	std::vector<Vec2> oldPositions = m_positions;

	// Clear and rebuild
	m_positions.clear();
	m_segmentLengths.clear();
	m_radiusCurve.clear();

	float newSegmentLength = totalLength / static_cast<float>(newCount);

	// Rebuild positions along the old tentacle's curve
	m_positions.push_back(m_rootPos);

	for (int i = 1; i <= newCount; ++i)
	{
		// Interpolate position along old tentacle
		float t = static_cast<float>(i) / static_cast<float>(newCount);
		int oldIndex = static_cast<int>(t * (oldPositions.size() - 1));
		oldIndex = GetClamped(oldIndex, 0, (int)oldPositions.size() - 2);

		float localT = (t * (oldPositions.size() - 1)) - oldIndex;
		Vec2 newPos = Interpolate(oldPositions[oldIndex], oldPositions[oldIndex + 1], localT);

		m_positions.push_back(newPos);

		if (i < newCount)
		{
			m_segmentLengths.push_back(newSegmentLength);
		}
	}

	// Rebuild radius curve
	for (int i = 0; i <= newCount; ++i)
	{
		m_radiusCurve.push_back(5.0f);
	}

	UpdateRadiusCurveFromControlPoints();
	UpdateSegmentsFromPositions();
}

// =============================================================================
// Mouse Interaction
// =============================================================================
//void Tentacle::UpdateMouseInteraction(float deltaSeconds)
//{
//	UNUSED(deltaSeconds);
//
//	if (!m_camera)
//		return;
//
//	bool isPressed = IsMousePressed();
//	TentacleStateType currentStateType = GetCurrentStateType();
//
//	m_mouseWorldPos = CalculateCursorWorldPosition();
//
//	// Pressed: Switch to TrackTarget
//	if (isPressed && currentStateType == TentacleStateType::IDLE)
//	{
//		ChangeState(TentacleStateType::TRACK_TARGET);
//	}
//
//	// Pressed: Switch to TrackTarget
//	if (!isPressed && currentStateType == TentacleStateType::TRACK_TARGET)
//	{
//		ChangeState(TentacleStateType::IDLE);
//	}
//}
//
//bool Tentacle::IsMousePressed() const
//{
//	return g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
//}
//
Vec2 Tentacle::CalculateCursorWorldPosition() const
{
	if (!m_camera)
		return Vec2::ZERO;

	Vec2 mousePos = g_theInput->GetCursorClientPosition();
	Vec2 worldPos = m_camera->GetOrthoBottomLeft() +
		Vec2(mousePos.x / g_theWindow->GetClientDimensions().x,
			(g_theWindow->GetClientDimensions().y - mousePos.y) / g_theWindow->GetClientDimensions().y) *
		(m_camera->GetOrthoTopRight() - m_camera->GetOrthoBottomLeft());

	return worldPos;
}

void Tentacle::ApplyAngleConstraints()
{
	m_positions[0] = m_rootPos;

	for (int iter = 0; iter < 10; ++iter)  // constraint_iterations
	{
		for (int i = 0; i < (int)m_positions.size() - 1; ++i)
		{
			Vec2 current_vec = m_positions[i + 1] - m_positions[i];
			float distance = current_vec.GetLength();

			// 防止重叠
			if (distance < 0.0001f)
			{
				m_positions[i + 1] = m_positions[i] + Vec2(1.0f, 0.0f) * m_segmentLengths[i];
				continue;
			}

			// 计算误差
			Vec2 target_vec = current_vec.GetNormalized() * m_segmentLengths[i];
			Vec2 error_vec = target_vec - current_vec;

			// 双边校正（bilateral correction）
			if (i > 0)
			{
				m_positions[i] -= error_vec * 0.25f;
			}
			m_positions[i + 1] += error_vec * 0.25f;
		}

		m_positions[0] = m_rootPos;  // 每次迭代重新锚定
	}
}

void Tentacle::ApplyWaveAnimation(float deltaSeconds)
{
	if (m_positions.size() < 2)
		return;

	if (m_waveAmplitude <= 0.0f)
		return;

	m_waveTime += deltaSeconds * m_waveSpeed;

	// Calculate total length
	float total_length = 0.0f;
	for (float len : m_segmentLengths)
	{
		total_length += len;
	}

	float accumulated_length = 0.0f;

	// Skip first position (root), start from i=1
	for (int i = 1; i < (int)m_positions.size(); ++i)
	{
		accumulated_length += m_segmentLengths[i - 1];

		// Normalized position (0-1) along the arm determines wave phase offset
		float t = accumulated_length / total_length;

		// Get direction vector from previous position to current position
		Vec2 vec = m_positions[i] - m_positions[i - 1];
		Vec2 direction = vec.GetNormalized();
		Vec2 perpendicular = direction.GetRotated90Degrees();

		// Phase combines time (animation) with position (traveling wave)
		float wave_phase = m_waveTime - t * m_waveFrequency * 6.28318f; // TAU
		float wave_offset = sin(wave_phase) * m_waveAmplitude;

		// Apply offset to the position
		m_positions[i] += perpendicular * wave_offset;
	}
}

void Tentacle::UpdateSegmentsFromPositions()
{
	m_segments.clear();

	for (int i = 0; i < (int)m_positions.size() - 1; ++i)
	{

		Vec2 startPos = m_positions[i];
		Vec2 endPos = m_positions[i + 1];
		float startRadius = GetRadiusAtPosition(i);
		float endRadius = GetRadiusAtPosition(i + 1);

		m_segments.push_back(TentacleSegment(startPos, endPos, startRadius, endRadius));
	}
}

void Tentacle::SetRadiusCurve(const std::vector<float>& radiusValues)
{
	m_radiusCurve = radiusValues;
	UpdateSegmentsFromPositions();
}

void Tentacle::SetRadiusControlPoints(const std::vector<Vec2>& controlPoints)
{
	m_radiusControlPoints = controlPoints;
	UpdateRadiusCurveFromControlPoints();
}

void Tentacle::UpdateRadiusCurveFromControlPoints()
{
	if (m_radiusControlPoints.size() < 2 || m_radiusCurve.empty())
		return;

	// Use CubicHermiteSpline2D for smooth interpolation
	CubicHermiteSpline2D spline(m_radiusControlPoints);

	int numPoints = (int)m_radiusCurve.size();
	for (int i = 0; i < numPoints; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
		Vec2 point = spline.EvaluateAtParametric(t);
		m_radiusCurve[i] = point.y;  // Use y component as radius
	}

	UpdateSegmentsFromPositions();
}

void Tentacle::UpdateSegmentRadii()
{
	UpdateSegmentsFromPositions();
}

float Tentacle::GetRadiusAtPosition(int positionIndex) const
{
	if (m_radiusCurve.empty())
		return 1.0f;

	if (positionIndex < 0)
		return m_radiusCurve.front();

	if (positionIndex >= (int)m_radiusCurve.size())
		return m_radiusCurve.back();

	return m_radiusCurve[positionIndex];
}

// =============================================================================
// Lunge State Data
// =============================================================================

void Tentacle::SetLungeTarget(Vec2 const& targetPos, Food* targetFood)
{
	m_lungeTargetPos = targetPos;
	m_lungeTargetFood = targetFood;
}