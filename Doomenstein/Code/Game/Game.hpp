#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Button.hpp"
#include "Engine/Core/EventSystem.hpp"

class Clock;
class Player;
class Texture;
class Entity;
class Map;
class PlayerController;
typedef std::vector<Entity*> EntityList;
class BitmapFont;

enum class GameState
{
	NONE,
	ATTRACT,
	Menu,
	LOBBY,
	PLAYING,
	GOLD,
	COUNT
};

class Game
{
public:
	Game();
	~Game();

	void Update();
	void Renderer() const;

	void EnterState(GameState state);
	void ExitState(GameState state);

	void EnterAttractMode();
	void EnterPlayingMode(bool isGold);

	void ExitLobbyMode();
	void ExitPlayingMode();

	// event
	static bool BtnEvent_NormalMode(EventArgs& args);
	static bool BtnEvent_GoldlMode(EventArgs& args);
	static bool BtnEvent_TutorialMode(EventArgs& args);

private:
	void UpdateAttractMode(float deltaTime);
	void UpdateLobbyMode(float deltaTime);
	void AddTextForLobby();
	void UpdateGameplayMode(float deltaTime);

	void UpdateDeveloperCheats();
	void UpdateCamera(float deltaTime);
	void AdjustForPauseAndTimeDitortion();

	void RenderAttractMode() const;
	void RenderLobbyMode() const;
	void RenderPlayingMode() const;
	void RenderPlayingModeHUD() const;
	void RenderDebugMode() const;

	void ParseGameConfig();

	void UpdateMenuMode();
	void RenderMenuMode() const;

	void InitializeMenuButtons();

public:
	Clock* m_gameClock = nullptr;
	Camera m_screenCamera;
	RandomNumberGenerator m_rng;

	bool m_isDevConsole = false;
	static GameState m_curGameState;
	static GameState m_nextState;

	std::vector<Map*> m_maps;
	Map* m_curMap = nullptr;
	int m_curMapIndex = 0;
	PlayerController* m_playerController0 = nullptr;
	PlayerController* m_playerController1 = nullptr;
private:
	float m_previousTimeScale = 1.f;
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_pauseAfterUpdate = false;

	BitmapFont* m_font = nullptr;
	BitmapFont* m_menuBtnFont = nullptr;
	std::vector<Vertex_PCU> m_lobbyTextVerts;

	float m_musicVolume = 0.1f;

	std::string m_mainMenuMusicPath;
	std::string	m_gameMusicPath;
	std::string	m_buttonClickSoundPath;

	SoundPlaybackID m_menuMusic;
	SoundPlaybackID m_gameMusic;

	//Menu UI
	Button m_btnMenuNormalMode;
	Button m_btnMenuGoldMode;
	Button m_btnMenuTutorialMode;
};