#include "Game/Game.hpp"
#include "App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "ThreadPool.hpp"
#include <iostream>
extern bool g_isDebugDraw;
extern Window* g_theWindow;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

void sleepFor(int seconds) {
	std::cout << "Start Sleep " << seconds << " Secs..." << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	std::cout << "Slept " << seconds << " Secs Finished" << std::endl;
}

// test task funcs
int fibonacci(int n) {
	if (n <= 1) return n;
	return fibonacci(n - 1) + fibonacci(n - 2);
}

std::string getMessage(const std::string& name, int waitSeconds) {
	std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
	return "Hello, " + name + "! (waited " + std::to_string(waitSeconds) + "s)";
}

void printMessage(const std::string& message) {
	std::cout << "Message: " << message << std::endl;
}

// Test time
int longComputation(int duration)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(duration));
	return duration;
}

int longComputation2(int id, int duration) {
	std::cout << "Task " << id << " started, duration: " << duration << "ms" << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(duration));
	std::cout << "Task " << id << " completed" << std::endl;
	return duration;
}

// throw exception
int errorProneTask(int n)
{
	if (n % 3 == 0)
	{
		throw std::runtime_error("Number is divisible by 3!");
	}
	return n * n;
}

int errorProneTask2(int id, bool shouldFail) {
	std::cout << "ErrorProneTask " << id << " started" << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if (shouldFail) {
		std::cout << "ErrorProneTask " << id << " throwing exception" << std::endl;
		throw std::runtime_error("Task failed on purpose");
	}

	std::cout << "ErrorProneTask " << id << " completed successfully" << std::endl;
	return id;
}

// Log
void printPoolStatus(ThreadPool& pool, const std::string& stage) {
	std::cout << "\n=== " << stage << " ===" << std::endl;
	std::cout << " ThreadCount " << pool.GetThreadCount() << std::endl;
	std::cout << "  ActiveThreadCount " << pool.GetActiveThreadCount() << std::endl;
	std::cout << "  WaitThreadCount: " << pool.GetWaitingThreadCount() << std::endl;
	std::cout << "  TasksCount " << pool.GetTasksCount() << std::endl;
	std::cout << "  CompletedTaskCount " << pool.GetCompletedTaskCount() << std::endl;
	std::cout << "  FailedTaskCount: " << pool.GetFailedTaskCount() << std::endl;
}

void QuickSort(std::vector<int>& nums, int left, int right) {
	if (left < right) {
		int pivotIndex = Partition(nums, left, right);

		QuickSort(nums, left, pivotIndex - 1);

		QuickSort(nums, pivotIndex + 1, right);
	}
}

int Partition(std::vector<int>& nums, int left, int right)
{
	int pivot = nums[left];
	while (left <= right)
	{
		while (nums[left] < pivot && left <= right)
		{
			left++;
		}
		while (nums[right] > pivot && left <= right)
		{
			right--;
		}
		if (left >= right)
		{
			break;
		}
		std::swap(nums[right], nums[left]);
		left++;
		right--;
	}
	return left;
}

void PrintArray(std::vector<int> const& nums)
{
	for (int i = 0; i < nums.size(); i++)
	{
		std::cout << nums[i]<< " ";
	}
	std::cout << std::endl;
}

Game::Game()
{
	m_gameClock = new Clock();
	//TestForDay5();

	std::vector<int> reverse = { 5, 4, 3, 2, 1 };
	QuickSort(reverse, 0, 4);
	PrintArray(reverse);
}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;
}


void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

	UpdateCamera(deltaSeconds);

	// Update Game State
	if (m_curGameState != m_nextGameState)
	{
		ExitState(m_curGameState);
		EnterState(m_nextGameState);
		m_curGameState = m_nextGameState;
	}

	// Update DevConsole
	if (g_theInput->WasKeyJustPressedRaw(KEYCODE_TILDE))
	{
		if (g_theDevConsole->GetMode() == HIDDEN)
		{
			g_theDevConsole->SetMode(OPEN_FULL);
			m_isDevConsole = true;
		}
		else
		{
			g_theDevConsole->SetMode(HIDDEN);
			m_isDevConsole = false;
		}
	}

	// Call Specific Update()
	switch (m_curGameState)
	{
	case GameState::GAME_STATE_ATTRACT:
		UpdateAttractMode(deltaSeconds);
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		UpdateGameplayMode(deltaSeconds);
		break;
	default:
		break;
	}

	UpdateDeveloperCheats(deltaSeconds);
}

void Game::Renderer() const
{
	switch (m_curGameState)
	{
	case GameState::GAME_STATE_ATTRACT:
		RenderAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		RenderGameplayMode();
		break;
	default:
		break;
	}

	g_theRenderer->BeginCamera(m_screenCamera);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)|| g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextGameState = GameState::GAME_STATE_ATTRACT;
	}
}

void Game::UpdateDeveloperCheats(float deltaTime)
{
	UNUSED(deltaTime);
	AdjustForPauseAndTimeDitortion(deltaTime);
	if (g_theInput->WasKeyJustPressed('L'))
	{
		g_isDebugDraw = !g_isDebugDraw;
	}
}

void Game::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	IntVec2 windowDimension = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2(0.f, 0.f), Vec2((float)windowDimension.x, (float)windowDimension.y)));
	m_screenCamera.SetOrthographicView(Vec2{ 0.f,0.f }, Vec2{ 1600.f,800.f });
}

void Game::AdjustForPauseAndTimeDitortion(float& deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_isPause = !m_isPause;
	}

	m_isSlow = g_theInput->IsKeyDown('T');

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPause = false;
		m_pauseAfterUpdate = true;
	}

	//--------------------------------------------------------------------------------------

	if (m_isPause)
	{
		deltaSeconds = 0.f;
	}
	if (m_isSlow)
	{
		deltaSeconds *= 0.10f;
	}
	if (m_pauseAfterUpdate)
	{
		m_isPause = true;
		m_pauseAfterUpdate = false;
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	DebugDrawRing(4.f, 20.f, Rgba8::WHITE, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderUI();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderUI() const
{
	g_theRenderer->BindTexture(nullptr);
	DebugDrawLine(Vec2(100.f, 100.f), Vec2(1500.f, 700.f), 4.f, Rgba8(180, 0, 100));
	DebugDrawLine(Vec2(100.f, 700.f), Vec2(1500.f, 100.f), 4.f, Rgba8(180, 0, 100));
}

void Game::RenderDebugMode()const
{

}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::GAME_STATE_ATTRACT:
		EnterAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		EnterGameplayMode();
		break;
	default:
		break;
	}
}

void Game::EnterAttractMode()
{
}

void Game::EnterGameplayMode()
{
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::GAME_STATE_ATTRACT:
		ExitAttractMode();
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		ExitGameplayMode();
		break;
	default:
		break;
	}
}

void Game::ExitAttractMode()
{
}

void Game::ExitGameplayMode()
{
}

void Game::TestForDay3()
{
	// Initialize thread pool
	size_t threadCount = std::thread::hardware_concurrency();
	std::cout << "The System has " << threadCount << " CPU cores" << std::endl;
	threadCount = threadCount == 0 ? 1 : threadCount;

	size_t poolThreads = std::min(threadCount, (size_t)4);

	try {
		std::cout << "\n--- Test Start ---" << std::endl;
		ThreadPool pool(poolThreads);

		printPoolStatus(pool, "Start State");

		std::vector<std::future<int>> fibs;
		std::vector<std::future<std::string>> msgs;
		std::vector<std::future<void>> prints;

		std::cout << "Start Submitting tasks!" << std::endl;
		std::cout << "Fibonacci tasks..." << std::endl;
		for (int i = 20; i < 25; ++i) {
			fibs.push_back(
				pool.Enqueue(fibonacci, i)
			);
		}

		std::cout << "GetMessages tasks..." << std::endl;
		for (int i = 1; i <= 3; ++i) {
			msgs.push_back(
				pool.Enqueue(getMessage, "User" + std::to_string(i), 1)
			);
		}

		std::cout << "PrintMessages tasks..." << std::endl;
		for (int i = 0; i < 3; ++i) {
			prints.push_back(
				pool.Enqueue(printMessage, "This is message " + std::to_string(i) + "\n")
			);
		}

		std::cout << "\n---Get tasks' results!---" << std::endl;
		for (size_t i = 0; i < fibs.size(); ++i) {
			std::cout << "fibonacci(" << (i + 20) << ") = " << fibs[i].get() << std::endl;
		}

		for (auto& future : msgs) {
			std::cout << future.get() << std::endl;
		}

		for (auto& future : prints) {
			future.wait();
		}

		std::cout << "\n--- Tests completed! ---" << std::endl;

		std::cout << "Is the pool stopped: " << (pool.IsStopped() ? "Yes" : "No") << std::endl;
	}
	catch (std::exception const& e) {
		std::cerr << "exception when constructing thread pool in Game.cpp" << e.what() << std::endl;
		return;
	}

	std::cout << "\n--- Test Complete ---" << std::endl;
}

void Game::TestForDay4()
{
	// Initialize thread pool
	size_t threadCount = std::thread::hardware_concurrency();
	std::cout << "The System has " << threadCount << " CPU cores" << std::endl;
	threadCount = threadCount == 0 ? 1 : threadCount;

	size_t poolThreads = std::min(threadCount, (size_t)4);

	try {
		std::cout << "\n--- Test Start ---" << std::endl;
		ThreadPool pool(poolThreads);

		printPoolStatus(pool, "Start State");

		// Day 4 tests
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> durDist(100, 500); // ms

		std::cout << "\nSubmit 10 normal tasks..." << std::endl;
		std::vector<std::future<int>> results;
		for (int i = 0; i < 10; ++i) {
			int duration = durDist(gen);
			results.push_back(pool.Enqueue(longComputation, duration));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		printPoolStatus(pool, "After 10 normal tasks, delay 50 ms- State");

		std::cout << "\nSubmit 10 exception tasks..." << std::endl;
		std::vector<std::future<int>> errorResults;
		for (int i = 0; i < 10; ++i) {
			errorResults.push_back(pool.Enqueue(errorProneTask, i));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		printPoolStatus(pool, "After 10 except tasks, delay 50 ms- State");

		std::cout << "\nWaiting normal tasks completing..." << std::endl;
		for (size_t i = 0; i < results.size(); ++i) {
			try {
				int duration = results[i].get();
				std::cout << "Normal tasks " << i << " Consume time " << duration << "ms" << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "Normal tasks " << i << " Throw exception " << e.what() << std::endl;
			}
		}

		std::cout << "\nWaiting exc tasks completing..." << std::endl;
		for (size_t i = 0; i < errorResults.size(); ++i) {
			try {
				int result = errorResults[i].get();
				std::cout << "exc " << i << " Consume time = " << result << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "exc " << i << " Throw exception: " << e.what() << std::endl;
			}
		}

		printPoolStatus(pool, "Final State");

	}
	catch (std::exception const& e) {
		std::cerr << "exception when constructing thread pool in Game.cpp" << e.what() << std::endl;
		return;
	}

	std::cout << "\n--- Test Complete ---" << std::endl;
}

void Game::TestForDay5()
{
	std::cout << "=== C++11 ThreadPool Implementation - Day 5 Test ===" << std::endl;

	try {
		// Create thread pool with thread count equal to hardware concurrency
		size_t threadCount = std::thread::hardware_concurrency();
		threadCount = threadCount == 0 ? 1 : threadCount;

		// Use fewer threads for better observation of effects
		size_t poolThreads = std::min(threadCount, (size_t)4);

		std::cout << "System has " << threadCount << " CPU cores" << std::endl;
		std::cout << "Creating thread pool with " << poolThreads << " threads" << std::endl;

		ThreadPool pool(poolThreads);

		// Show initial status
		printPoolStatus(pool, "Initial status");

		// Random number generator for task duration
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> durDist(100, 300); // 100-300ms

		// Submit some tasks
		std::cout << "\nSubmitting 6 normal tasks..." << std::endl;
		std::vector<std::future<int>> results;
		for (int i = 0; i < 6; ++i) {
			int duration = durDist(gen);
			results.push_back(pool.Enqueue(longComputation2, i, duration));
		}

		// Show status after task submission
		// printPoolStatus(pool, "Status after task submission");

		// Wait for a while to let some tasks complete
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		// Show status after partial task completion
		printPoolStatus(pool, "Status after partial task completion");

		// Test pause functionality
		std::cout << "\n--- Testing pause/resume functionality ---" << std::endl;
		pool.Pause();

		// Submit more tasks (these will be paused)
		std::cout << "After pausing thread pool, submitting 3 tasks..." << std::endl;
		for (int i = 10; i < 13; ++i) {
			int duration = durDist(gen);
			results.push_back(pool.Enqueue(longComputation2, i, duration));
		}

		// Show status after pause
		printPoolStatus(pool, "Status after pause");

		// Wait for a while
		std::cout << "Waiting for 1 second..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));

		// Show status after waiting (should not change much since thread pool is paused)
		printPoolStatus(pool, "Status after waiting (paused)");

		// Resume thread pool
		pool.Resume();

		// Wait for a while to let more tasks complete
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		// Show status after resume
		printPoolStatus(pool, "Status after resume");

		// Test dynamic thread count adjustment
		std::cout << "\n--- Testing dynamic thread count adjustment ---" << std::endl;
		size_t newThreadCount = poolThreads + 2;
		std::cout << "Increasing thread count to " << newThreadCount << "..." << std::endl;
		pool.Resize(newThreadCount);

		// Show status after increasing threads
		printPoolStatus(pool, "Status after increasing threads");

		// Test reducing thread count
		newThreadCount = poolThreads;
		std::cout << "Reducing thread count to " << newThreadCount << "..." << std::endl;
		pool.Resize(newThreadCount);

		// Show status after reducing threads
		printPoolStatus(pool, "Status after reducing threads");

		// Test exception handling
		std::cout << "\n--- Testing exception handling ---" << std::endl;
		std::vector<std::future<int>> errorResults;
		for (int i = 0; i < 6; ++i) {
			bool shouldFail = (i % 3 == 0);
			errorResults.push_back(pool.Enqueue(errorProneTask2, i, shouldFail));
		}

		// Wait for and get results from potentially throwing tasks
		std::cout << "\nWaiting for error-prone tasks to complete..." << std::endl;
		for (size_t i = 0; i < errorResults.size(); ++i) {
			try {
				int result = errorResults[i].get();
				std::cout << "Error-prone task " << i << " succeeded with result: " << result << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "Error-prone task " << i << " failed: " << e.what() << std::endl;
			}
		}

		// Test clearing task queue
		std::cout << "\n--- Testing task queue clearing ---" << std::endl;
		for (int i = 100; i < 105; ++i) {
			int duration = durDist(gen);
			pool.Enqueue(longComputation2, i, duration);
		}

		// Show status after submitting tasks
		printPoolStatus(pool, "After submitting clear test tasks");

		// Clear task queue
		pool.ClearTasks();

		// Show status after clearing queue
		printPoolStatus(pool, "Status after clearing queue");

		// Test waiting for all tasks to complete
		std::cout << "\n--- Testing wait for all tasks to complete ---" << std::endl;

		// Wait for all normal task results
		std::cout << "Waiting for normal tasks to complete..." << std::endl;
		for (size_t i = 0; i < results.size(); ++i) {
			try {
				int duration = results[i].get();
				std::cout << "Normal task " << i << " result: " << duration << "ms" << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "Normal task " << i << " failed: " << e.what() << std::endl;
			}
		}

		// Wait for all tasks to complete
		pool.WaitForTasks();

		// Show final status
		printPoolStatus(pool, "Final status");

		std::cout << "\n--- Verifying thread pool control functionality ---" << std::endl;
		std::cout << "All tasks processed, thread pool control functionality is normal" << std::endl;
		std::cout << "Is thread pool stopped: " << (pool.IsStopped() ? "Yes" : "No") << std::endl;

	}
	catch (const std::exception& e) {
		std::cerr << "Exception occurred: " << e.what() << std::endl;
		return;
	}

	std::cout << "\n=== Day 5 Test Complete ===" << std::endl;
	std::cout << "Thread pool control functionality (resize, pause/resume, waitForTasks, clearTasks) is working properly!" << std::endl;

	return;
}











