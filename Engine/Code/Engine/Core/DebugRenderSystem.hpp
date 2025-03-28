#pragma once
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "EventSystem.hpp"
//#include "Engine/Renderer/Renderer.hpp"	

class Renderer;
class Camera;
struct Vec3;
struct Vec2;
class Mat44;
struct AABB2;
//class EventArgs;

enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY
};

struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
};

//Set up
void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();

//Control
void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();

//Output
void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();

//Geometry
void DebugAddWorldPoint(const Vec3& pos,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldLine(const Vec3& start, const Vec3& end,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldWireSphere(const Vec3& center,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldArrow(const Vec3& start, const Vec3& end,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldText(const std::string& text,
	const Mat44& transform, float textHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldBillboardText(const std::string& text,
	const Vec3& origin, float textHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
//void DebugAddWorldBasis(const Mat44& transform, float duration, 
//	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldBasis(const Mat44& transform, float duration, DebugRenderMode mode=DebugRenderMode::USE_DEPTH, float scale = 1.0f);
void DebugAddScreenText(const std::string& text, const AABB2& textBox, 
	float textHeight, const Vec2& alignment, 
	float duration, const Rgba8& startColor, 
	const Rgba8& endColor);
void DebugAddMessage(const std::string& text,
	float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE);

//Console commands
bool Command_DebugRenderClear(EventArgs& args);
bool Command_DebugRenderToggle(EventArgs& args);