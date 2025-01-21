#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Entity.hpp"

class SpriteAnimDefinition;
class App;
class InputSystem;
class AudioSystem;
class Renderer;
class Window;
struct Vec2;
struct Rgba8;
class Game;

extern App* g_theApp;
extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
extern Game* g_theGame;
extern BitmapFont* g_testFont;

extern SpriteSheet* g_explosionAnimSpriteSheet;
extern SpriteAnimDefinition* g_explosionAnimDef_Slow;
extern SpriteAnimDefinition* g_explosionAnimDef_Fast;
extern SpriteSheet* g_decoSpriteSheet;
//-------------------------------------------------------------------------------------
constexpr float		PLAYER_PHY_RADIUS = 0.45f;
constexpr float		PLAYER_COS_RADIUS = 0.8f;
constexpr float		PLAYER_TURRET_HDIM_X = 0.1f;
constexpr float		PLAYER_TURRET_HDIM_Y = 0.5f;
//-------------------------------------------------------------------------------------
constexpr float		SCORPIO_PHY_RADIUS = 0.4f;
constexpr float		SCORPIO_COS_RADIUS = 0.4f;
//-------------------------------------------------------------------------------------
constexpr float		LEO_PHY_RADIUS = 0.4f;
constexpr float		LEO_COS_RADIUS = 0.4f;
//-------------------------------------------------------------------------------------
constexpr float		ARIES_PHY_RADIUS = 0.4f;
constexpr float		ARIES_COS_RADIUS = 0.4f;
//-------------------------------------------------------------------------------------
constexpr float	 BULLET_PLAYER_PHY_RADIUS=0.03f;
constexpr float	 BULLET_PLAYER_COS_RADIUS=0.03f;
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------

void DebugDrawLine(Vec2 const& start, Vec2 const& end, float width,Rgba8 color);
void DebugDrawRing(float thickness, float innerRadius, Rgba8 color, Vec2 ori);
void DebugDrawCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawHighCircle(float radius, Vec2 ori, Rgba8 color);
void DebugDrawBoxLine(Vec2 botLeft, Vec2 topRight, float width, Rgba8 color);
void DebugDrawBox(Vec2 botLeft, Vec2 topRight, Rgba8 color);
void DebugDrawBox(AABB2 const& bounds, Rgba8 color);