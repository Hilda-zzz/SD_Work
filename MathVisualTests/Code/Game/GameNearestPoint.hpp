#pragma once
#include "Game/Game.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Triangle2.hpp"

class Clock;

class GameNearestPoint :public Game
{
public:
	GameNearestPoint();
	~GameNearestPoint();
	void Update() override;
	void Renderer() const override;
	void UpdateCamera(float deltaTime) override;

	Clock* m_gameClock = nullptr;
private:
	void UpdateInput(float deltaTime);
	void RenderBasicShape() const;
	void SetColor(bool isBright, Rgba8& shapeColor);
	void ReRandomObject();
	//dics
	Vec2 m_disc_pos;
	float m_disc_radius;
	Rgba8 m_disc_color;
	//capsule
	Vec2 m_cap_start;
	Vec2 m_cap_end;
	float m_cap_radius;
	Rgba8 m_cap_color;
	//OBB2
	Vec2 m_obb_center;
	Vec2 m_obb_iBasisNormal;
	Vec2 m_obb_halfDimensions;
	Rgba8 m_obb_color;
	//AABB2
	Vec2 m_aabb_mins;
	Vec2 m_aabb_maxs;
	Rgba8 m_aabb_color;
	//line
	Vec2 m_lineseg_start;
	Vec2 m_lineseg_end;
	//infinite line
	Vec2 m_infline_start;
	Vec2 m_infline_end;
	//triangle
	Triangle2 m_triangle;
	Rgba8 m_triangle_color;
	//aim
	Vec2 m_aimP_pos;

	Camera m_screenCamera;
};