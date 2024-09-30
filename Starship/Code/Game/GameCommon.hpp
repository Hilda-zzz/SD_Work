#pragma once
#include <Engine/Math/Vec2.hpp>
#include <Engine/Core/Rgba8.hpp>

constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 12;
constexpr int MAX_BULLETS = 20;
constexpr int MAX_BEETLES = 10;
constexpr int MAX_WASPS = 10;
constexpr int MAX_DEBRIS = 50;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_ASTEROID_VERTS = 48;

constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_BULLET_VERTS = 6;

constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr int NUM_SHIP_VERTS = 15;

constexpr float BEETLE_SPEED = 8.0f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.5f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_BEETLE_VERTS = 6;

constexpr float WASP_SPEED = 15.f;
constexpr float WASP_MAX_SPEED = 50.f;
constexpr float WASP_PHYSICS_RADIUS = 1.5f;
constexpr float WASP_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_WASP_VERTS = 6;

constexpr int NUM_DEBRIS_VERTS = 32;

constexpr int WAVE1_ASTEROIDS = 2;
constexpr int WAVE1_BEETLES = 1;
constexpr int WAVE1_WASP = 0;

constexpr int WAVE2_ASTEROIDS = 4;
constexpr int WAVE2_BEETLES = 2;
constexpr int WAVE2_WASP = 1;

constexpr int WAVE3_ASTEROIDS = 6;
constexpr int WAVE3_BEETLES = 4;
constexpr int WAVE3_WASP = 2;

void DebugDrawLine(Vec2 const& start, Vec2 const& end, float width,Rgba8 color);
void DebugDrawRing(float thickness, float innerRadius, Rgba8 color, Vec2 ori);
