#pragma once
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "RaycastAtrrow.hpp"
#include "Engine/Math/AABB2.hpp"
class Shape;
class Player;
class Game3DTestShapes :public Game
{
public:
	Game3DTestShapes();
	~Game3DTestShapes();
	void Update() override;
	void Renderer() const override;
	void Rerandom3DShapes();
	void UpdateCamera(float deltaTime) override;
private:
	void UpdateIsStationary();
	void ToggleReferPointMode();
	void UpdateOverlap();
public:
	Clock* m_gameClock = nullptr;
private:
	Camera m_screenCamera;
	RaycastResult2D m_final_result;
	RaycastArrow m_arrow;
	AABB2 m_worldUV;
	std::vector<Shape*> m_shapes;
	int m_index = -1;
	std::vector<Vertex_PCU> m_allVerts;
	Player* m_player = nullptr;
	bool m_isStationary = false;
	Vec3 m_curReferPoint=Vec3(0.f,0.f,0.f);
	EulerAngles m_curOrientation=EulerAngles(0.f,0.f,0.f);
	bool m_isGrabSth = false;
	int m_grabIndex = -1;
	bool m_isHitSth = false;
	Vec3 m_grabObjRelativePos = Vec3(0.f, 0.f, 0.f);
};