#pragma once
#include "GameCommon.hpp"
#include "PlayerShip.hpp"
#include "Bullet.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Asteroid.hpp"
#include "Beetle.hpp"
#include "Wasp.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Debris.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "ExplosionBullet.hpp"
#include "ShootWasp.hpp"
#include "EnemyBullet.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Turret.hpp"
class BkgStar;
class Clock;
//for att mode
constexpr float ORBIT_LEN = 500.f;
constexpr float	SHIP_REVOLUTION_SPEED = 30.f;
//constexpr Vec2 ORBIT_CENTER = Vec2{800.f,400.f} ;

struct EnemyNumInWave
{
	int numAsteroids;
	int numBeetles;
	int numWasp;
	int numShootWasp;
	int numTurret;
};

enum EnemiesTypes
{
	TYPE_ASTEROID,
	TYPE_BEETELE,
	TYPE_WASP,
	TYPE_SHOOTWASP,
	TYPE_TURRET,
	ENEMIES_TYPE_NUM
};

class Game
{
public:
	Game();
	~Game();

	void InitGameplay();
	void ResetGameplayBackToAttractMode();

	//-----------------------------------------------------------------
	void Renderer() const;
	void RenderBkg() const;
	void RenderBullet() const;
	void RenderExpBullet() const;
	void RenderEnmBullet() const;
	void RenderAsteroids() const;
	void RenderBeetles() const;
	void RenderWasps() const;
	void RenderShootWasps() const;
	void RenderTurrets() const;
	void RenderDebris() const;
	void RenderDebugMode() const;
	void RenderUI() const;
	void RenderAttractMode() const;

	//-----------------------------------------------------------------
	void Update();
	void AdjustForPauseAndTimeDitortion();
	void UpdateBullet(float deltaTime);
	void UpdateEnmBullet(float deltaTime);
	void UpdateExpBullet(float deltaTime);
	void UpdateAsteroids(float deltaTime);
	void UpdateBeetles(float deltaTime);
	void UpdateWasps(float deltaTime);
	void UpdateShootWasps(float deltaTime);
	void UpdateTurrets(float deltaTime);
	void UpdateDebris(float deltaTime);
	void UpdateBkg(float deltaTime);

	void UpdateAttractMode(float deltaTime);
	void UpdateGameplayMode(float deltaTime);
	void UpdateWorldCamera();
	void Translate2DToFollowedObj(Camera& worldCamera,const Vec2& aimPos);
	void UpdateQuitGameplayMode(float deltaTime);
	void UpdateCheatInput();

	//-----------------------------------------------------------------
	void DeleteGarbageEntities();
	//-----------------------------------------------------------------
	void PlayerShipKeepShoot();
	void CanonShootL1();
	void CanonShootL2();
	void CanonShootL3();
	void CanonShootL4();
	void PlayerShipJustShoot();
	void LaserCanonExplosionShoot();
	//-----------------------------------------------------------------
	void ShootWaspShootBullet(ShootWasp& thisShootWasp);
	void TurretShootBullet(Turret& thisTurret);
	//-----------------------------------------------------------------
	void GenerateEachWave(EnemyNumInWave curWaveEnemyNum);
	void GenerateAsteroids(int count);
	void GenerateEachAsteroids(int index);
	void GenerateBeetles(int count);
	void GenerateEachBeetles(int index);
	void GenerateWasps(int count);
	void GenerateEachWasp(int index);
	void GenerateTurret(int count);
	void GenerateEachTurret(int index);
	void GenerateDebris(Vec2 const& position, Rgba8 color, int count, float minSpeed,
		float maxSpeed,float minRadius, float maxRadius,Vec2 const&  oriVelocity);
	void GenerateExpDebris(Vec2 const& position, Rgba8 color, int count, float minSpeed,
		float maxSpeed, float minRadius, float maxRadius, Vec2 const&  oriVelocity);

	void GenerateShootWasps(int count);
	void GenerateEachShootWasps(int index);

	//Entity* SpawnEntity(EnemiesTypes type, Vec2 const& startPos, float orientationDegrees, Vec2 const& initialVelocity);
	void GenerateBkgStar();

	void WaveHandler(int aimWaveState);
	void DeleteCurAllEnemies();
	//-----------------------------------------------------------------
	void CheckBulletsVsEnemies();
	void CheckPlayerShipVsEnemies();
	void CheckExpBulletsVsEnemies();
	void CheckExpDebrisVSEnemies();
	void CheckLightSaberVsEnemies();
	//-----------------------------------------------------------------
	void ShakeScreen();
	RandomNumberGenerator* m_rng;
	//-----------------------------------------------------------------
	//bool m_isPause = false;
	//bool m_isSlow = false;
	//bool m_pauseAfterUpdate = false;
	float m_previousTimeScale = 1.f;
	//-------------------------------------------------------------------
	static bool Event_AdjustTimeScale(EventArgs& args);
	static bool Event_PrintKeyToDevConsole(EventArgs& args);

	Clock* m_gameClock = nullptr;
	PlayerShip*		m_playerShip = nullptr;			
	Asteroid*		m_asteroids[MAX_ASTEROIDS] = {};	
	Bullet*			m_bullets[MAX_BULLETS] = {};
	EnemyBullet* m_enmBullets[MAX_ENMBULLETS] = {};
	ExplosionBullet* m_expBullets[MAX_EXPBULLETS] = {};
	Beetle*			m_beetles[MAX_BEETLES] = {};
	Wasp*			m_wasps[MAX_WASPS] = {};
	Debris*			m_debris[MAX_DEBRIS] = {};
	Debris* m_expDebris[MAX_EXPDEBRIS];
	BkgStar* m_bkgStarsL1[1000] = {};
	BkgStar* m_bkgStarsL2[1000] = {};
	BkgStar* m_bkgStarsL4[300] = {};
	ShootWasp* m_shootWasps[MAX_SHOOTWASPS] = {};
	Turret* m_turret[MAX_TURRET] = {};

	bool m_isAsteroidsClear = true;
	bool m_isBeetlesClear = true;
	bool m_isWaspsClear = true;
	bool m_isShootWaspsClear = true;

	bool m_flagToReset = false;

	bool m_isBulletTime = false;
	float m_deltaTimeRatio = 1.f;

	Camera* m_worldCamera;
	Camera* m_screenCamera;
	Vec2 m_followedAim;

	double m_startShakeTime=0.0;

	//SoundID
	SoundID clickSound;
	SoundID playerNormalShootSound;
	SoundID playerExpShootSound;
	SoundID playerHurtSound;
	SoundID playerDiedSound;
	SoundID playerChangeWeaponSound;
	SoundID playerSaberSound;

	SoundID asteroidHurtSound;
	SoundID asteroidExpSound;
	SoundID expBulletShootSound;

	SoundID playerUpgradPickupSound;
	SoundID playerHpPickupSound;

	SoundID enemyHurtSound;
	SoundID enmeyDiedSound;
	SoundID enemyShootSound;

	SoundID playmodeBGM;
	SoundID attrackmodeBGM;

	//----new-------------
	SoundID playerRespawn;
	SoundID finalSuccess;
	SoundID finalDefeat;
	SoundID newWave;
	SoundID bulletTimeStart;
	SoundID setBulletTimeMove;
	SoundID dash;
	
private:

	bool isDebugMode = false;
	bool m_isArractMode = true;
	int m_waveState = 0;
	
	double m_startTime = 0;
	double m_currentTime = 0;
	float m_curAttractTriangle_a = 0;
	EnemyNumInWave m_enemyNumInEachWaves[5];
	bool m_isWin = false;
	float m_winTime = 0.f;

	bool m_isShakeScreen = false;
	Vec2 m_oriWorldScreenBL;
	Vec2 m_oriWorldScreenTR;

	//attrack mode
	float m_shipRevolutionDeg = 0;
	float m_shipScale = 10.f;
	Rgba8 m_at_shipColor;

	BitmapFont* m_font=nullptr;
};