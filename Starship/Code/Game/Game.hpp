#pragma once
#include "GameCommon.hpp"
#include "PlayerShip.hpp"
#include "Bullet.hpp"
#include"Engine/Math/RandomNumberGenerator.hpp"
//class PlayerShip;
//class Bullet;
//class RandomNumberGeneration;
#include "Asteroid.hpp"
#include "Beetle.hpp"
#include "Wasp.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Debris.hpp"
class Game
{
public:
	Game();
	~Game();
	void InitGameplay();
	void ResetGameplay();
	void Update(float deltaSeconds);
	void Renderer() const;
	void RenderBullet() const;
	void RenderAsteroids() const;

	void RenderBeetles() const;
	void RenderWasps() const;
	void RenderDebris() const;
	void RenderDebugMode() const;

	void RenderEnemyDebugMode(int count, Entity* entityGroup[]) const;

	void RenderUI() const;

	void RenderAttractMode() const;
	void UpdateBullet(float deltaTime);

	void UpdateAsteroids(float deltaTime);

	void UpdateBeetles(float deltaTime);
	void UpdateWasps(float deltaTime);
	void UpdateDebris(float deltaTime);

	void UpdateAttractMode(float deltaTime);
	void DeleteGarbageEntities();

	void Shoot();

	void GenerateAsteroids(int count);
	void GenerateEachAsteroids(int index);

	void GenerateBeetles(int count);
	void GenerateEachBeetles(int index);

	void GenerateWasps(int count);
	void GenerateEachWasp(int index);

	void GenerateDebris(Vec2& const position, Rgba8 color, int count, float minSpeed,float maxSpeed,float minRadius, float maxRadius,Vec2& const oriVelocity);

	void GenerateNewWave(int state);

	void CheckBulletsVsAsteroids();
	void CheckPlayerShipVsAsteroids();

	void CheckBulletsVsBeetles();

	void CheckPlayerShipVsBeetles();


	PlayerShip* m_playerShip = nullptr;			// Just one player ship (for now...)
	Asteroid* m_asteroids[MAX_ASTEROIDS] = {};	// Fixed number of asteroid “slots”; nullptr if unused.
	Bullet* m_bullets[MAX_BULLETS] = {};		// The “= {};” syntax initializes the array to zeros.	
	int m_curBulletCount = 0;
	RandomNumberGenerator* m_rng;
	bool isDebugMode=false;

	Beetle* m_beetles[MAX_BEETLES] = {};
	Wasp* m_wasps[MAX_WASPS] = {};
	Debris* m_debris[MAX_DEBRIS] = {};

	int m_waveState = 1;
	bool m_isAsteroidsClear = true;
	bool m_isBeetlesClear = true;
	bool m_isWaspsClear = true;
	bool m_isArractMode = true;
	bool m_flagToReset = false;
	Vertex_PCU vertices[NUM_SHIP_VERTS];
private:
	double startTime = 0;
	double currentTime = 0;
	float curAttractTriangle_a = 0;
};