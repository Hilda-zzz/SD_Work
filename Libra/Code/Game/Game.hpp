#pragma once
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <vector>
class Player;
class Map;

class Game
{
public:
	Game();
	~Game();

	void Update(float deltaSeconds);
	void Renderer() const;
	bool IsNoClip();
	void ChangeToNextMap();
	void ShakeScreen();
private:
	void UpdateInput(float& deltaTime);
	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);
	void UpdateDeveloperCheats(float& deltaTime);
	
	void AdjustForPauseAndTimeDitortion(float& deltaSeconds);
	void RenderAttractMode() const;
	void RenderUI() const;
	void RenderDebugMode() const;

	void ChangeMap(Map* nextMap,float nextPlayerOrientation);

	void SetDebugCameraOrtho(float tileSizeX, float tileSizeY);


	void CheckIsPlayerAlive();
	void GameOverState();
	void ResetAndBackToAttractMode();

public:
	int m_isHeatMapCount =0;
	RandomNumberGenerator m_rng;

	Map* m_currentMap = nullptr;
	Camera m_screenCamera;
	Camera m_worldCamera;
	Camera m_debugCamera;
	
	//------------------------------------------------------------------------------------
	SoundID m_clickSound;
	SoundID m_attractModeBgm;
	SoundID m_gameplayModeBgm;
	SoundID m_pause;
	SoundID m_unpause;

	SoundID m_success;
	SoundID m_fall;

	SoundID m_playerShoot;
	SoundID m_enemyShoot;

	SoundID m_enemyDead;
	SoundID m_playerDead;

	SoundID m_enemyHit;
	SoundID m_playerHit;
	SoundID m_bulletBounce;
	//------------------------------------------------------------------------------------

	Player* m_player = nullptr;
	std::vector<Map*> m_maps;

	bool m_isChangeMap = false;
	bool m_isStartMap = false;
	float m_transitionTime = 1.f;
private:
	bool m_isAttractMode = true;
	bool m_isDebugMode = false;
	bool m_isUseDebugCam = false;
	
	bool m_isPause = false;
	bool m_isSlow = false;
	bool m_isSpeedUp = false;
	bool m_pauseAfterUpdate = false;
	bool m_isNoclip = false;
	bool m_isGameOver = false;
	bool m_isSuccess = false;
	

	double m_playerDeadCountDownStart=0.0;
	
	bool m_isPlayerDeadCountDownStart = false;

	int m_curMapIndex = 0;

	float m_startTransTime = 0.f;
	float m_alphaBlackFront = 0.f;

	bool m_isShakeScreen = false;
	bool m_isGapShake = false;
	double m_startGapShakeTime = 0.f;
	double  m_startShakeTime=0.f;
};