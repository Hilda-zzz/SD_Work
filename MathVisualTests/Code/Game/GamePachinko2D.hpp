#pragma once
#include "Game/Game.hpp"
#include "RaycastAtrrow.hpp"
#include "Bumper2D.hpp"
#include "Ball.hpp"

class Clock;

class GamePachinko2D :public Game
{
public:
	GamePachinko2D();
	~GamePachinko2D();
	void Update() override;
	void UpdatePhysics(float fixedTimeStep);
	void Renderer() const override;
	void UpdateCamera(float deltaTime) override;
public:
	Clock* m_gameClock = nullptr;
private:
	void UpdateInput(float deltaTime);
	void UpdateKBInputForArrow(float& deltaTime, RaycastArrow& arrow, float speed);

	void ParseGameConfig();
	void GenerateEachBall();
	void GenerateBumper2D();
	void ReRandomObject();
	void ChangeBallElasicity(float change);
private:
	Camera m_screenCamera;

	AABB2 m_worldUV;
	RaycastArrow m_arrow;

	std::vector<Bumper2D> m_bumpers;
	std::vector<Vertex_PCU> m_bumpersVerts;
	std::vector<Vertex_PCU> m_ballVerts;

	std::vector<Ball> m_balls;
	float m_ballElasicity = 0.9f;
	float m_wallElasicity = 0.9f;

	bool m_isFloor = true;

	float m_phyOwe = 0.f;
	float m_fixedTimeStep = 0.005f;
	float m_curDeltaTime = 0.f;
	size_t m_ballCount = 0;

	bool m_isUseFixedUpdate = true;

	//---------------------------------------
	float m_pachinkoMinBallRadius=5.f;
	float m_pachinkoMaxBallRadius=25.f;

	int m_pachinkoNumDiscBumpers=10;
	float m_pachinkoMinDiscBumperRadius=5;
	float m_pachinkoMaxDiscBumperRadius=50;

	int m_pachinkoNumCapsuleBumpers=10;
	float m_pachinkoMinCapsuleBumperLength=1;
	float m_pachinkoMaxCapsuleBumperLength=150;
	float m_pachinkoMinCapsuleBumperRadius=5;
	float m_pachinkoMaxCapsuleBumperRadius=50;

	int m_pachinkoNumObbBumpers=10;
	float m_pachinkoMinObbBumperWidth=5;
	float m_pachinkoMaxObbBumperWidth=80;

	float m_pachinkoWallElasticity=0.9f;
	float m_pachinkoMinBumperElasticity=0.01f;
	float m_pachinkoMaxBumperElasticity=0.99f;

	float m_pachinkoExtraWarpHeight=300.f;

};