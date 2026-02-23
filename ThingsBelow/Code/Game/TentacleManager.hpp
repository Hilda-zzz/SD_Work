#pragma once
#include "ITentacleContext.hpp"
#include "GlobalEvent.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

class Tentacle;
class Camera;
class Food;

// ============================================================================
// Active Events Structure - 当前活跃的所有事件
// ============================================================================
struct ActiveEvents
{
	bool mouseTracking;                  // 鼠标是否按下
	std::vector<Food*> droppingFoods;    // 掉落中的食物列表

	// TODO Phase 3
	// bool mouseHovering;
	// Food* heldFood;
	// LightBall* heldBall;

	ActiveEvents()
		: mouseTracking(false)
	{
	}
};

// ============================================================================
// Tentacle Task - 触手任务记录
// ============================================================================
struct TentacleTask
{
	Tentacle* tentacle;           // 执行的触手
	GlobalEvent eventType;        // 任务类型
	void* taskData;               // 任务数据（如 Food 指针）
	int priority;                 // 任务优先级
	bool canInterrupt;            // 是否可被中断

	TentacleTask()
		: tentacle(nullptr)
		, eventType(GlobalEvent::NONE)
		, taskData(nullptr)
		, priority(20)
		, canInterrupt(true)
	{
	}
};

// ============================================================================
// TentacleManager - 触手群体管理器
// ============================================================================
// 职责：
// 1. 检测全局事件（鼠标、食物、光球等）
// 2. 优先级管理和任务分配
// 3. 触手池管理（Leader, Resident, Temporary）
// 4. 实现 ITentacleContext 接口，提供全局数据访问
// ============================================================================

class TentacleManager : public ITentacleContext
{
public:
	TentacleManager();
	~TentacleManager();

	void Initialize(Camera const* camera, Vec2 const& wellPosition);
	void Update(float deltaSeconds);
	void Render() const;
	void Shutdown();

	// ========================================================================
	// Tentacle Creation and Destruction
	// ========================================================================

	/// Create and set the leader tentacle
	void CreateLeaderTentacle(int segmentCount, float totalLength, Vec2 const& rootPos);

	/// Create and add a resident tentacle
	void AddNewResidentTentacle(int segmentCount, float totalLength, Vec2 const& rootPos);

	/// Clean up all tentacles
	void DestroyAllTentacles();

	// ========================================================================
	// Tentacle Management
	// ========================================================================
	
	/// Set the leader tentacle (only one)
	void SetLeaderTentacle(Tentacle* tentacle);
	
	/// Add a resident tentacle (multiple allowed)
	void AddResidentTentacle(Tentacle* tentacle);
	
	/// Get leader tentacle
	Tentacle* GetLeaderTentacle() const { return m_leaderTentacle; }
	
	/// Get all resident tentacles
	const std::vector<Tentacle*>& GetResidentTentacles() const { return m_residentTentacles; }

	// ========================================================================
	// ITentacleContext Interface Implementation
	// ========================================================================
	
	virtual Vec2 GetMouseWorldPosition() const override { return m_mouseWorldPos; }
	virtual Camera const* GetCamera() const override { return m_camera; }

	// TODO Phase 3: Idle behavior request system
	// virtual bool RequestIdleBehavior(Tentacle* requester, IdleBehaviorType type) override;
	
	// ========================================================================
	// External Event Data (called by Game)
	// ========================================================================

	/// 设置掉落中的食物列表（每帧由 Game 调用）
	void SetDroppingFoods(const std::vector<Food*>& foods);


private:
	// ========================================================================
	// Event Detection
	// ========================================================================
	
	void UpdateMouseState(float deltaSeconds);
	bool IsMousePressed() const;
	Vec2 CalculateMouseWorldPosition() const;

	// ========================================================================
	// Tentacle Updates
	// ========================================================================
	
	void UpdateAllTentacles(float deltaSeconds);

	// ========================================================================
	// Multi-Event System
	// ========================================================================

	/// 检测所有当前活跃的事件
	ActiveEvents DetectAllActiveEvents();

	/// 处理所有活跃事件（分配任务）
	void ProcessEvents(const ActiveEvents& events);

	/// 清理已完成的任务
	void CleanupCompletedTasks();

	/// 检查触手是否可用
	bool IsTentacleAvailable(Tentacle* tentacle) const;

	/// 获取空闲触手
	Tentacle* GetAvailableTentacle();

	/// 为食物分配触手
	void AssignFoodCapture(Food* food);

private:
	// World data
	Camera const* m_camera;
	Vec2 m_wellPosition;

	// Tentacle pool
	Tentacle* m_leaderTentacle;
	std::vector<Tentacle*> m_residentTentacles;
	std::vector<Tentacle*> m_temporaryTentacles;  // TODO Phase 3

	// Mouse tracking state
	Vec2 m_mouseWorldPos;
	bool m_isMousePressed;
	bool m_wasMousePressed;  // For detecting press/release transitions
	
	// Event state
	ActiveEvents m_activeEvents;              // 当前活跃的事件
	std::vector<TentacleTask> m_activeTasks;  // 当前活跃的任务

	// Food tracking
	std::vector<Food*> m_droppingFoods;  // 从 Game 获取的掉落食物列表

	// TODO Phase 3: Food tracking
	// std::vector<Food*> m_trackedFoods;  // 已经分配任务的食物
	
	// TODO Phase 3: Mouse hovering
	// float m_mouseHoverTimer;
	// Vec2 m_lastMousePos;
};
