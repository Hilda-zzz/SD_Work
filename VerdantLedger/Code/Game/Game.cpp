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
#include "TileMapManager.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include <iostream>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ObstacleDefinitions.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/GameUISystem/GameUISystem.hpp"
#include "Engine/GameUISystem/Panel.hpp"
#include "InventoryItemDef.hpp"
#include "Game/Inventory.hpp"
#include "CropDefinitions.hpp"


extern GameUISystem* g_gameUISystem;
extern bool g_isDebugDraw;
extern Window* g_theWindow;
extern Game* g_theGame;

GameState Game::m_curGameState = GameState::GAME_STATE_ATTRACT;
GameState Game::m_nextGameState = GameState::GAME_STATE_ATTRACT;

Vec2 const g_halfGameCamDimensions{ 10.f, 5.f };

Game::Game()
{
	m_gameClock = new Clock();
	ObstacleDefinition::InitializeObstacleDefinitionFromFile();
	InventoryItemDef::InitializeInventoryItemDefinitionFromFile();
	CropDefinitions::InitializeCropDefinitionsFromFile();

	g_tileManager = &TileMapManager::GetInstance();
	g_tileManager->InitAllTilemapResources();

	// ---------------- UI System ---------------------------------
	InitializeMenuPanel();
	g_gameUISystem->PushPanel(&m_menuPanel);

	InitializeToolBarPanel();
	
	InitializeStatusPanel();

	// g_theEventSystem->SubscribeEventCallbackFuction("UpdateInventoryPanels", UpdateToolBarFromInventory);
	// g_theEventSystem->SubscribeEventCallbackFuction("UpdateInventoryPanels");
	//--------------------------------------------------------------
	m_player = new Player(this);
	UpdateToolBarFromInventory();
	m_curMap = new Map(this, g_tileManager->m_loadedMaps["Data/Tiled/MyFarmMap.tmx"], m_player);

	g_theDevConsole->AddLine(DevConsole::HELPLIST, "WASD: Move around\n\
Tools: 0-None, 1-Axe, 2-Hoe, 3-Pickaxe, 4-Shovel, 5-Sickle, 6-Water\n\
Left Mouse Button: Use selected tool");
}

Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;

	TileMapManager::DestroyInstance();
	g_tileManager = nullptr;

	delete m_curMap;
	m_curMap = nullptr;

	CropDefinitions::ShutdownCropDefinitions();
	ObstacleDefinition::ShutdownObstacleDefinition();
	InventoryItemDef::ShutdownInventoryItemDefinition();
}

void Game::Update()
{
	float deltaSeconds = (float)m_gameClock->GetDeltaSeconds();

	m_curDeltaTime = deltaSeconds;

	float framerate = 1.f / m_curDeltaTime;
	if (framerate < 200)
		m_dropTimes += 1;

	UpdateCamera(deltaSeconds);

	g_gameUISystem->Update(deltaSeconds);

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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		RenderGameplayMode();
		break;
	default:
		break;
	}
	
	g_theRenderer->BeginCamera(m_screenCamera);
	g_gameUISystem->Render();
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::InitializeMenuPanel()
{
	//Texture* menuBkg = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/dialogue box.png");
	Texture * menuBkg = nullptr;
	m_menuPanel =Panel(Vec2(800.f, 400.f), menuBkg, AABB2(Vec2(-30.f, -30.f), Vec2(30.f, 30.f)));

	Texture* menuBtnTexture1 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn1.png");
	Texture* menuBtnTexture2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn2.png");
	Texture* menuBtnTexture3 = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBtn3.png");

	float btnWidth = 240.0f;
	float btnHeight = 80.0f;

	float leftMargin = 200.0f;
	float startY = 350.0f;
	float buttonSpacing = 100.0f;

	AABB2 bkgExtent = AABB2(-btnWidth / 2.0f, -btnHeight / 2.0f, btnWidth / 2.0f, btnHeight / 2.0f);

	AABB2 textExtent = AABB2(-btnWidth / 2.0f + 10.0f, -btnHeight / 2.0f + 10.0f,
		btnWidth / 2.0f - 10.0f, btnHeight / 2.0f - 10.0f);

	BitmapFont* gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	float textHeight = 30.0f;

	Vec2 btnNormalPos = Vec2(leftMargin + btnWidth / 2.0f, startY);
	m_btnMenuStartNew = Button(g_gameUISystem,btnNormalPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "New", textHeight, gameFont, "StartNew");

	Vec2 btnGoldPos = Vec2(leftMargin + btnWidth / 2.0f, startY - buttonSpacing);
	m_btnMenuLoad = Button(g_gameUISystem, btnGoldPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Load", textHeight, gameFont, "Load");

	Vec2 btnTutorialPos = Vec2(leftMargin + btnWidth / 2.0f, startY - 2 * buttonSpacing);
	m_btnMenuExit = Button(g_gameUISystem, btnTutorialPos, menuBtnTexture1, menuBtnTexture2, menuBtnTexture3,
		bkgExtent, textExtent, "Exit", textHeight, gameFont, "Exit");

	//call back
	g_theEventSystem->SubscribeEventCallbackFuction("StartNew", BtnEvent_StartNew, true);
	g_theEventSystem->SubscribeEventCallbackFuction("Load", BtnEvent_Load, true);
	g_theEventSystem->SubscribeEventCallbackFuction("Exit", BtnEvent_Exit, true);

	m_menuPanel.AddChild(&m_btnMenuStartNew);
	m_menuPanel.AddChild(&m_btnMenuLoad);
	m_menuPanel.AddChild(&m_btnMenuExit);
}

void Game::InitializeToolBarPanel()
{
	Texture* toolBarBkg = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/ToolBarBkg.png");

	float windowWidth = 1600.0f;
	//float windowHeight = 800.0f;

	float toolBarWidth = 550.0f;  
	float toolBarHeight = 150.0f;

	float toolBarX = windowWidth / 2.0f;        
	float toolBarY = 60.0f;                     
	Vec2 toolBarPos = Vec2(toolBarX, toolBarY);

	AABB2 toolBarExtent = AABB2(-toolBarWidth / 2.0f, -toolBarHeight / 2.0f,
		toolBarWidth / 2.0f, toolBarHeight / 2.0f);

	m_toolBarPanel = Panel(toolBarPos, toolBarBkg, toolBarExtent);
	m_toolBarPanel.SetIsrenderSelf(true);

	Texture* slotNormalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/Inventory/InventorySlotBtn1.png");
	Texture* slotSelectedTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/Inventory/InventorySlotBtn2.png");
	Texture* slotBorderTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/Inventory/InventorySlotBtn3.png");

	float slotSize = 47.0f;     
	float slotSpacing = 52.0f;    
	float totalSlotsWidth = 9 * slotSpacing; 
	float startX = -totalSlotsWidth / 2.0f + slotSpacing / 2.0f; 
	float slotY = 0.0f;

	AABB2 slotExtent = AABB2(-slotSize / 2.0f, -slotSize / 2.0f,
		slotSize / 2.0f, slotSize / 2.0f);
	AABB2 textExtent = AABB2(Vec2(-10.f,-10.f)+Vec2(10.f,-15.f), Vec2(10.f, 10.f) + Vec2(10.f, -15.f));

	BitmapFont* gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	for (int i = 0; i < 9; i++) 
	{
		Vec2 slotPos = Vec2(startX + i * slotSpacing, slotY)+ toolBarPos;

		std::string eventName = "ToolBarSlotClicked";

		m_toolBarSlots[i] = InventorySlotButton(
			g_gameUISystem,           // GameUISystem* uiSystem
			slotPos,                  // const Vec2& position
			slotNormalTexture,        // Texture* normalTex
			slotSelectedTexture,         // Texture* hoverTex
			slotSelectedTexture,      // Texture* clickTex
			nullptr,                  // Texture* iconTexture (初始为空)
			slotBorderTexture,                  // Texture* borderTexture (可选边框)
			slotExtent,              // AABB2 bkgExtent
			textExtent,              // AABB2 textExtent
			"",                      // std::string text (初始为空)
			10.0f,                    // float textHeight
			gameFont,                // BitmapFont* font
			eventName                // std::string clickEventName
		);
		m_toolBarSlots[i].SetHoverSound((size_t) - 1);
		m_toolBarSlots[i].SetSlotIndex(i);
		m_toolBarPanel.AddChild(&m_toolBarSlots[i]);
	}
	g_theEventSystem->SubscribeEventCallbackFuction("ToolBarSlotClicked", BtnEvent_ToolBarSlotClicked, true);

}

 void Game::UpdateToolBarFromInventory()
 {
 	if (m_player && m_player->GetInventory()) {
 		for (int i = 0; i < 9; i++)
 		{
 			if (i < (int)m_player->GetInventory()->m_items.size())
 			{
 				m_toolBarSlots[i].UpdateFromInventoryItem(&m_player->GetInventory()->m_items[i]);
 			}
 			else
 			{
 				m_toolBarSlots[i].UpdateFromInventoryItem(nullptr);
 			}
 		}
 	}
 }

void Game::SelectToolBarSlot(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= 9)
	{
		return;  
	}

	for (InventorySlotButton& btn : m_toolBarSlots)
	{
		btn.SetSelectedState(false);
	}
	m_toolBarSlots[slotIndex].SetSelectedState(true);
	m_player->m_curSelectedBtn = &m_toolBarSlots[slotIndex];

	if (m_player && m_player->GetInventory()) 
	{
		InventoryItem* curItem=m_toolBarSlots[slotIndex].GetItem();
		if (curItem&&curItem->m_itemDef)
		{
			InventoryItemDef const* curDef = curItem->m_itemDef;
			if (curDef->m_itemType == ItemType::ITEM_TYPE_TOOL)
			{
				m_player->m_curTool = curDef->m_toolType;
			}
			else if(curDef->m_itemType == ItemType::ITEM_TYPE_SEED)
			{
				m_player->m_curTool = PlayerTools::SEEDS;
			}
			else
			{
				m_player->m_curTool = PlayerTools::NONE;
			}
		}
		else 
		{
			m_player->m_curTool = PlayerTools::NONE;
		}
	}
}

// bool Game::UpdateToolBarFromInventory(EventArgs& args)
// {
// 	if (g_theGame->m_player && g_theGame->m_player->GetInventory())
// 	{
// 		for (int i = 0; i < 9; i++)
// 		{
// 			if (i < (int)g_theGame->m_player->GetInventory()->m_items.size())
// 			{
// 				g_theGame->m_toolBarSlots[i].UpdateFromInventoryItem(&g_theGame->m_player->GetInventory()->m_items[i]);
// 			}
// 			else
// 			{
// 				g_theGame->m_toolBarSlots[i].UpdateFromInventoryItem(nullptr);
// 			}
// 		}
// 	}
// 	return false;
// }

void Game::InitializeStatusPanel()
{
	Texture* statusPanelBkg = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/GameStateHUD.png");

	float windowWidth = 1600.0f;
	float windowHeight = 800.0f;

	float panelWidth = 250.0f;
	float panelHeight = 160.0f;
	float panelX = windowWidth - 130.0f;    
	float panelY = windowHeight - 90.0f;    

	Vec2 statusPanelPos = Vec2(panelX, panelY);
	AABB2 statusPanelExtent = AABB2(-panelWidth / 2.0f, -panelHeight / 2.0f,
		panelWidth / 2.0f, panelHeight / 2.0f);

	m_statusPanel = Panel(statusPanelPos, statusPanelBkg, statusPanelExtent);
	m_statusPanel.SetIsrenderSelf(true);

	BitmapFont* gameFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	Texture* avatarTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/Clock/Clock.png");
	float imageSize = 85.0f;
	Vec2 imagePos = Vec2(-panelWidth / 2.0f + imageSize / 2.0f + 20.0f, 20.0f);

	m_statusImage = UIImage(imagePos+statusPanelPos, avatarTexture,
		AABB2(-imageSize / 2, -imageSize / 2, imageSize / 2, imageSize / 2),AABB2::ZERO_TO_ONE);
	m_statusPanel.AddChild(&m_statusImage);


	float textStartX = imagePos.x + imageSize / 2.0f + 45.0f;  
	float textWidth = panelWidth / 2.0f; 
	float textHeight = 10.0f;
	float textSpacing = 25.0f;  
	float startY = 30.0f;

	Vec2 daysTextPos = Vec2(textStartX, startY)+statusPanelPos;
	AABB2 daysTextBounds = AABB2(daysTextPos.x - textWidth / 2, daysTextPos.y - textHeight / 2,
		daysTextPos.x + textWidth / 2,  daysTextPos.y + textHeight / 2);

	m_daysText = UIText("DaysText", "Day 1", gameFont, daysTextBounds, textHeight,Rgba8::BLACK);
	m_statusPanel.AddChild(&m_daysText);

	Vec2 timeTextPos = Vec2(textStartX, startY - textSpacing) + statusPanelPos;
	AABB2 timeTextBounds = AABB2(timeTextPos.x - textWidth / 2, timeTextPos.y - textHeight / 2,
		timeTextPos.x + textWidth / 2, timeTextPos.y + textHeight / 2);

	m_timeText = UIText("TimeText", "Morning", gameFont, timeTextBounds, textHeight, Rgba8::BLACK);
	m_statusPanel.AddChild(&m_timeText);

	Vec2 coinTextPos = Vec2(textStartX, startY - 3.f * textSpacing+5.f) + statusPanelPos;
	AABB2 coinTextBounds = AABB2(coinTextPos.x - textWidth / 2, coinTextPos.y - textHeight / 2,
		coinTextPos.x + textWidth / 2, coinTextPos.y + textHeight / 2);
	m_coinText = UIText("CoinText", "Coins: 1000", gameFont, coinTextBounds, textHeight,Rgba8::BLACK);
	m_statusPanel.AddChild(&m_coinText);
}

void Game::UpdateAttractMode(float deltaTime)
{
	UNUSED(deltaTime);
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->m_isQuitting = true;
	}
}

void Game::UpdateGameplayMode(float deltaTime)
{
	UNUSED(deltaTime);

	m_curMap->Update(deltaTime);

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

	std::vector<Vertex_PCU> verts;
	verts.reserve(20);
	AddVertsForAABB2D(verts, AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f)), Rgba8::WHITE);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Art/FarmAssets/UI - Tiny Asset Pack/MenuBkg.png"));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(verts);

	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()), g_theRenderer);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayMode() const
{
	m_curMap->Render();

// 	g_theRenderer->BindTexture(nullptr);
// 	g_theRenderer->SetModelConstants();
// 	DebugDrawBoxLine(m_player->m_position - Vec2(10.f, 5.f), m_player->m_position + Vec2(10.f, 5.f), 0.5f, Rgba8::GREEN);

	g_theRenderer->BeginCamera(m_screenCamera);
	RenderGameplayUI();
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::string curTool;
	curTool = "Current tool: ";
	switch (m_player->m_curTool) {
	case PlayerTools::NONE:
		curTool += "None";
		break;
	case PlayerTools::AXE:
		curTool += "Axe";
		break;
	case PlayerTools::HOE:
		curTool += "Hoe";
		break;
	case PlayerTools::PICKAXE:
		curTool += "Pickaxe";
		break;
	case PlayerTools::SHOVEL:
		curTool += "Shovel";
		break;
	case PlayerTools::SICKLE:
		curTool += "Sickle";
		break;
	case PlayerTools::WATER:
		curTool += "Water";
		break;
	default:
		curTool += "Unknown";
		break;
	}
	curTool += " || Use 0-6 to switch tools || Use mouse left btn to use the tool (Pickaxe--Rock || Sickle--Weed)";
	font->AddVertsForTextInBox2D(title, curTool,
		AABB2(Vec2(10.f, 745.f), Vec2(1000.f, 765.f)), 15.f, Rgba8::BLACK, 0.7f, Vec2(0.f, 0.f));

	float framerate = 1.f / m_curDeltaTime;
	DebuggerPrintf("Framerate: %.2f\n", framerate);
	char buffer[256];
	sprintf_s(buffer, 
		"DT = %.2f\n"
		"Framerate = %.2f\n",
		m_curDeltaTime * 1000.f,
		framerate);
	std::string statsMessage(buffer);

	font->AddVertsForTextInBox2D(title, statsMessage,
		AABB2(Vec2(10.f, 500.f), Vec2(600.f, 700.f)), 15.f, Rgba8::WHITE, 0.7f, Vec2(0.f, 1.f));

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderGameplayUI() const
{

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
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
		break;
	case GameState::GAME_STATE_GAMEPLAY:
		EnterGameplayMode();
		break;
	default:
		break;
	}
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::GAME_STATE_ATTRACT:
		ExitAttractMode();
		break;
	case GameState::GAME_STATE_MENU:
		break;
	case GameState::GAME_STATE_SAVELOAD:
		break;
	case GameState::GAME_STATE_SETTINGS:
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
	g_gameUISystem->PopAllPanel();
}

void Game::ExitGameplayMode()
{
	g_gameUISystem->PopAllPanel();
}

void Game::EnterAttractMode()
{
	g_gameUISystem->PushPanel(&m_menuPanel);
}

void Game::EnterGameplayMode()
{
	g_gameUISystem->PushPanel(&m_toolBarPanel);
	g_gameUISystem->PushPanel(&m_statusPanel);
}

bool Game::BtnEvent_StartNew(EventArgs& args)
{
	UNUSED(args);
	m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	return true;
}

bool Game::BtnEvent_Load(EventArgs& args)
{
	UNUSED(args);
	m_nextGameState = GameState::GAME_STATE_GAMEPLAY;
	return true;
}

bool Game::BtnEvent_Exit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true;
	return true;
}

bool Game::BtnEvent_ToolBarSlotClicked(EventArgs& args)
{
	std::string slotIndexStr = args.GetValue("slotIndex", "-1");
	int slotIndex = std::stoi(slotIndexStr);

	if (slotIndex >= 0 && slotIndex < 9) 
	{
		g_theGame->SelectToolBarSlot(slotIndex);
	}

	return true;
}










