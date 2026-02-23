#pragma once
#include "TentacleSegment.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include "TentacleStateType.hpp"

class Camera;
class TentacleStateMachine;
class TentacleState;
class ITentacleContext;
class Food;

class Tentacle
{
public:
	Tentacle();
	~Tentacle();

	void Initialize(int segmentCount, float totalLength, Vec2 const& rootPos);

	void SetContext(ITentacleContext* context) { m_context = context; }
	ITentacleContext* GetContext() const { return m_context; }

	void Update(float deltaSeconds);
	void Render() const;

	// =========================================================================
	// Camera Setup (REQUIRED for cursor following)
	// =========================================================================
	void SetCamera(Camera const* camera) { m_camera = camera; }
	Camera const* GetCamera() const { return m_camera; }

	// =========================================================================
	// State Machine Integration
	// =========================================================================
	void ChangeState(TentacleStateType newStateType);
	char const* GetCurrentStateName() const;
	TentacleState* GetCurrentState() const { return m_currentState; }
	TentacleStateType GetCurrentStateType() const;

	// ================ FABRIK - operates on positions =========================
	void SetFABRIKTarget(Vec2 const& target) { m_targetPos = target; }
	Vec2 GetFABRIKTarget() const { return m_targetPos; }
	void SolveFABRIK(int iterations = 2);
	Vec2 GetRealTipPos() const { return m_segments.back().GetEndPos(); }

	// Mouse position (will be generalized later in Phase 2)
	void SetMouseWorldPosition(Vec2 const& pos) { m_mouseWorldPos = pos; }
	Vec2 GetMouseWorldPosition() const { return m_mouseWorldPos; }

	// Configuration
	void SetRootFixed(bool fixed) { m_isRootFixed = fixed; }
	bool IsRootFixed() const { return m_isRootFixed; }

	void SetAngleConstraint(float maxAngleDegrees) { m_maxAngleConstraintDegrees = maxAngleDegrees; }
	float GetAngleConstraint() const { return m_maxAngleConstraintDegrees; }

	// Wave animation
	void SetWaveEnabled(bool enabled) { m_waveEnabled = enabled; }
	bool IsWaveEnabled() const { return m_waveEnabled; }

	void SetWaveFrequency(float frequency) { m_waveFrequency = frequency; }
	float GetWaveFrequency() const { return m_waveFrequency; }

	void SetWaveAmplitude(float amplitude) { m_waveAmplitude = amplitude; }
	float GetWaveAmplitude() const { return m_waveAmplitude; }

	void SetWaveSpeed(float speed) { m_waveSpeed = speed; }
	float GetWaveSpeed() const { return m_waveSpeed; }

	// Radius curve
	void SetRadiusCurve(const std::vector<float>& radiusValues);
	std::vector<float>& GetRadiusCurveReference() { return m_radiusCurve; }
	void UpdateSegmentRadii();

	// Control points for radius curve editing
	void SetRadiusControlPoints(const std::vector<Vec2>& controlPoints);
	std::vector<Vec2>& GetRadiusControlPointsReference() { return m_radiusControlPoints; }
	void UpdateRadiusCurveFromControlPoints();

	// Accessors
	int GetSegmentCount() const { return (int)m_segments.size(); }
	int GetPositionCount() const { return (int)m_positions.size(); }
	Vec2 GetRootPos() const { return m_rootPos; };
	std::vector<TentacleSegment>& GetSegments() { return m_segments; }
	std::vector<Vec2>& GetPositions() { return m_positions; }
	

	// =========================================================================
	// Dynamic Length Control (for Lunge and Spline Follow states)
	// =========================================================================
	float GetTotalLength() const;
	void SetTotalLength(float newLength);  // Dynamically adjust segment lengths
	void SetSegmentCount(int newCount);    // Dynamically adjust number of segments

	// =========================================================================
	// Lunge State Data
	// =========================================================================
	void SetLungeTarget(Vec2 const& targetPos, Food* targetFood);
	Vec2 GetLungeTargetPos() const { return m_lungeTargetPos; }
	Food* GetLungeTargetFood() const { return m_lungeTargetFood; }

	float GetOriginalLength() const { return m_originalLength; }
	void StoreOriginalLength() { m_originalLength = GetTotalLength(); }

private:
	void ApplyAngleConstraints();
	void ApplyWaveAnimation(float deltaSeconds);
	void UpdateSegmentsFromPositions();
	float GetRadiusAtPosition(int positionIndex) const;

	//// Mouse interaction detection
	//void UpdateMouseInteraction(float deltaSeconds);
	//bool IsMousePressed() const;

	Vec2 CalculateCursorWorldPosition() const;

	void TransitionToIdle();
	void UpdateTransition(float deltaSeconds);
	bool IsInIdleState() const;

private:
	Camera const* m_camera;

	ITentacleContext* m_context;  // Interface to access global data from Manager

	Vec2 m_mouseWorldPos;
	// states
	TentacleStateMachine* m_stateMachine;
	TentacleState* m_currentState;
	TentacleState* m_pendingState;
	bool m_isTransitioning;
	float m_transitionTimer;
	float m_transitionDuration;

	// Core data - positions are the source of truth
	std::vector<Vec2> m_positions;              // Joint/control points (FABRIK operates on these)
	std::vector<float> m_segmentLengths;        // Original length of each segment (for FABRIK)
	float m_followRadius;

	// Rendering data - derived from positions
	std::vector<TentacleSegment> m_segments;    // Visual segments built from positions
	std::vector<float> m_radiusCurve;           // Radius values for each position
	std::vector<Vec2> m_radiusControlPoints;    // Control points for curve editing

	Vec2 m_targetPos;
	Vec2 m_rootPos;

	// Configuration
	bool m_isRootFixed;
	float m_maxAngleConstraintDegrees;

	// Wave animation
	bool m_waveEnabled;
	float m_waveFrequency;
	float m_waveAmplitude;
	float m_waveSpeed;
	float m_waveTime;

	// Lunge state data
	Vec2 m_lungeTargetPos;
	Food* m_lungeTargetFood;
	float m_originalLength;  // 记录初始长度，用于 Lunge 恢复
};