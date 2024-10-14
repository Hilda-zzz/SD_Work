#pragma once
#include <Engine/Math/Vec2.hpp>
#include <Engine/Core/Rgba8.hpp>

constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 500;
constexpr int MAX_BULLETS = 1000;
constexpr int MAX_ENMBULLETS = 1000;
constexpr int MAX_BEETLES = 10;
constexpr int MAX_WASPS = 10;
constexpr int MAX_DEBRIS = 1000;
constexpr int MAX_EXPBULLETS = 100;
constexpr int MAX_EXPDEBRIS = 200;
constexpr int MAX_SHOOTWASPS = 20;
constexpr int MAX_TURRET = 20;

constexpr float WORLDSPACE_SIZE_TR_X = 400.f;
constexpr float WORLDSPACE_SIZE_TR_Y = 200.f;
constexpr float WORLDSPACE_SIZE_BL_X = -200.f;
constexpr float WORLDSPACE_SIZE_BL_Y = -100.f;

constexpr float WORLDSPACE_SIZE_X = 100.f;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_ASTEROID_VERTS = 48;

constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_BULLET_VERTS = 6;

constexpr float ENEMYBULLET_LIFETIME_SECONDS = 2.0f;
constexpr float ENEMYBULLET_SPEED = 50.f;
constexpr float ENEMYBULLET_PHYSICS_RADIUS = 0.5f;
constexpr float ENEMYBULLET_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_ENEMYBULLET_VERTS = 6;

constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_TURN_SPEED_SLOW = 100.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr int	NUM_SHIP_VERTS = 15;

constexpr float BEETLE_SPEED = 8.0f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.5f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_BEETLE_VERTS = 6;

constexpr float WASP_SPEED = 15.f;
constexpr float WASP_MAX_SPEED = 50.f;
constexpr float WASP_PHYSICS_RADIUS = 1.5f;
constexpr float WASP_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_WASP_VERTS = 6;

constexpr float SHOOTWASP_SPEED = 15.f;
constexpr float SHOOTWASP_MAX_SPEED = 50.f;
constexpr float SHOOTWASP_PHYSICS_RADIUS = 3.f;
constexpr float SHOOTWASP_COSMETIC_RADIUS = 4.0f;
constexpr int NUM_SHOOTWASP_VERTS = 21;

constexpr int	NUM_DEBRIS_VERTS = 32;

constexpr int NUM_CANONL1_VERTS = 3;
constexpr int NUM_CANONL2_VERTS = 6;
constexpr int NUM_CANONL3_VERTS = 9;
constexpr int NUM_LASER_CANON_VERTS = 15;

constexpr float EXPBULLET_PHYSICS_RADIUS = 0.5f;
constexpr float EXPBULLET_COSMETIC_RADIUS = 3.0f;
constexpr int NUM_EXPLOSION_BULLET_VERTS = 12;
constexpr float EXPBULLET_SPEED = 80.f;

constexpr int NUM_LIGHTSABER_EACHVERTS = 6;

constexpr float TURRET_FLOAT_SPEED = 50.f;
constexpr float TURRET_PHYSICS_RADIUS = 4.f;
constexpr float TURRET_COSMETIC_RADIUS = 2.0f;
constexpr int	NUM_TURRET_VERTS = 18;
constexpr int   DETECT_LEN = 70;

void DebugDrawLine(Vec2 const& start, Vec2 const& end, float width,Rgba8 color);
void DebugDrawRing(float thickness, float innerRadius, Rgba8 color, Vec2 ori);
void DebugDrawCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawHighCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawBoxLine(Vec2 botLeft, Vec2 topRight, float width, Rgba8 color);
void DebugDrawBox(Vec2 botLeft, Vec2 topRight, Rgba8 color);