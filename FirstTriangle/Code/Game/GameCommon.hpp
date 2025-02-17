#pragma once
#include "Engine/Core/EngineCommon.hpp"

class App;
class InputSystem;
class AudioSystem;
//class Renderer;
class Window;
struct Vec2;
struct Rgba8;

extern App* g_theApp;
extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern AudioSystem* g_theAudio;

constexpr float		SCREEN_SIZE_X=1600.f;
constexpr float		SCREEN_SIZE_Y=800.f;
