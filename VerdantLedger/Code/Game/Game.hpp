#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/GameUISystem/Button.hpp"
#include "Engine/GameUISystem/Panel.hpp"
#include "InventorySlotButton.hpp"
#include "Engine/GameUISystem/UIImage.hpp"
#include "Engine/GameUISystem/UIText.hpp"
class Clock;
class Map;
class TileMapManager;
class Player;

extern const Vec2 g_halfGameCamDimensions;
constexpr int RESOLUTION = 16;
constexpr int CHUNK_SIZE = 16;
constexpr int Z_OFFSET = 200;

enum class GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_MENU,
	GAME_STATE_SAVELOAD,
	GAME_STATE_SETTINGS,
	GAME_STATE_GAMEPLAY,
};


class Game
{
public:
	Game();
	~Game();

	void Update();
	void Renderer() const;

	void UpdateToolBarFromInventory();
private:
	void InitializeMenuPanel();

	void InitializeToolBarPanel();
	
	void SelectToolBarSlot(int slotIndex);

	static bool UpdateToolBarFromInventoryEvent(EventArgs& args);

	void InitializeStatusPanel();

	//------Update--------------
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);

	void UpdateDeveloperCheats(float deltaTime);
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);

	//------Render--------------
	void RenderAttractMode() const;
	void RenderGameplayMode() const;

	void RenderGameplayUI() const;
	void RenderDebugMode() const;

	//------Process Control------
	void EnterState(GameState state);
	void EnterAttractMode();
	void EnterGameplayMode();

	void ExitState(GameState state);
	void ExitAttractMode();
	void ExitGameplayMode();


	//-------event--------------------
	static bool BtnEvent_StartNew(EventArgs& args);
	static bool BtnEvent_Load(EventArgs& args);
	static bool BtnEvent_Exit(EventArgs& args);
	static bool BtnEvent_ToolBarSlotClicked(EventArgs& args);

public:
	Clock* m_gameClock = nullptr;
	RandomNumberGenerator m_rng;

	static GameState m_curGameState;
	static GameState m_nextGameState;
	
	Map* m_curMap = nullptr;
	Player* m_player = nullptr;
	TileMapManager* g_tileManager;
	int m_dropTimes = 0;

private:
	
	Camera m_screenCamera;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;
	bool m_isDevConsole = false;

	float m_curDeltaTime = 0.f;

	//Menu UI
	Panel m_menuPanel;
	Button m_btnMenuStartNew;
	Button m_btnMenuLoad;
	Button m_btnMenuExit;

	Panel m_toolBarPanel;
	InventorySlotButton m_toolBarSlots[9];

	Panel m_statusPanel;
	UIImage m_statusImage; 
	UIText m_daysText;
	UIText m_timeText;
	UIText m_coinText;
};