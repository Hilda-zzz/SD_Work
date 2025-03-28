#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
class App;
class InputSystem;
class AudioSystem;
class Renderer;
class Window;
struct Vec2;
struct Rgba8;

extern App* g_theApp;
extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;

constexpr float		SCREEN_SIZE_X=1600.f;
constexpr float		SCREEN_SIZE_Y=800.f;
constexpr float		GAME_SIZE_X = 26.f;
constexpr float		GAME_SIZE_Y = 13.f;

void DebugDrawLine(Vec2 const& start, Vec2 const& end, float width,Rgba8 color);
void DebugDrawRing(float thickness, float innerRadius, Rgba8 color, Vec2 ori);
void DebugDrawCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawHighCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawBoxLine(Vec2 botLeft, Vec2 topRight, float width, Rgba8 color);
void DebugDrawBox(Vec2 botLeft, Vec2 topRight, Rgba8 color);