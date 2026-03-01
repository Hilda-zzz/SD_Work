#include "TentacleManager.hpp"
#include "Tentacle.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "GameCommon.hpp"
#include "Food.hpp"

extern InputSystem* g_theInput;
extern Window* g_theWindow;

TentacleManager::TentacleManager()
	: m_camera(nullptr)
	, m_wellPosition(Vec2::ZERO)
	, m_leaderTentacle(nullptr)
	, m_mouseWorldPos(Vec2::ZERO)
	, m_isMousePressed(false)
	, m_wasMousePressed(false)
	, m_droppingFoods()
{
}

TentacleManager::~TentacleManager()
{
	Shutdown();
}

void TentacleManager::Initialize(Camera const* camera, Vec2 const& wellPosition)
{
	m_camera = camera;
	m_wellPosition = wellPosition;
}

void TentacleManager::Update(float deltaSeconds)
{
	// Step 1: Update mouse state
	UpdateMouseState(deltaSeconds);

	// Step 2: Detect all active events
	ActiveEvents newEvents = DetectAllActiveEvents();

	ProcessEvents(newEvents);

	// Step 4: Update all tentacles (they execute assigned states)
	UpdateAllTentacles(deltaSeconds);
	// Step 3: Process events (assign tasks to tentacles)

	// Step 5: Cleanup completed tasks
	CleanupCompletedTasks();

	// Step 6: Store previous mouse state for next frame
	m_wasMousePressed = m_isMousePressed;
}

void TentacleManager::Render() const
{
	// Render all tentacles
	if (m_leaderTentacle)
	{
		m_leaderTentacle->Render();
	}

	for (const Tentacle* tentacle : m_residentTentacles)
	{
		tentacle->Render();
	}

	for (const Tentacle* tentacle : m_temporaryTentacles)
	{
		tentacle->Render();
	}
}

void TentacleManager::Shutdown()
{
	DestroyAllTentacles();
}

// ============================================================================
// Tentacle Management
// ============================================================================

void TentacleManager::SetLeaderTentacle(Tentacle* tentacle)
{
	m_leaderTentacle = tentacle;
	if (m_leaderTentacle && m_camera)
	{
		m_leaderTentacle->SetContext(this);
		m_leaderTentacle->SetCamera(m_camera);
	}
}

void TentacleManager::AddResidentTentacle(Tentacle* tentacle)
{
	m_residentTentacles.push_back(tentacle);
	if (tentacle && m_camera)
	{
		tentacle->SetContext(this);
		tentacle->SetCamera(m_camera);
	}
}

// ============================================================================
// Event Detection
// ============================================================================

void TentacleManager::SetDroppingFoods(const std::vector<Food*>& foods)
{
	m_droppingFoods = foods;
}

void TentacleManager::UpdateMouseState(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	
	if (!m_camera)
		return;

	// Update mouse position
	m_mouseWorldPos = CalculateMouseWorldPosition();

	// Update pressed state
	m_isMousePressed = IsMousePressed();
}

bool TentacleManager::IsMousePressed() const
{
	return g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
}

Vec2 TentacleManager::CalculateMouseWorldPosition() const
{
	if (!m_camera)
		return Vec2::ZERO;

	Vec2 mousePos = g_theInput->GetCursorClientPosition();
	IntVec2 windowDims = g_theWindow->GetClientDimensions();

	Vec2 worldPos = m_camera->GetOrthoBottomLeft() +
		Vec2(mousePos.x / windowDims.x,
			(windowDims.y - mousePos.y) / windowDims.y) *
		(m_camera->GetOrthoTopRight() - m_camera->GetOrthoBottomLeft());

	return worldPos;
}

// ============================================================================
// Tentacle Updates
// ============================================================================

void TentacleManager::UpdateAllTentacles(float deltaSeconds)
{
	// Update leader
	if (m_leaderTentacle)
	{
		m_leaderTentacle->Update(deltaSeconds);
	}

	// Update residents
	for (Tentacle* tentacle : m_residentTentacles)
	{
		tentacle->Update(deltaSeconds);
	}

	// Update temporary tentacles (Phase 3)
	for (Tentacle* tentacle : m_temporaryTentacles)
	{
		tentacle->Update(deltaSeconds);
	}

	// TODO Phase 3: Cleanup completed temporary tentacles
}

// ============================================================================
// Tentacle Creation and Destruction
// ============================================================================

void TentacleManager::CreateLeaderTentacle(int segmentCount, float totalLength, Vec2 const& rootPos)
{
	// Delete existing leader if any
	if (m_leaderTentacle)
	{
		delete m_leaderTentacle;
		m_leaderTentacle = nullptr;
	}

	// Create new leader
	m_leaderTentacle = new Tentacle();
	m_leaderTentacle->Initialize(segmentCount, totalLength, rootPos);
	m_leaderTentacle->SetContext(this);

	m_residentTentacles.push_back(m_leaderTentacle);

	if (m_camera)
	{
		m_leaderTentacle->SetCamera(m_camera);
	}
}

void TentacleManager::AddNewResidentTentacle(int segmentCount, float totalLength, Vec2 const& rootPos)
{
	Tentacle* resident = new Tentacle();
	resident->Initialize(segmentCount, totalLength, rootPos);
	resident->SetContext(this);

	if (m_camera)
	{
		resident->SetCamera(m_camera);
	}

	m_residentTentacles.push_back(resident);
}

void TentacleManager::DestroyAllTentacles()
{
	// Delete residents
	for (Tentacle* tentacle : m_residentTentacles)
	{
		delete tentacle;
	}
	m_residentTentacles.clear();

	// Delete temporary tentacles (Phase 3)
	for (Tentacle* tentacle : m_temporaryTentacles)
	{
		delete tentacle;
	}
	m_temporaryTentacles.clear();
}

// ============================================================================
// Multi-Event System Implementation
// ============================================================================

ActiveEvents TentacleManager::DetectAllActiveEvents()
{
	ActiveEvents events;

	// 检测鼠标跟随
	events.mouseTracking = m_isMousePressed;

	// 使用从 Game 传入的掉落食物列表
	events.droppingFoods = m_droppingFoods;

	return events;
}

void TentacleManager::ProcessEvents(const ActiveEvents& events)
{
	// ====================================================================
	// 处理鼠标跟随事件
	// ====================================================================

	if (events.mouseTracking)
	{
		// 检查 Leader 是否已经在跟随
		bool leaderTracking = false;
		for (const auto& task : m_activeTasks)
		{
			if (task.tentacle == m_leaderTentacle &&
				task.eventType == GlobalEvent::MOUSE_TRACKING)
			{
				leaderTracking = true;
				break;
			}
		}

		// 如果 Leader 还没开始跟随，分配任务
		if (!leaderTracking && IsTentacleAvailable(m_leaderTentacle))
		{
			m_leaderTentacle->SetMouseWorldPosition(m_mouseWorldPos);
			m_leaderTentacle->ChangeState(TentacleStateType::TRACK_TARGET);

			// 添加任务记录
			TentacleTask task;
			task.tentacle = m_leaderTentacle;
			task.eventType = GlobalEvent::MOUSE_TRACKING;
			task.priority = GetEventPriority(GlobalEvent::MOUSE_TRACKING);
			task.canInterrupt = true;
			m_activeTasks.push_back(task);
		}
		else if (leaderTracking)
		{
			// 持续更新鼠标位置
			m_leaderTentacle->SetMouseWorldPosition(m_mouseWorldPos);
		}
	}
	else
	{
		// 鼠标松开，移除鼠标跟随任务
		for (auto it = m_activeTasks.begin(); it != m_activeTasks.end(); )
		{
			if (it->eventType == GlobalEvent::MOUSE_TRACKING)
			{
				// 让触手回到 Idle
				it->tentacle->ChangeState(TentacleStateType::IDLE);
				it = m_activeTasks.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	// ====================================================================
	// 处理食物掉落事件
	// ====================================================================

	for (Food* food : events.droppingFoods)
	{
		// 检查这个食物是否已经被处理
		bool alreadyHandled = false;
		for (const auto& task : m_activeTasks)
		{
			if (task.eventType == GlobalEvent::DROPPING_FOOD &&
				task.taskData == food)
			{
				alreadyHandled = true;
				break;
			}
		}

		// 新食物，需要分配触手
		if (!alreadyHandled)
		{
			AssignFoodCapture(food);
		}
	}
}

void TentacleManager::AssignFoodCapture(Food* food)
{
	// 获取一个可用的触手
	Tentacle* availableTentacle = GetAvailableTentacle();

	if (!availableTentacle)
	{
		// TODO Phase 3: 生成临时触手
		// availableTentacle = SpawnTemporaryTentacle();
		return;  // 暂时没有可用触手，跳过
	}

	// 设置 Lunge 目标
	availableTentacle->SetLungeTarget(food->GetPosition(), food);

	// 切换到 LUNGE 状态
	availableTentacle->ChangeState(TentacleStateType::LUNGE);

	// 添加任务记录
	TentacleTask task;
	task.tentacle = availableTentacle;
	task.eventType = GlobalEvent::DROPPING_FOOD;
	task.taskData = food;
	task.priority = GetEventPriority(GlobalEvent::DROPPING_FOOD);
	task.canInterrupt = false;  // 抓食物不可中断
	m_activeTasks.push_back(task);
}

bool TentacleManager::IsTentacleAvailable(Tentacle* tentacle) const
{
	if (!tentacle)
		return false;

	// 检查是否已经有任务
	for (const auto& task : m_activeTasks)
	{
		if (task.tentacle == tentacle)
		{
			// 已经有任务，检查是否可中断
			return task.canInterrupt;
		}
	}

	return true;  // 没有任务，可用
}

Tentacle* TentacleManager::GetAvailableTentacle()
{
	// 优先使用常驻触手（不用 Leader）
	for (Tentacle* resident : m_residentTentacles)
	{
		if (IsTentacleAvailable(resident))
		{
			return resident;
		}
	}

	// TODO Phase 3: 如果没有常驻触手可用，生成临时触手

	return nullptr;
}

void TentacleManager::CleanupCompletedTasks()
{
	// 移除已完成的任务
	for (auto it = m_activeTasks.begin(); it != m_activeTasks.end(); )
	{
		bool shouldRemove = false;

		if (it->eventType == GlobalEvent::DROPPING_FOOD)
		{
			Food* food = static_cast<Food*>(it->taskData);

			// 检查触手是否回到 Idle（任务完成）
			if (it->tentacle->GetCurrentStateType() == TentacleStateType::IDLE)
			{
				shouldRemove = true;

				// 标记食物可以删除
				if (food)
				{
					food->MarkForDeletion();  // ← 添加这个方法
				}
			}
		}


		if (shouldRemove)
		{
			it = m_activeTasks.erase(it);
		}
		else
		{
			++it;
		}
	}
}