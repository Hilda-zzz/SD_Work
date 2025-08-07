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
class DayTimeSystem;
class InventoryItemDef;

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

	void TransMap(Map* toMap, Vec2 const& playerPos);
private:
	void InitializeMenuPanel();

	void InitializeToolBarPanel();
	
	void SelectToolBarSlot(int slotIndex);

	static bool UpdateToolBarFromInventoryEvent(EventArgs& args);

	void InitializeStatusPanel();

	// Seed Shop
	void InitializeSeedShopPanel();
	void InitializeSeedShopSlots(Vec2 const& panelPos);
	void InitializeSeedShopInventory();
	void InitializeSeedShopBuyButton(Vec2 const& panelPos);
	void UpdateSeedShopDisplay();
	void SelectSeedShopSlot(int slotIndex);

	//Side Panel
	void InitializeSidePanel();

	//--------------Inventory----------
	void InitializeInventoryPanel();
	void InitializeInventoryLeftPanel();
	void InitializeInventoryRightPanel();

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

	static bool BtnEvent_SeedShopSlotClicked(EventArgs& args);
	static bool BtnEvent_SeedShopBuyClicked(EventArgs& args);

	static bool BtnEvent_ToggleSeedShop(EventArgs& args);
	static bool	BtnEvent_ToggleTutorial(EventArgs& args);

	static bool StartNewDayEvent(EventArgs& args);

	static bool UpdateCoinEvent(EventArgs& args);

public:
	Clock* m_gameClock = nullptr;
	RandomNumberGenerator m_rng;

	static GameState m_curGameState;
	static GameState m_nextGameState;
	
	Map* m_curMap = nullptr;
	Map* m_innerMap= nullptr;
	Map* m_outsideMap = nullptr;
	Player* m_player = nullptr;
	TileMapManager* g_tileManager;
	int m_dropTimes = 0;

private:
	DayTimeSystem* m_dayTimeSystem = nullptr;
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

	// Status Panel
	Panel m_statusPanel;
	UIImage m_statusImage; 
	UIText m_daysText;
	UIText m_timeText;
	UIText m_coinText;

	// Inventory Panel
	Panel m_inventoryPanel;
	Panel m_inventoryLeftPanel ;  
	Panel m_inventoryRightPanel; 

	static constexpr int INVENTORY_ROWS = 5;
	static constexpr int INVENTORY_COLS = 6;
	static constexpr int INVENTORY_SLOTS = INVENTORY_ROWS * INVENTORY_COLS;
	InventorySlotButton m_inventorySlots[INVENTORY_SLOTS];

	UIImage m_itemIconImage;
	UIText m_itemNameText;
	UIText m_itemQuantityText;
	UIText m_itemValueText;
	UIText m_itemDescriptionText;

	int m_selectedInventorySlot = -1;
	bool m_isInventoryOpen = false;

	// Seeds Shop
	Panel m_seedShopPanel;
	Button m_seedShopBuyButton;

	static constexpr int SEED_SHOP_ROWS = 2;
	static constexpr int SEED_SHOP_COLS = 5;
	static constexpr int SEED_SHOP_SLOTS = SEED_SHOP_ROWS * SEED_SHOP_COLS;
	InventorySlotButton m_seedShopSlots[SEED_SHOP_SLOTS];

	int m_selectedSeedShopSlot = -1;
	bool m_isSeedShopOpen = false;
	InventoryItemDef* m_seedShopItemsDef[SEED_SHOP_SLOTS];
	InventoryItem* m_seedShopItems[SEED_SHOP_SLOTS];

	// Side button panel
	Panel m_sidePanel;
	Button m_btnSeedShop;
	Button m_btnTutorial;

	bool m_isOpenInstruction = false;
	std::vector<Vertex_PCU> m_instructionVerts;

	//----------------------------------------
	SoundID m_bgm;                    // BGM.mp3
	SoundID m_coinSound;              // Coin.wav
	SoundID m_doorSound;              // Door.wav
	SoundID m_plowSound;              // Plow.wav
	SoundID m_rockSound;              // Rock.wav
	SoundID m_waterSound;             // Water.wav
	SoundID m_weedSound;              // Weed.wav
	SoundID m_woodSound;              // Wood.wav
};